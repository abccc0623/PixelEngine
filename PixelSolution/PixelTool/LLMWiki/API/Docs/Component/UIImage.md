# PixelEngine Lua component

## UIImage

### 언제 사용하는지
화면 UI의 이미지·크기·피벗·표시 순서를 설정할 때 사용합니다. `SetTexture`, `SetAspectFit`, `SetContentAspectFit`, `SetPivot`, `SetOrder` 등이 등록되어 있습니다.

### 최소 예제
```lua
function Script:Spawn()
    self.image = UIImage.Add(self.ID)
end
```

### 주의사항
- UI 좌표는 창 크기와 다른 1920×1080 가상 캔버스를 사용합니다.
- 피벗은 0~1 범위이며 가운데는 `(0.5, 0.5)`입니다.
- 색상 RGBA 값은 0~255입니다.
- `SetAspectFit`은 전체 이미지 비율을 유지합니다. `SetContentAspectFit`은 투명 여백을 제외하고 UV도 바꿉니다.

