"""Deterministic, local PNG editing. No model or GPU is used."""
import argparse
import json
import os
import re
import sys
from pathlib import Path
from PIL import Image


def emit(kind, **values):
    print(json.dumps(dict(type=kind, **values), ensure_ascii=True), flush=True)


def absolute_path(value, label):
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f'{label} is required.')
    path = Path(value)
    if not path.is_absolute():
        raise ValueError(f'{label} must be an absolute path.')
    return path.resolve()


def rgb(value):
    if not isinstance(value, str) or not re.fullmatch(r'#[0-9a-fA-F]{6}', value):
        raise ValueError('Colors must use #RRGGBB format.')
    return tuple(int(value[i:i + 2], 16) for i in (1, 3, 5))


def load(path):
    with Image.open(path) as image:
        return image.convert('RGBA')


def run(request):
    if not isinstance(request, dict):
        raise ValueError('Request must be an object.')
    action = request.get('action')
    if action not in ('flip', 'recolor', 'sheet'):
        raise ValueError('action must be flip, recolor or sheet.')
    output = absolute_path(request.get('outputPath'), 'outputPath')
    if output.suffix.lower() != '.png':
        raise ValueError('outputPath must have .png extension.')
    if action == 'sheet':
        frames = request.get('frames')
        if not isinstance(frames, list) or not 1 <= len(frames) <= 256:
            raise ValueError('frames must contain 1 to 256 ordered image paths.')
        inputs = [absolute_path(path, 'frame') for path in frames]
    else:
        inputs = [absolute_path(request.get('inputPath'), 'inputPath')]
    for path in inputs:
        if output == path or (output.exists() and path.exists() and os.path.samefile(output, path)):
            raise ValueError('Output must not overwrite any original input image.')
    metadata = None
    if action == 'flip':
        axes = {'horizontal': Image.Transpose.FLIP_LEFT_RIGHT, 'vertical': Image.Transpose.FLIP_TOP_BOTTOM}
        axis = request.get('axis', 'horizontal')
        if axis not in axes:
            raise ValueError('axis must be horizontal or vertical.')
        result = load(inputs[0]).transpose(axes[axis])
    elif action == 'recolor':
        old, new = rgb(request.get('fromColor')), rgb(request.get('toColor'))
        result = load(inputs[0])
        result.putdata([(*new, a) if (r, g, b) == old else (r, g, b, a) for r, g, b, a in result.getdata()])
    else:
        images = [load(path) for path in inputs]
        cell_width = max(image.width for image in images)
        cell_height = max(image.height for image in images)
        width = cell_width * len(images)
        if width * cell_height > 64_000_000:
            raise ValueError('Sprite sheet exceeds the 64 million pixel safety limit.')
        result = Image.new('RGBA', (width, cell_height), (0, 0, 0, 0))
        metadata = {'imagePath': str(output), 'layout': 'horizontal', 'cellWidth': cell_width,
                    'cellHeight': cell_height, 'frameCount': len(images), 'frames': []}
        for index, (path, image) in enumerate(zip(inputs, images)):
            bounds = image.getchannel('A').getbbox()
            if bounds is None:
                raise ValueError(f'Frame {index + 1} is fully transparent: {path.name}')
            crop = image.crop(bounds)
            cell_x = index * cell_width
            x = cell_x + (cell_width - crop.width) // 2
            y = cell_height - crop.height
            # Copy RGBA directly: using alpha as a paste mask would square semi-transparent alpha.
            result.paste(crop, (x, y))
            metadata['frames'].append({'index': index, 'sourcePath': str(path),
                'x': cell_x, 'y': 0, 'width': cell_width, 'height': cell_height,
                'contentX': x, 'contentY': y, 'contentWidth': crop.width, 'contentHeight': crop.height,
                'sourceBounds': list(bounds)})
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_name(output.name + '.tmp')
    metadata_path = output.with_suffix('.json') if metadata else None
    metadata_tmp = metadata_path.with_name(metadata_path.name + '.tmp') if metadata else None
    try:
        result.save(temporary, format='PNG')
        if metadata:
            metadata_tmp.write_text(json.dumps(metadata, ensure_ascii=False, indent=2), encoding='utf-8')
        os.replace(temporary, output)
        if metadata:
            os.replace(metadata_tmp, metadata_path)
    finally:
        temporary.unlink(missing_ok=True)
        if metadata_tmp:
            metadata_tmp.unlink(missing_ok=True)
    response = {'outputPath': str(output), 'width': result.width, 'height': result.height}
    if metadata_path:
        response['metadataPath'] = str(metadata_path)
    return response


def protect_request_file(request, request_path):
    output = absolute_path(request.get('outputPath'), 'outputPath')
    targets = [output]
    if request.get('action') == 'sheet':
        targets.append(output.with_suffix('.json'))
    if request_path.resolve() in targets:
        raise ValueError('Output or sheet metadata must not overwrite the request JSON file.')


def main():
    sys.stdout.reconfigure(encoding='utf-8')
    parser = argparse.ArgumentParser()
    parser.add_argument('--request', required=True)
    args = parser.parse_args()
    try:
        with open(args.request, encoding='utf-8-sig') as stream:
            request = json.load(stream)
        protect_request_file(request, Path(args.request))
        response = run(request)
        emit('result', **response)
        return 0
    except Exception as error:
        emit('error', message=f'{type(error).__name__}: {error}')
        return 1


if __name__ == '__main__':
    raise SystemExit(main())

