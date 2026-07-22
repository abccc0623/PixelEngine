from pathlib import Path

from PIL import Image, ImageEnhance


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "output" / "imagegen" / "basic-energy-orb-alpha.png"
SHEET = ROOT / "output" / "imagegen" / "basic-energy-orb-8f-256x32.png"
PREVIEW = ROOT / "output" / "imagegen" / "basic-energy-orb-8f-preview.png"


source = Image.open(SOURCE).convert("RGBA")
box = source.getchannel("A").getbbox()
if box is None:
    raise RuntimeError("The source image has no visible pixels")

orb = source.crop(box)
sheet = Image.new("RGBA", (256, 32), (0, 0, 0, 0))

# A gentle pulse plus quarter-turns makes the internal light and orange accents
# circulate while keeping every frame readable as the same basic projectile.
sizes = (13, 14, 15, 16, 15, 14, 13, 12)
turns = (0, 1, 2, 3, 0, 1, 2, 3)
brightness = (0.92, 0.98, 1.04, 1.10, 1.04, 0.98, 0.92, 0.88)

for index, (size, turn, light) in enumerate(zip(sizes, turns, brightness)):
    frame_orb = orb.resize((size, size), Image.Resampling.NEAREST)
    if turn:
        frame_orb = frame_orb.rotate(turn * 90, resample=Image.Resampling.NEAREST, expand=False)
    rgb = ImageEnhance.Brightness(frame_orb.convert("RGB")).enhance(light)
    alpha = frame_orb.getchannel("A").point(lambda value: 255 if value >= 128 else 0)
    frame_orb = Image.merge("RGBA", (*rgb.split(), alpha))

    frame = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    frame.alpha_composite(frame_orb, ((32 - size) // 2, (32 - size) // 2))
    sheet.alpha_composite(frame, (index * 32, 0))

sheet.save(SHEET)
sheet.resize((2048, 256), Image.Resampling.NEAREST).save(PREVIEW)

print(f"Wrote {SHEET}")
print(f"Wrote {PREVIEW}")
