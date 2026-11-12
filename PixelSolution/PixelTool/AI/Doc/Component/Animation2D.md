# PixelEngine Lua component

## Animation2D

### 언제 사용하는지
스프라이트 시트의 프레임을 재생하거나 특정 프레임에서 콜백을 받을 때 사용합니다. `Create`, `CreateRegion`, `Play`, `Stop`, `Reset` 등이 등록되어 있습니다.

### 최소 예제
```lua
function Script:Spawn()
    self.animation = Animation2D.Add(self.ID)
end
```

### 주의사항
- `Create`는 시트를 프레임 수로 나누고 `CreateRegion`은 가로로 연속된 프레임을 픽셀 영역으로 지정합니다.
- 애니메이션 번호와 프레임 번호는 0부터 시작합니다.
- `SetLoop`, `SetUseUnscaledTime`, `GetFrameIndex`, `SetFrameIndex`도 등록되어 있습니다.
- 화면에 보이려면 `Renderer2D` 또는 `UIImage`도 필요합니다.
- 프레임 콜백 이름은 `OnAnimationCallBack`의 철자를 그대로 사용합니다.

