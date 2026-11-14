import argparse
import json
import os
import secrets
import sys
import time
from pathlib import Path

ROOT = Path(os.environ.get('LOCALAPPDATA', str(Path.home()))) / 'PixelArtStudio'
os.environ.setdefault('HF_HOME', str(ROOT / 'models'))
os.environ.setdefault('U2NET_HOME', str(ROOT / 'models' / 'rembg'))
os.environ.setdefault('HF_HUB_DISABLE_SYMLINKS_WARNING', '1')
BASE_MODEL = 'stabilityai/stable-diffusion-xl-base-1.0'
LORA = 'nerijs/pixel-art-xl'
CATEGORY_PREFIXES = {
    'character': 'pixel art, simple flat colors, centered single game sprite, isolated on plain white background, ',
    'background': 'pixel art, game background environment, full scene composition, ',
    'tile': 'pixel art, seamless repeating game texture tile, edge-to-edge pattern, ',
}


def emit(kind, **values):
    print(json.dumps(dict(type=kind, **values), ensure_ascii=True), flush=True)


def validate(request):
    if not isinstance(request, dict):
        raise ValueError('Request must be a JSON object.')
    if request.get('category', 'character') not in CATEGORY_PREFIXES:
        raise ValueError('category must be character, background or tile.')
    if type(request.get('protectInterior', True)) is not bool:
        raise ValueError('protectInterior must be a boolean.')
    for name in ('stylePrompt', 'styleNegativePrompt'):
        if not isinstance(request.get(name, ''), str) or len(request.get(name, '')) > 2000:
            raise ValueError(f'{name} must be a string of at most 2000 characters.')
    prompt = request.get('prompt', '')
    if not isinstance(prompt, str) or not prompt.strip() or len(prompt) > 4000:
        raise ValueError('Prompt must contain 1 to 4000 characters.')
    for name in ('width', 'height'):
        if type(request.get(name, 32)) is not int or request.get(name, 32) not in (32, 64, 128):
            raise ValueError('Output dimensions must be 32, 64 or 128.')
    steps = request.get('steps', 20)
    if type(steps) is not int or not 4 <= steps <= 50:
        raise ValueError('Steps must be an integer from 4 to 50.')
    strength = request.get('strength', 0.6)
    if not isinstance(strength, (int, float)) or not 0.1 <= strength <= 1:
        raise ValueError('Strength must be from 0.1 to 1.')
    seed = request.get('seed', -1)
    if type(seed) is not int or not -1 <= seed <= 4294967295:
        raise ValueError('Seed must be -1 or a 32-bit unsigned integer.')
    out = Path(request.get('outputPath', ''))
    if not out.is_absolute() or out.suffix.lower() != '.png':
        raise ValueError('outputPath must be an absolute PNG path.')
    reference = request.get('referenceImage')
    if reference and not Path(reference).is_file():
        raise ValueError('Reference image does not exist.')
    if reference:
        reference_path = Path(reference).resolve()
        targets = [out, out.with_name(out.stem + '.source.png'), out.with_name(out.stem + '.generation.json')]
        for target in targets:
            if target.resolve() == reference_path or (target.exists() and os.path.samefile(target, reference_path)):
                raise ValueError('Generation outputs must not overwrite the reference image.')
    return request


def compose_prompts(request):
    style = request.get('stylePrompt', '').strip()
    resource = request['prompt'].strip()
    prompt = CATEGORY_PREFIXES[request.get('category', 'character')] + ', '.join(part for part in (style, resource) if part)
    base_negative = request.get('negativePrompt') or 'photo, realistic, blurry, text, watermark, cropped, multiple objects'
    style_negative = request.get('styleNegativePrompt', '').strip()
    negative = ', '.join(part for part in (base_negative.strip(), style_negative) if part)
    return prompt, negative


def transparency_warning(image):
    # ponytail: occupancy heuristic only; use segmentation-aware checks if thin effects need support.
    alpha = image.getchannel('A')
    visible = sum(alpha.histogram()[128:]) / (image.width * image.height)
    if visible < 0.05:
        return 'Very little foreground remains (under 5%). Check whether the object is intentionally small or background removal lost detail.'
    return None


def protect_interior_holes(image, original):
    """Conservative topology heuristic, not semantic recovery of missing subjects."""
    if image.size != original.size or image.mode != 'RGBA' or original.mode != 'RGBA':
        raise ValueError('Interior protection requires aligned RGBA images of equal size.')
    width, height = image.size
    alpha = list(image.getchannel('A').getdata())
    pending = {i for i, value in enumerate(alpha) if value < 128}
    limit = min(16, max(1, width * height // 1000))
    result = image.copy()
    restored_pixels = restored_regions = 0
    while pending:
        first = pending.pop()
        component, stack = [first], [first]
        exterior = False
        while stack:
            index = stack.pop()
            x, y = index % width, index // width
            exterior |= x in (0, width - 1) or y in (0, height - 1)
            for ny in range(max(0, y - 1), min(height, y + 2)):
                for nx in range(max(0, x - 1), min(width, x + 2)):
                    neighbor = ny * width + nx
                    if neighbor in pending:
                        pending.remove(neighbor)
                        component.append(neighbor)
                        stack.append(neighbor)
        if exterior or len(component) > limit:
            continue
        changed = 0
        for index in component:
            xy = index % width, index // width
            source = original.getpixel(xy)
            if alpha[index] <= 8 and source[3] >= 128:
                result.putpixel(xy, source)
                changed += 1
        restored_pixels += changed
        restored_regions += int(changed > 0)
    return result, restored_pixels, restored_regions


def save_png(image, output):
    temporary = output.with_name(output.name + '.tmp')
    image.save(temporary, format='PNG')
    os.replace(temporary, output)


def save_generation_metadata(request, output, source, seed, effective_prompt, protection=None, effective_negative_prompt=None):
    path = output.with_name(output.stem + '.generation.json')
    metadata = dict(request)
    metadata.update(seed=seed, category=request.get('category', 'character'),
                    outputPath=str(output), sourcePath=str(source), effectivePrompt=effective_prompt,
                    effectiveNegativePrompt=effective_negative_prompt if effective_negative_prompt is not None else compose_prompts(request)[1],
                    stylePrompt=request.get('stylePrompt', ''), styleNegativePrompt=request.get('styleNegativePrompt', ''),
                    model=BASE_MODEL, lora=LORA)
    if protection is not None:
        metadata.update(protection)
    temporary = path.with_name(path.name + '.tmp')
    try:
        temporary.write_text(json.dumps(metadata, ensure_ascii=False, indent=2), encoding='utf-8')
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)
    return path


def generate(request):
    request = validate(request)
    started = time.monotonic()
    emit('progress', message='Loading local image engine. First use downloads model weights.', percent=1)
    import torch
    from PIL import Image, ImageOps
    from diffusers import StableDiffusionXLPipeline, StableDiffusionXLImg2ImgPipeline
    if not torch.cuda.is_available():
        raise RuntimeError('CUDA GPU is unavailable. Install a compatible NVIDIA driver and run setup.ps1.')
    seed = request.get('seed', -1)
    if seed == -1:
        seed = secrets.randbelow(4294967296)
    reference = request.get('referenceImage')
    cls = StableDiffusionXLImg2ImgPipeline if reference else StableDiffusionXLPipeline
    pipe = cls.from_pretrained(BASE_MODEL, torch_dtype=torch.float16, variant='fp16', use_safetensors=True)
    pipe.load_lora_weights(LORA, weight_name='pixel-art-xl.safetensors')
    pipe.enable_model_cpu_offload()
    pipe.enable_vae_tiling()
    pipe.set_progress_bar_config(disable=True)
    steps = request.get('steps', 20)
    actual_steps = max(1, int(steps * request.get('strength', 0.6))) if reference else steps
    def callback(pipeline, index, timestep, values):
        emit('progress', message=f'Generating image {index + 1}/{actual_steps}', percent=min(87, 10 + int(77 * (index + 1) / actual_steps)))
        return values
    prompt, negative_prompt = compose_prompts(request)
    args = dict(prompt=prompt, negative_prompt=negative_prompt, num_inference_steps=steps, guidance_scale=7.0, generator=torch.Generator(device='cpu').manual_seed(seed), callback_on_step_end=callback)
    # SDXL works at large resolution; pixel-size export is a separate deterministic step.
    if reference:
        with Image.open(reference) as source:
            rgba = source.convert('RGBA')
            white = Image.new('RGBA', rgba.size, (255, 255, 255, 255))
            white.alpha_composite(rgba)
            args['image'] = ImageOps.fit(white.convert('RGB'), (768, 768), method=Image.Resampling.NEAREST)
        args['strength'] = request.get('strength', 0.6)
    else:
        args.update(width=768, height=768)
    image = pipe(**args).images[0]
    del pipe
    torch.cuda.empty_cache()
    emit('progress', message='Preparing pixel PNG.', percent=90)
    output = Path(request['outputPath'])
    output.parent.mkdir(parents=True, exist_ok=True)
    source = output.with_name(output.stem + '.source.png')
    save_png(image, source)
    output_size = (request.get('width', 32), request.get('height', 32))
    original = image.convert('RGBA').resize(output_size, Image.Resampling.NEAREST)
    if request.get('transparent', True):
        from rembg import remove, new_session
        image = remove(image, session=new_session('u2netp', providers=['CPUExecutionProvider']), putalpha=True)
    image = image.convert('RGBA').resize(output_size, Image.Resampling.NEAREST)
    protection = dict(protectInterior=request.get('protectInterior', True), interiorRestoredPixels=0, interiorRestoredRegions=0)
    if request.get('transparent', True) and protection['protectInterior']:
        image, pixels, regions = protect_interior_holes(image, original)
        protection.update(interiorRestoredPixels=pixels, interiorRestoredRegions=regions)
    warning = transparency_warning(image) if request.get('transparent', True) else None
    if protection['interiorRestoredPixels']:
        note = f"Restored {protection['interiorRestoredPixels']} pixels in small enclosed alpha holes. Tiny intentional holes may also be filled; disable interior protection if needed."
        warning = (warning + ' ' + note) if warning else note
    if warning:
        emit('progress', message=warning, percent=95)
    save_png(image, output)
    metadata_path = save_generation_metadata(request, output, source, seed, prompt, dict(protection, warning=warning), negative_prompt)
    emit('result', outputPath=str(output), sourcePath=str(source), metadataPath=str(metadata_path), warning=warning, seed=seed, elapsedSeconds=round(time.monotonic() - started, 2), **protection)


def main():
    sys.stdout.reconfigure(encoding='utf-8')
    parser = argparse.ArgumentParser()
    parser.add_argument('--request')
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    try:
        if args.check:
            import torch, diffusers, rembg
            emit('status', ready=torch.cuda.is_available(), gpu=torch.cuda.get_device_name(0) if torch.cuda.is_available() else None, torchVersion=torch.__version__, model=BASE_MODEL)
            return 0 if torch.cuda.is_available() else 1
        if not args.request:
            raise ValueError('--request <json> is required.')
        with open(args.request, encoding='utf-8-sig') as stream:
            generate(json.load(stream))
        return 0
    except Exception as error:
        emit('error', message=f'{type(error).__name__}: {error}')
        return 1


if __name__ == '__main__':
    raise SystemExit(main())








