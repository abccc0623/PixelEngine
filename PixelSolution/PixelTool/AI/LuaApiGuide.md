# PixelEngine Lua API 가이드

이 문서는 PixelTool AI 채팅에 제공되는 공통 API 지침이다. 엔진 DLL이나 게임 Asset에 복사하지 않는다.
모델을 별도로 학습시키는 자료가 아니며, 이 문서가 파일 변경이나 명령 실행 권한을 부여하지 않는다.

## API를 확인하는 기준

- 사용자가 연 프로젝트의 `Engine/*.lua`에 선언된 함수, 매개변수, 반환값을 먼저 확인한다.
- 이 문서와 실제 API가 다르면 차이를 알리고 해당 프로젝트의 엔진 버전을 확인한다.
- 존재하지 않는 API를 추측하지 않는다. 다른 엔진의 함수명을 그대로 사용하지 않는다.
- 엔진 내부 C++ 소스가 사용자 PC에도 있다고 가정하지 않는다.
- 배포 환경에서는 엔진 DLL 내부의 분석·변경을 요청하거나 시도하지 않는다. 지원하지 않는 API는 미지원이라고 알리고, 현재 제공 API와 Lua로 가능한 구현을 검토한다. 엔진 내부 분석·수정 요청에 대한 거절 규칙은 `AIRestrictions.md`를 따른다.
- 프로젝트에 `AGENTS.md`와 추가 사용 설명서가 있으면 함께 확인한다.

## 실행 진입점과 씬

- 프로젝트 실행 진입점은 `Asset/main.lua`이며, 엔진은 이 파일을 읽은 뒤 전역 함수 `Main()`을 호출한다. 메인 파일은 기본 Lua 문법으로 작성한다.
- 엔진의 객체는 씬에 속한다. 현재 씬이 없는 상태에서 메인에서 객체를 생성하면 엔진이 내부 기본 씬을 준비하고 그 씬에 객체를 생성한다.
- 씬 파일을 만들거나 불러와 등록하는 것만으로 해당 씬이 실행되는 것은 아니다. 메인의 실행 흐름에서 Lua API `Scene.Change("씬 이름")`로 사용할 씬을 지정해야 한다.
- 직접 등록한 씬으로 먼저 변경했다면 이후 객체는 그 씬에 생성되므로 객체 생성을 위해 기본 씬을 따로 만들지 않는다. 씬을 변경하지 않은 채 메인에서 생성하는 객체는 기본 씬에 속한다.
- 이미 기본 씬에서 객체를 만든 뒤 다른 씬으로 변경한다고 해서 기존 객체가 새 씬으로 자동 이동하는 것은 아니다.
- 에디터의 씬 파일 기본 형식은 `local Scene = {}` 테이블에 함수를 정의하고 마지막에 `return Scene`으로 반환하는 형태다. 메인의 전역 `Main()`과 씬의 콜론 메서드를 혼동하지 않는다.
- 현재 씬 템플릿에는 `Scene:Start()`와 `Scene:Release()`가 있다. `Start`는 해당 씬으로 변경할 때 호출된다. 현재 엔진 구현에서는 Lua `Release` 호출이 비활성화되어 있으므로 템플릿에 있다는 이유만으로 종료 시 호출된다고 안내하지 않는다.

## 객체와 컴포넌트

- Entity는 이 엔진에서 가장 기본이 되는 객체이며, 엔진은 ECS(Entity·Component·System) 패턴으로 구성된다.
- 스크립트를 사용하는 객체의 기본 생성 방식은 `Entity.Create("스크립트 이름")`이다. 엔티티 하나와 연결된 Lua 객체 스크립트(`.pxm`) 하나가 한 쌍을 이룬다. 여러 Lua 파일을 하나로 합친다는 뜻이 아니다.
- 해당 객체 고유의 초기화와 동작 로직은 연결된 `.pxm` 스크립트 내부에 작성한다. 생성하는 쪽의 main·씬·다른 객체 스크립트는 생성 및 필요한 설정 전달을 담당하며, 생성된 객체의 동작 로직을 대신 구현하지 않는다.
- 객체의 기본 초기값 설정과 `Transform.Add(self.ID)`, `UIImage.Add(self.ID)` 등 필요한 컴포넌트 추가는 해당 `.pxm`의 `function Script:Spawn()`에서 수행한다. 필요한 컴포넌트는 객체의 역할에 맞게 선택하며, 모든 객체에 같은 컴포넌트를 무조건 추가하는 것은 아니다.
- 이동속도·생성 주기 등 객체별 설정값과 상태의 기본값은 스크립트 위쪽의 별도 변수에 모으기보다 `Spawn()` 안에서 `self.moveSpeed`, `self.spawnInterval` 같은 self 필드로 초기화한다. 여러 객체가 공유하는 아이템·데이터 리스트와는 구분한다.
- 일반 객체의 이미지 표시는 Renderer 계열을 사용한다. 현재 2D 이미지 객체에는 `Renderer2D.Add(self.ID)`를 사용하며, `Renderer.Add`라는 이름을 임의로 만들어 쓰지 않는다. 현재 프로젝트의 `Engine` API 선언을 먼저 확인한다.
- `Renderer2DData:SetColor(R, G, B, A)`의 색상과 알파 값은 모두 `0~255` 기준이다. `0~1`로 정규화해서 전달하지 않는다. 예를 들어 불투명한 흰색은 `renderer:SetColor(255, 255, 255, 255)`, 불투명한 빨간색은 `renderer:SetColor(255, 0, 0, 255)`다. 엔진 내부에서 각 값을 `0~255`로 제한한 뒤 255로 나누어 처리한다.
- UI 생성 요청에는 UI 계열 컴포넌트를 사용한다. UI 이미지는 `UIImage.Add(self.ID)`, UI 텍스트는 `UIText.Add(self.ID)`처럼 용도에 맞게 선택한다. 일반 객체와 UI를 혼동해 렌더링 컴포넌트를 서로 대체하지 않는다.
- 객체 생성 요청은 `.pxm` 스크립트와 `Entity.Create("스크립트 이름")`을 사용하는 것이 기본이다. 사용자가 스크립트 없는 객체를 명시하지 않았다면 스크립트 사용 여부를 매번 묻지 않는다. 생성 전에 스크립트가 프로젝트의 `Asset.Import` 경로에 포함되는지 확인한다.
- 기본적인 객체 식별 방식은 ID다. ID와 컴포넌트 데이터를 혼동하지 않는다.
- `Entity.Create("스크립트 이름")`은 스크립트 객체의 ID를 반환한다.
- `Entity.Create`는 내부에서 객체를 만들고 해당 스크립트의 `Spawn()` 실행까지 완료한 뒤 ID를 반환한다. 따라서 반환 직후 `Entity.GetScript(ID)`로 스크립트를 가져와 필요한 함수를 호출할 수 있다.
- `Entity.CreateEmpty()`는 빈 객체의 ID를 반환한다.
- 필요한 컴포넌트는 `Transform.Add(ID)`, `Camera.Add(ID)`, `UIImage.Add(ID)`처럼 추가한다.
- 추가된 컴포넌트는 `Transform.Get(ID)`, `Camera.Get(ID)` 등으로 조회한다.
- 객체 생성 실패 시 사용하는 잘못된 ID 값은 `4294967295`다. 편의 함수는 실패 시 nil을 반환할 수도 있으므로 실제 선언을 확인한다.
- `Entity.SetActive(ID, false)`는 삭제가 아니라 비활성화다.
- 비활성화된 객체는 Lua `Update`뿐 아니라 엔진 ECS System에서 처리하는 충돌·애니메이션 등 객체의 동작도 멈춘다. 비활성화를 삭제로 간주하거나 비활성 상태에서도 이러한 동작이 계속된다고 가정하지 않는다.
- 객체 삭제 후 기존 컴포넌트 데이터/포인터를 계속 사용하지 않는다. Lua 변수에 남아 있다고 객체가 살아 있는 것은 아니다.
- 객체 삭제에는 `Entity.Destroy(ID)`를 사용한다. 삭제 작업을 구현할 때 해당 ID나 `Entity.GetScript(ID)`로 얻은 Lua 테이블을 보관·사용하는 다른 코드도 확인하여, 삭제 후 접근이나 남은 참조로 문제가 생기지 않도록 정리한다.

## 카메라 생성

`Entity.CreateCamera()`는 객체에 Transform과 Camera를 추가하고 **객체 ID**를 반환한다. CameraData를 반환하지 않는다.

```lua
local cameraID = Entity.CreateCamera()
if not cameraID then
	return
end

local transform = Transform.Get(cameraID)
transform.position.z = -20

local camera = Camera.Get(cameraID)
camera.ZoomLevel = 50
```

카메라를 만들기 위해 별도의 카메라 `.pxm` 스크립트가 반드시 필요한 것은 아니다.

## 스크립트와 데이터

- 객체 간 함수 호출은 `local target = Entity.GetScript(targetID)`로 스크립트 테이블을 가져와 `target:FunctionName(...)`처럼 직접 호출하는 방식을 사용한다. 실제 함수 정의의 점/콜론 방식과 매개변수를 확인한다. 필드도 가져온 스크립트 테이블에서 직접 읽고 쓴다.
- 다른 객체의 Lua 테이블을 불필요하게 장기 보관하지 않고 필요할 때 `Entity.GetScript(ID)`로 조회한다. 엔진 DLL에 포함된 `Setting.lua`의 `LuaManager.entities`가 Lua 객체 ID와 스크립트 테이블의 등록·제거를 관리한다. 그 밖의 게임별 참조·목록·상태 관리는 클라이언트가 담당하며 엔진이 자동으로 정리한다고 가정하지 않는다. 활성 상태 확인은 기존 API를 사용하고 같은 역할의 함수를 불필요하게 새로 만들지 않는다.
- `.pxm`·`.scene` 파일은 직접 작성해도 된다. 새 파일의 기본 문법·함수 순서·초기화 위치·주석은 에디터의 해당 생성 템플릿과 맞춘다. 객체 스크립트의 현재 기준은 툴바 `CreateModule()`이 사용하는 `LuaCode/Module.lua`로, `local Script = {}` → `Script:Spawn()`(내부에서 `self.transform = Transform.Add(self.ID)`) → `Script:Update(DTime)` → `return Script` 순서다. 템플릿이 변경되면 최신 내용을 확인하고, 존재하지 않는 기본 함수나 주석을 임의로 추가하지 않는다.
- `Asset.Import`로 필요한 파일 또는 폴더를 가져온다. 폴더를 가져오면 하위 폴더도 탐색한다.
- 로딩 순서는 `Asset.Import`의 호출 순서를 기준으로 정한다. 먼저 호출한 임포트가 끝난 뒤 다음 작업으로 진행하므로, 객체가 사용하는 전역 데이터 테이블을 먼저 임포트하고 필요한 스크립트를 임포트한 다음 객체를 생성한다. 이 순서라면 `Spawn()`에서 이미 로딩된 전역 테이블을 사용할 수 있다. 단일 폴더 내부의 파일 탐색 순서까지 의존성 순서로 보장된다고 확대 해석하지 않는다.
- 게임 스크립트를 작성할 때 `require`, `dofile`, `loadfile` 등으로 파일을 직접 불러오는 방식은 최대한 사용하지 않는 것을 기본으로 한다. 필요한 스크립트·전역 테이블은 엔진이 제공하는 `Asset.Import`와 기존 로딩 흐름을 통해 사용하고, 별도의 Lua 모듈 로더나 중복 로딩 코드를 추가하지 않는다. 해당 파일이 엔진 임포트 경로에 포함되는지 먼저 확인한다.
- 외부 라이브러리 등 별도 로딩이 실제로 필요한 경우에는 필요성과 엔진 로더로 처리할 수 없는 이유를 확인한 뒤 사용한다. 이 규칙은 사용자가 작성하는 게임 스크립트의 기본 방침이며, 엔진 제공 파일 내부의 `require("ffi")` 같은 필수 구현을 삭제하라는 뜻이 아니다.
- 엔진 객체 스크립트는 `local Script = {}`를 만들고 마지막에 `return Script`로 반환하는 형태를 사용한다.
- `Spawn`은 객체 생성 시 초기화를 담당하고, `Update(DTime)`은 프레임 갱신을 담당한다.
- `Spawn()`은 최초 생성 시 한 번만 호출된다. 풀에서 재활성화할 때 다시 호출된다고 가정하지 않는다. 별도의 재설정 함수가 필요하면 클라이언트에서 구현하며, `Setting`이나 `Initialize` 같은 이름은 사용자 정의일 뿐 엔진의 필수 함수가 아니다.
- Lua 함수/조건문/반복문은 여러 줄로 작성하고 본문은 탭으로 들여쓴다.
- 함수와 변수 이름은 반드시 영어로 작성한다. 함수 역할 설명 등 주석은 한국어로 작성한다.
- 아이템 리스트·데이터 리스트처럼 여러 곳에서 전역으로 사용하는 공통 데이터 테이블은 `Asset/Table/` 하위의 별도 `.lua` 파일로 관리한다. 객체 스크립트와 게임 로직을 두는 `Asset/Script/`와 분리한다. 이 규칙을 모든 Lua 테이블이나 공통 동작 로직까지 옮기라는 뜻으로 확대하지 않으며, 객체 내부 상태와 고유 로직은 해당 객체 스크립트 안에 둔다.
- `Asset/Table/`은 기획자가 수정할 데이터 중심으로 구성하고 함수는 원칙적으로 제외한다. 처리 본문이 한 줄 정도인 단순 조회 함수는 허용하지만, 복잡한 계산이나 게임 동작 로직은 `Script`에 둔다. 이는 함수 정의를 한 줄로 축약해도 된다는 의미가 아니며, 짧은 함수도 설명 주석과 여러 줄 함수 형식을 유지한다.
- Lua 스크립트(`.lua`, `.pxm`, `.scene`)를 작성하거나 수정할 때 코드의 중첩 단계에 맞춰 탭으로 들여쓰고 줄맞춤을 일관되게 유지한다. 함수·조건문·반복문의 본문과 닫는 `end`의 위치를 명확히 구분한다. 요청과 무관한 기존 코드 전체를 재정렬하지는 않는다.
- 새로 작성하거나 수정하는 각 함수 바로 위에는 어떤 역할을 하는 함수인지 짧은 한국어 설명 주석(`---`)을 추가한다. `Spawn`, `Update` 같은 기본 함수도 포함하며, 기존의 정확한 설명은 보존·보완한다. 함수 내용과 다른 설명이나 의미 없는 상투적 주석은 넣지 않는다.
- 키 입력에 따른 객체의 동작은 보통 해당 객체의 스크립트 내부에서 구현한다. 별도의 중앙 입력 관리 구조를 임의로 강제하지 않는다.
- 함수 정의와 호출에서 점(`.`)과 콜론(`:`)을 맞춘다. 콜론은 self를 전달한다.
- 일반적인 Lua 배열은 1부터 시작한다. ID, 슬롯 번호, 아이템 번호가 모두 같은 인덱스라고 가정하지 않는다.

## UI

- UI를 만들 때 빈 객체에 Transform과 UIImage 또는 UIText를 추가할 수 있다.
- 위치와 크기는 부모 Transform의 영향을 받는다. 로컬 위치와 화면 위치를 구분한다.
- 부모에 연결된 Transform의 `position`은 부모 기준의 로컬 좌표다. 부모를 연결하는 순간 기존 화면 위치가 유지되는지는 별도 동작이므로 필요할 때 실제 API 구현을 확인한다.
- 이미지와 텍스트의 피벗은 필요한 기준에 맞춰 명시한다. 중앙 기준은 `(0.5, 0.5)`다.
- 같은 자식을 반복해서 연결하거나 부모자식 관계에 순환을 만들지 않는다.
- 게임이 멈춘 상태에서도 동작해야 하는 UI는 실제 시간 사용 API가 있는지 확인한다.

## 시간과 게임 일시정지

- 게임 시간은 `Time.SetPaused(true)`로 멈추고 `Time.SetPaused(false)`로 재개한다. 상태 확인은 `Time.IsPaused()`를 사용한다.
- 일시정지 중 `Time.GetDeltaTime()`과 Lua `Update(DTime)`의 `DTime`은 0이다. 플레이 모드의 활성 객체는 `Update` 호출 자체가 중단되는 것이 아니므로, 시간과 무관하게 값을 바꾸는 로직까지 자동으로 멈춘다고 가정하지 않는다.
- 일시정지 중에도 진행할 UI·빌드 테스트 등에는 `Time.GetUnscaledDeltaTime()`을 사용한다. `Time.GetTotalTime()`도 일시정지에 영향받지 않는 경과 시간이므로 게임 진행 시간으로 그대로 사용하지 않는다.
- 게임 시간 일시정지와 객체 비활성화는 다르다. 일시정지는 시간 증가량을 0으로 만들고, 객체 비활성화는 해당 객체의 동작을 중단한다.

## 트윈

- `Tween.Move(ID, Property, StartValue, EndValue, Duration, UseUnscaledTime, Ease)`는 지정 시간 동안 한 번 이동한다.
- `Tween.PingPong(ID, Property, MinValue, MaxValue, Duration, UseUnscaledTime, Ease)`는 두 값 사이를 반복한다. Duration은 편도 시간이다.
- Property는 `TweenProperty`의 Position/Rotation/Scale X/Y/Z를 사용한다.
- UseUnscaledTime이 true면 게임 시간 정지와 관계없이 진행한다.
- Ease는 마지막 선택 인수이며 생략하면 `TweenEase.Linear`다. `In`, `Out`, `InOut`도 지원한다.
- 같은 ID와 속성으로 새 트윈을 등록하면 이전 트윈을 교체한다. 다른 속성은 함께 실행할 수 있다.
- `Tween.Pause`, `Tween.Resume`, `Tween.Kill`은 ID와 속성을 받는다. Kill은 현재 값을 유지한 채 종료한다.
- 객체 비활성화 시 진행 시간과 값 갱신이 멈추고, 다시 활성화하면 이어서 진행한다. 객체 삭제 시 해당 객체의 트윈은 제거된다.

```lua
Tween.Move(targetID, TweenProperty.PositionX, 0, 100, 1, true, TweenEase.InOut)
```

## 컴포넌트 API 공통 사용법

- 현재 제공되는 컴포넌트는 `Transform`, `Camera`, `Renderer2D`, `Rigidbody2D`, `Animation2D`, `UIImage`, `UIText`, `UISilder`다. `UISilder`는 현재 API에 실제 등록된 철자이므로 임의로 `UISlider`로 바꾸지 않는다.
- 각 컴포넌트는 `이름.Add(ID)`, `이름.Get(ID)`, `이름.Has(ID)`를 제공한다. Add/Get의 반환값은 객체 ID가 아니라 컴포넌트 데이터다. 생성된 데이터를 통해 `image:SetTexture(...)`처럼 콜론 메서드를 호출한다.
- `Transform` 데이터는 `position`, `rotation`, `scale`을 제공한다. 벡터 필드는 소문자 `x`, `y`, `z`다. `transform:SetParent(parentID)`와 `transform:SetChild(childID)`는 ID로 부모·자식을 연결한다.
- 카메라 데이터에는 `Projection`, `FovY`, `NearZ`, `FarZ`, `ZoomLevel`, `ViewportX`, `ViewportY`, `ViewportWidth`, `ViewportHeight`가 있다. 대소문자를 유지하고 투영 방식은 프로젝트의 `ProjectionType` 선언을 확인한다.

## 객체 조회와 편의 함수

- 활성 상태 조회의 실제 API 이름은 `Entity.GetActive(ID)`다. 별도 근거 없이 `Entity.IsActive`나 전역 `isActive`를 만들어 사용하지 않는다. 활성 상태와 객체 존재 여부는 서로 다른 개념이다.
- `Entity.GetScript(ID)`는 등록된 객체의 Lua 테이블을 반환하고, 등록되지 않은 ID는 `nil`을 반환한다. 컴포넌트 데이터가 아니다.
- `Entity.Create2DImage(textureName)`는 Transform과 UIImage가 붙은 객체를 생성하지만 반환값은 **UIImageData**다. 객체 ID를 반환하는 `Entity.CreateCamera()`와 혼동하지 않는다.

## 이미지와 텍스트

- `Renderer2DData:SetTexture(textureName)`는 일반 객체의 텍스처를 설정한다. UV 관련 필드는 `TilingX`, `TilingY`, `OffsetX`, `OffsetY`이고, `SetTransparent(boolean)`도 제공한다.
- `UIImageData:SetTexture(textureName)` 이후 `SetTextureSize(width, height)`로 표시 크기를 직접 지정한다.
- `UIImageData:SetAspectFit(slotWidth, slotHeight)`는 원본 전체 이미지 비율을 유지하며 슬롯 안에 맞춘다. `SetContentAspectFit(slotWidth, slotHeight)`는 투명 여백을 제외한 콘텐츠 영역을 기준으로 맞추며 UV 범위도 변경한다. 애니메이션 UV와 섞어 쓸 때는 덮어쓰기 여부를 확인한다. 두 함수는 성공 여부를 반환한다.
- UIImage는 `SetPivot(x, y)`, `SetTiling(x, y)`, `SetOffset(x, y)`, `SetOrder(order)`, `SetColor(r, g, b, a)`를 제공한다. 피벗은 `0~1`, RGBA는 `0~255` 기준이다.
- UIText는 `SetText(text)`, `SetFont(fontName)`, `SetTextSize(scale)`, `SetWrapWidth(width)`, `SetPivot(x, y)`, `SetColor(r, g, b, a)`를 제공한다. 텍스트 크기의 인수는 배율이며 RGBA는 `0~255`다.
- `UITextData:SetWrapWidth(width)`는 자동 줄바꿈 폭을 설정한다. 양수가 아닌 값은 폭 제한을 해제한다. 줄바꿈 폭과 줄높이는 다른 기능이므로 존재하지 않는 줄높이 API를 추측하지 않는다.
- `UISilderData`는 `SetRange(minValue, maxValue)`, `SetValue(value)`, `SetSize(width, height)`, `SetBackgroundColor(r, g, b, a)`, `SetFillColor(r, g, b, a)`, `SetBackgroundTexture(name)`, `SetFillTexture(name)`, `SetOrder(order)`를 제공한다.

## 2D 애니메이션

- 화면 표시용 Renderer2D 또는 UIImage와 함께 Animation2D를 사용한다. Animation2D만 추가했다고 이미지가 표시된다고 가정하지 않는다.
- `animation:Create(textureName, maxFrameX, maxFrameY, speed)`는 시트를 가로·세로 프레임 수로 나누고 애니메이션 인덱스를 반환한다. `speed`를 초 단위 재생 시간이라고 단정하지 않는다.
- `animation:CreateRegion(textureName, startX, startY, frameWidth, frameHeight, frameCount, speed)`는 큰 시트에서 지정한 픽셀 영역부터 가로로 연속된 프레임을 사용한다. 시작 위치와 프레임 크기는 픽셀 단위다.
- `Play(animationIndex)`, `Stop()`, `Reset()`으로 재생을 제어한다. 애니메이션 인덱스와 프레임 인덱스는 **0부터**다. `GetFrameIndex(animationIndex)`, `SetFrameIndex(animationIndex, frameIndex)`도 제공한다.
- `SetLoop(animationIndex, loop)`는 반복 여부, `SetUseUnscaledTime(true)`는 게임 시간 정지와 무관한 진행을 설정한다.
- `SetFrameCallback(animationIndex, frameIndex, repeat)`로 프레임 콜백을 설정하며 스크립트 함수 형식은 `Script:OnAnimationCallBack(targetID, nowAnimationID, nowAnimationFrame)`이다. 철자와 대소문자를 유지한다.

## 입력과 이벤트

- `Input.Key(Keycode.W)`는 누르고 있는 상태, `Input.KeyDown(Keycode.W)`는 누른 순간, `Input.KeyUp(Keycode.W)`는 뗀 순간을 조회한다. 문자열 `"W"`나 Windows 키 숫자를 임의로 전달하지 않고 `Keycode` 열거형을 사용한다.
- 마우스 위치는 `Input.GetMousePositionX()`와 `Input.GetMousePositionY()`로 각각 조회한다. 이 값을 월드 좌표나 부모 기준 UI 좌표로 바로 간주하지 않는다.
- `Event.Bind(ID, eventName, bindFunctionName)`로 스크립트 함수를 이벤트에 연결하고 `Event.Call(eventName)`로 호출한다. 두 함수는 성공 여부를 반환한다. 등록 함수는 함수 이름 문자열로 전달한다.

## 오브젝트 풀과 그룹

- Pool과 Group은 현재 씬의 관리자를 사용한다. 씬을 넘어 자동으로 유지되는 전역 객체 저장소라고 가정하지 않는다.
- `Pool.Active(scriptName)`는 풀 객체를 활성화하고 **ID**를 반환한다. 남은 비활성 객체가 없으면 확장 수만큼 새로 생성한다. `Pool.SetExpansionSize(scriptName, createCount)`로 확장 수를 설정하며 양수를 사용한다.
- `Pool.Disable(scriptName, ID)`는 해당 풀로 반납하며 비활성화한다. 풀 반납을 `Entity.Destroy`로 대체하지 않는다. 재사용 시 위치·체력 등 이전 사용 상태는 클라이언트에서 재설정한다.
- `Pool.GetActiveCount(scriptName)`와 `Pool.GetActiveID(scriptName, index)`로 활성 객체를 조회한다. index는 **0부터 count-1까지**다. 순회 중 반납하면 목록이 변하므로 누락이 생기지 않도록 순회 방식을 정한다.
- 현재 `Pool.Clear(scriptName)`는 활성·비활성 목록을 비울 뿐 객체를 Destroy하거나 자동으로 비활성화하지 않는다. 일괄 삭제 함수라고 안내하지 않는다.
- `Group.CreateGroupAndEntity(groupName, scriptName)`는 객체를 생성해 그룹에 넣고 ID를 반환한다. `Group.Set(groupName, ID)`는 기존 ID 등록, `Group.Remove(groupName, ID)`는 등록 해제, `Group.Clear(groupName)`는 목록 비우기에 사용한다. 등록 해제와 객체 삭제를 혼동하지 않는다.
- 그룹 조회에는 `Group.GetCount(groupName)`와 `Group.GetID(groupName, index)`를 사용한다. index는 **0부터 count-1까지**다. 기존 `Group.Count`, `Group.First`도 있지만 `Group.First`는 빈 그룹의 반환값이 구현상 보장되지 않으므로 빈 그룹에 호출하지 않는다.

## 물리와 충돌

- Rigidbody2D 데이터는 `CreateBoxCollider(size, center)`, `CreateCircleCollider(radius, center)`로 충돌체를 만들고 `SetMotionType(MotionType)`로 운동 유형을 설정한다. 인수의 벡터 타입과 열거형 값은 현재 Engine 선언을 확인한다.
- 물리 위치·회전·속도는 `SetPosition(position)`, `SetRotation(rotation)`, `SetVelocity(velocity)`로 설정한다. 물리 객체의 Transform만 바꿨을 때 물리 상태까지 자동 변경된다고 가정하지 않는다.
- `SetGravity`, `SetRestitution`, `SetFriction`, `SetLinearDamping`, `SetSensor`, `SetActive`, `SetLayer(layerName)`, `GetLayer()`를 제공한다. Rigidbody2D의 SetActive와 Entity 전체 활성화는 다른 범위의 API다.
- 충돌 이벤트 스크립트 형식은 `Script:OnCollisionEnter(targetID)`와 `Script:OnCollisionExit(targetID)`다. 상대 객체 ID를 받는다.

## 벡터·로그·엔진 설정

- `PVector2(x, y)`, `PVector3(x, y, z)`로 벡터를 생성한다. PVector3는 `Normalize`, `Length`, `Distance`, `Dot`, `Cross`, `Direction`, `Lerp`, `ApproximatelyEquals` 메서드를 제공한다. 테이블이나 nil을 벡터처럼 연산하지 않는다.
- 로그는 `Debug.Log(text)`, `Debug.LogWarning(text)`, `Debug.LogError(text)`를 사용한다. 존재하지 않는 `Debug.LogInfo`를 추측하지 않는다. 문자열이 아닌 값을 출력할 때는 필요한 경우 `tostring`을 사용한다.
- `Debug.Line(startPoint, endPoint, color)`로 디버그 선을 요청할 수 있다. 인수 타입은 프로젝트 Engine 선언을 확인한다.
- 엔진 전역 설정에는 `Engine.BackgroundColor(r, g, b)`와 `Engine.SetWindowSize(width, height)`가 있다. 창 크기 변경을 에디터 도킹 패널 크기 변경 기능으로 사용하지 않는다.

## 문서의 범위와 버전 확인

- 이 목록은 실제 바인딩·생성 Lua 선언·Setting.lua 및 관련 구현을 읽어 정리한 사용 안내다. 함수가 선언되어 있다는 사실과 런타임 정상 동작 검증은 구분한다.
- Engine 자동 생성 파일과 이 가이드가 다르면 현재 프로젝트 선언을 확인하고 버전 차이를 알린다. 없는 기능을 비슷한 이름으로 만들어 호출하지 않는다.

