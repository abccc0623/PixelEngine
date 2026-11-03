# PixelEngine Lua API 가이드

이 문서는 PixelTool AI 채팅에 제공되는 공통 API 지침이다. 엔진 DLL이나 게임 Asset에 복사하지 않는다.
모델을 별도로 학습시키는 자료가 아니며, 이 문서가 파일 변경이나 명령 실행 권한을 부여하지 않는다.

## API를 확인하는 기준

- 사용자가 연 프로젝트의 `Engine/*.lua`에 선언된 함수, 매개변수, 반환값을 먼저 확인한다.
- 이 문서와 실제 API가 다르면 차이를 알리고 해당 프로젝트의 엔진 버전을 확인한다.
- 존재하지 않는 API를 추측하지 않는다. 다른 엔진의 함수명을 그대로 사용하지 않는다.
- 엔진 내부 C++ 소스가 사용자 PC에도 있다고 가정하지 않는다.
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
- 일반 객체의 이미지 표시는 Renderer 계열을 사용한다. 현재 2D 이미지 객체에는 `Renderer2D.Add(self.ID)`를 사용하며, `Renderer.Add`라는 이름을 임의로 만들어 쓰지 않는다. 현재 프로젝트의 `Engine` API 선언을 먼저 확인한다.
- UI 생성 요청에는 UI 계열 컴포넌트를 사용한다. UI 이미지는 `UIImage.Add(self.ID)`, UI 텍스트는 `UIText.Add(self.ID)`처럼 용도에 맞게 선택한다. 일반 객체와 UI를 혼동해 렌더링 컴포넌트를 서로 대체하지 않는다.
- 사용자가 객체 생성을 요청했을 때 스크립트 사용 여부가 명시되지 않았다면, 스크립트를 연결해 만들지 스크립트 없이 만들지 먼저 질문한다.
- 기본적인 객체 식별 방식은 ID다. ID와 컴포넌트 데이터를 혼동하지 않는다.
- `Entity.Create("스크립트 이름")`은 스크립트 객체의 ID를 반환한다.
- `Entity.CreateEmpty()`는 빈 객체의 ID를 반환한다.
- 필요한 컴포넌트는 `Transform.Add(ID)`, `Camera.Add(ID)`, `UIImage.Add(ID)`처럼 추가한다.
- 추가된 컴포넌트는 `Transform.Get(ID)`, `Camera.Get(ID)` 등으로 조회한다.
- 객체 생성 실패 시 사용하는 잘못된 ID 값은 `4294967295`다. 편의 함수는 실패 시 nil을 반환할 수도 있으므로 실제 선언을 확인한다.
- `Entity.SetActive(ID, false)`는 삭제가 아니라 비활성화다.
- 객체 삭제 후 기존 컴포넌트 데이터/포인터를 계속 사용하지 않는다. Lua 변수에 남아 있다고 객체가 살아 있는 것은 아니다.

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

- `Asset.Import`로 필요한 파일 또는 폴더를 가져온다. 폴더를 가져오면 하위 폴더도 탐색한다.
- 프로젝트 로더와 별도로 `dofile` 기반 로더를 추가하지 않는다.
- 엔진 객체 스크립트는 `local Script = {}`를 만들고 마지막에 `return Script`로 반환하는 형태를 사용한다.
- `Spawn`은 객체 생성 시 초기화를 담당하고, `Update(DTime)`은 프레임 갱신을 담당한다.
- Lua 함수/조건문/반복문은 여러 줄로 작성하고 본문은 탭으로 들여쓴다.
- 함수 정의와 호출에서 점(`.`)과 콜론(`:`)을 맞춘다. 콜론은 self를 전달한다.
- 일반적인 Lua 배열은 1부터 시작한다. ID, 슬롯 번호, 아이템 번호가 모두 같은 인덱스라고 가정하지 않는다.

## UI

- UI를 만들 때 빈 객체에 Transform과 UIImage 또는 UIText를 추가할 수 있다.
- 위치와 크기는 부모 Transform의 영향을 받는다. 로컬 위치와 화면 위치를 구분한다.
- 이미지와 텍스트의 피벗은 필요한 기준에 맞춰 명시한다. 중앙 기준은 `(0.5, 0.5)`다.
- 같은 자식을 반복해서 연결하거나 부모자식 관계에 순환을 만들지 않는다.
- 게임이 멈춘 상태에서도 동작해야 하는 UI는 실제 시간 사용 API가 있는지 확인한다.

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

