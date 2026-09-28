---
name: client-image-assets
description: Create or edit bitmap assets for the Survival client when the user requests game images, UI art, icons, tiles, or visual variants. Do not use for engine code or code-native UI changes.
---

# Client image assets

- Work in `D:\PixelEngine\Survival`; game images belong under `Survival\Asset\Image`. Do not place generated assets in `PixelSolution` or the repository root.
- Check a small set of relevant existing images first. Match the requested size, transparency, pixel density, palette, and dark-fantasy rune style when those details matter. Preserve the user's explicit visual direction over existing style.
- For item images, always use a fully transparent background and a 512 × 512 pixel canvas. Keep the item within the canvas without stretching its proportions. This rule applies only to item images.
- Use the available `imagegen` skill and its image tool for bitmap generation or edits. For an edit, inspect the target image first. Use code-native changes instead when the request concerns only UI layout, colors, or simple geometric shapes.
- Show the generated result for review. If it is for the game, copy the selected final file into the relevant `Survival\Asset\Image` subfolder using a clear, unique name; do not overwrite an existing image without a request to replace it.
- Verify the saved file's dimensions and transparency, and check that its name matches the texture name the client uses. Update Lua or scene references only when the user asked to integrate the asset.
- Keep drafts and temporary outputs outside the repository. Do not add unused variants to the game folder.
