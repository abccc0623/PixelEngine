---
name: client-image-assets
description: Create, edit, or integrate existing bitmap assets for the Survival client when the user requests game images, UI art, icons, tiles, or visual variants. Do not use for engine code or code-native UI changes.
---

# Client image assets

- Work in `D:\PixelEngine\Survival`; game images belong under `Survival\Asset\Image`. Do not place generated assets in `PixelSolution` or the repository root.
- Check a small set of relevant existing images first. Match the requested size, transparency, pixel density, palette, and dark-fantasy rune style when those details matter. Preserve the user's explicit visual direction over existing style.
- For item images, use a fully transparent background and a 512 × 512 pixel canvas unless preserving a purchased asset as specified below. Keep the item within the canvas without stretching its proportions. This rule applies only to item images.
- Use the available `imagegen` skill and its image tool for bitmap generation or edits. For an edit, inspect the target image first. Use code-native changes instead when the request concerns only UI layout, colors, or simple geometric shapes.
- Show the generated result for review. If it is for the game, copy the selected final file into the relevant `Survival\Asset\Image` subfolder using a clear, unique name; do not overwrite an existing image without a request to replace it.
- Verify the saved file's dimensions and transparency, and check that its name matches the texture name the client uses. Update Lua or scene references only when the user asked to integrate the asset.
- Keep drafts and temporary outputs outside the repository. Do not add unused variants to the game folder.

## 구매한 룬 에셋 사용

- 구매한 팩: `48 Magic Runes Pixel Art Icon Pack`. 원본 폴더: `D:\48 Magic Runes Pixel Art Icon Pack\PNG\Transperent\runes+bricks` (`Transperent`는 실제 폴더명).
- 사용할 이미지만 `D:\PixelEngine\Survival\Asset\Image\Items`에 복사하고, `Asset\Table\ItemList.lua`의 `Texture`에는 확장자 없는 복사본 이름을 연결한다. 외부 구매 폴더를 런타임 경로로 참조하지 않는다.
- 이 팩은 원본 32×32 픽셀 크기와 투명도를 유지한다. 단순 복사·연결 요청에서는 이미지 생성, 확대, 재채색을 하지 않는다.
- 이 구매 룬 팩을 자동으로 선택·연결하는 대상은 **아이템 이름에 `생성기`가 들어가는 경우에만** 해당한다. 설치 종류라는 이유만으로 반응기·분배기·분할기 등 다른 명칭의 아이템에 적용하지 않는다.
- 생성기에 연쇄·전기 기능을 표현할 때는 노란색 계열, 폭발·화염 기능을 표현할 때는 빨강·주황색 계열을 고른다.
- 이전 적용 기록: 연쇄 반응기는 `Icon1.png` → `rune_chain_reactor.png`, 폭발 반응기는 `Icon6.png` → `rune_explosion_reactor.png`를 사용했다. 이 기록은 현재 `생성기` 명칭 제한의 예외나 기존 이미지 유지·교체 명령을 뜻하지 않는다.
- 이름을 임의로 `생성기`로 바꾸거나, 이 규칙을 기록했다는 이유로 기존 이미지를 일괄 교체·원복하지 않는다. 실제 변경 요청의 범위에서 적용한다.
- 정적 룬으로 연결할 때 해당 아이템의 이전 스프라이트시트·애니메이션 프레임 설정을 제거하고, 텍스처 이름 중복과 복사본의 투명도를 확인한다.

## 몬스터·캐릭터 원본 에셋

- `D:\TRPG\Tiny RPG Character Asset Pack 01 v2.0 -Full 22 Characters`는 몬스터 또는 캐릭터용 원본 에셋 폴더다. 해당 이미지가 필요할 때 이 경로를 확인한다.
- 게임 적용을 요청받으면 필요한 파일만 `D:\PixelEngine\Survival\Asset\Image`의 해당 몬스터·캐릭터 하위 폴더에 복사해 연결한다. 원본 폴더를 런타임 경로로 직접 사용하지 않는다.
- 캐릭터별 애니메이션 구성과 프레임 크기는 실제 파일을 확인하고 설정한다. 경로 등록만 요청받은 경우 이미지 복사나 기존 몬스터·캐릭터 교체는 하지 않는다.
