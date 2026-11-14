# Local image backend

Run `powershell -File Backend/setup.ps1` once. It installs a private Python 3.11 and venv in `%LOCALAPPDATA%/PixelArtStudio`, without changing PATH or system drivers. CUDA-capable NVIDIA driver is required. First image downloads about 8 GB of model weights; runtime dependencies consume several additional GB. Models, runtime and outputs must not be committed to Git.

Protocol: `venv/Scripts/python.exe Backend/generate.py --request request.json`. Stdout contains JSON lines: `progress` with `percent`/`message`; `result` with `outputPath`/`seed`/`elapsedSeconds`; or `error` with `message` and exit code 1. Stderr contains third-party diagnostics/download progress. Cancellation kills only the owned Python process tree. `--check` verifies dependencies and CUDA, not model download or image quality.

Request fields: `prompt`, `negativePrompt`, `width`/`height` (32,64,128), `steps` (4..50, default20), `seed` (-1 random or uint32), `referenceImage` (optional local path), `strength` (0.1..1, default0.6), `transparent` (defaulttrue), `outputPath` (absolute PNG path). Prompt in English is recommended for SDXL. Generated game assets need visual review.

SDXL generates at 768x768. The Pixel Art XL LoRA guides style; rembg U2NetP removes the background on CPU, then nearest-neighbor export produces the requested pixel dimensions. This is not hand-authored pixel art, and removal may erase details or leave fringes. Reference mode is whole-image img2img, not precise localized editing. The original image is not overwritten unless explicitly selected as the output path. Model CPU offloading fits the target 10 GB GPU more conservatively but adds transfer latency. Close other GPU-heavy model applications if memory is insufficient.

Official sources and licenses:
- Python: https://www.python.org/downloads/release/python-3119/
- PyTorch: https://pytorch.org/get-started/previous-versions/
- Diffusers: https://github.com/huggingface/diffusers
- SDXL: https://huggingface.co/stabilityai/stable-diffusion-xl-base-1.0 (OpenRAIL++)
- Pixel Art XL: https://huggingface.co/nerijs/pixel-art-xl (CreativeML OpenRAIL-M)
- rembg: https://github.com/danielgatis/rembg

No API account or per-image cloud inference fee is used. Downloads require internet; cached generation runs locally. Model licenses have use restrictions; do not imply unrestricted redistribution of weights.

## Transparent-output safety

Every generation now preserves the full-size pre-removal image beside the export as `<output-stem>.source.png`. The result event includes `sourcePath` and nullable `warning`. If fewer than 5% of final pixels have alpha >= 128, a progress warning and result warning ask whether the foreground is intentionally small or detail was removed. Transparency is never silently disabled. This occupancy heuristic can warn on legitimately tiny/thin assets and does not guarantee segmentation quality. Previously generated images cannot recover their discarded source; preservation applies to subsequent generations.

## Deterministic editing (no model calls)

`python image_ops.py --request request.json` accepts:
- `flip`: `inputPath`, `axis` (`horizontal` or `vertical`).
- `recolor`: `inputPath`, `fromColor`, `toColor` as exact `#RRGGBB`. Alpha is preserved, including transparent pixel alpha.
- `sheet`: ordered `frames` paths. Creates one horizontal strip, fixed cell size equal to the maximum original canvas width/height. Alpha bounding boxes are bottom-center aligned without resampling or clipping. Fully transparent frames fail rather than changing frame order. Result `.json` contains frame source paths, cell coordinates and content rectangles.
All requests require absolute `outputPath` ending in `.png`. Input images and request JSON cannot be overwritten. `result` includes `outputPath`, `width`, `height` and (for sheets) `metadataPath`. Failure returns `error` and exit code 1.

Generation additionally accepts `category`: `character` (default), `background`, or `tile`, selecting a matching prompt prefix. Tile prompting does not guarantee mathematically seamless edges. Transparency remains an explicit independent option. A `<stem>.generation.json` sidecar stores request settings, actual random seed, effective prompt, model and file paths for headless and UI requests alike.

## Automatic conservative interior protection

`protectInterior` defaults to `true` and may be disabled. After final nearest-neighbor downsampling, 8-connected regions with alpha < 128 are examined. Any region touching an image edge is left unchanged, including diagonal/weak-alpha connections. Only enclosed regions <= 0.1% of output area (minimum 1, maximum 16 pixels) qualify. Within those, only pixels with alpha <= 8 are restored from the aligned original RGB and alpha. Semi-transparent edge pixels remain unchanged. This deterministic CPU heuristic does not recover an erased entire subject, large holes, or exterior-connected missing detail. Tiny intentional holes may be filled; disabling protection preserves them. Result and generation metadata include `protectInterior`, `interiorRestoredPixels`, `interiorRestoredRegions`, and warning text when restoration occurs. The full-size source remains unchanged.

## Shared style prompts

Optional `stylePrompt` and `styleNegativePrompt` are strings of at most 2000 characters each; omitted values are empty. Positive text is composed in this order: category prefix, shared style, resource prompt. Negative text is the resource negative prompt (or existing default) followed by shared style negatives. The generation sidecar retains original style text plus `effectivePrompt` and `effectiveNegativePrompt`; unrelated request fields such as a `background` dictionary are preserved without reinterpretation. Keep style descriptions short: SDXL text encoders may truncate long combined prompts, including later resource details. This is prompt guidance, not a guarantee of consistent characters or style; a fixed seed is not a style identity.

Background removal uses putalpha=True to preserve straight RGB instead of multiplying RGB by alpha. This fixes darkening at translucent pixels; it does not repair a wrong segmentation mask or restore missing body parts.

