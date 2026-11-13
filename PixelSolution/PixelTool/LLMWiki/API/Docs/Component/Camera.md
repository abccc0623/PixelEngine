# PixelEngine Lua component

## Camera

### 언제 사용하는지
장면을 화면에 보이게 할 카메라의 투영과 표시 범위를 설정할 때 사용합니다. `Add`, `Get`, `Has`가 등록되어 있습니다.

### 최소 예제
```lua
function Script:Spawn()
    self.camera = Camera.Add(self.ID)
end
```

### 주의사항
- 주요 필드는 `Projection`, `FovY`, `NearZ`, `FarZ`, `ZoomLevel`과 `ViewportX/Y/Width/Height`입니다. 투영 종류는 프로젝트의 `ProjectionType`을 확인하세요.
- `Entity.CreateCamera()`는 Camera 데이터가 아닌 객체 ID를 반환합니다.
- 배경색 설정만으로 카메라나 다른 객체가 생성되지는 않습니다.

