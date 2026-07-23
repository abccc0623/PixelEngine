from pathlib import Path

from PIL import Image, ImageEnhance


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "output" / "imagegen" / "neutral-energy-orb-alpha.png"
SHEET = ROOT / "output" / "imagegen" / "neutral-energy-orb-12f-384x32.png"
PREVIEW = ROOT / "output" / "imagegen" / "neutral-energy-orb-12f-preview.png"


source = Image.open(SOURCE).convert("RGBA")
box = source.getchannel("A").getbbox()
if box is None:
    raise RuntimeError("The source image has no visible pixels")

cropped = source.crop(box)
gray = cropped.convert("L")
orb = Image.merge("RGBA", (gray, gray, gray, cropped.getchannel("A")))
sheet = Image.new("RGBA", (384, 32), (0, 0, 0, 0))

# One seamless 12-frame cycle: a gentle pulse, brightness breathing, and a
# rotating highlight. The final image remains strictly achromatic.
sizes = (13, 13, 14, 15, 16, 16, 15, 14, 13, 12, 12, 13)
brightness = (0.90, 0.94, 0.99, 1.04, 1.09, 1.13, 1.09, 1.04, 0.99, 0.94, 0.90, 0.88)

for index, (size, light) in enumerate(zip(sizes, brightness)):
    frame_orb = orb.resize((size, size), Image.Resampling.NEAREST)
    frame_orb = frame_orb.rotate(
        index * 30,
        resample=Image.Resampling.NEAREST,
        expand=False,
        fillcolor=(0, 0, 0, 0),
    )
    luminance = ImageEnhance.Brightness(frame_orb.convert("L")).enhance(light)
    alpha = frame_orb.getchannel("A").point(lambda value: 255 if value >= 128 else 0)
    frame_orb = Image.merge("RGBA", (luminance, luminance, luminance, alpha))

    frame = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    frame.alpha_composite(frame_orb, ((32 - size) // 2, (32 - size) // 2))
    sheet.alpha_composite(frame, (index * 32, 0))

sheet.save(SHEET)
sheet.resize((3072, 256), Image.Resampling.NEAREST).save(PREVIEW)

print(f"Wrote {SHEET}")
print(f"Wrote {PREVIEW}")
