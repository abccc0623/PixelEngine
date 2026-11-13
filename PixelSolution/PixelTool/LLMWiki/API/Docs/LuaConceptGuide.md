# Lua API 개념과 동작 규칙

이 문서는 배포된 PixelTool AI가 Lua API의 공통 동작을 이해할 때 읽는다. API 문서는 위키 검색으로 찾는다. 정확한 매개변수·반환값은 Tool 설치 경로의 `LLMWiki/API/Lua/*.lua` 선언으로 확인한다. 선언, 성공한 리임포트, 실제 실행 검증은 서로 다르다. 엔진 DLL 내부를 추측하거나 수정하지 않는다.

## 실행 흐름과 객체 수명

- 엔진은 `Asset/main.lua`에서 시작해 전역 `Main()`을 호출한다. `Main()`에서 Scene을 만들고 선택한 뒤 객체를 생성해 게임을 구성한다. Scene 파일을 가져오기만 해서는 실행되지 않으며, `Scene.Change`로 선택해야 한다.
- Entity에 연결하는 객체 스크립트 파일의 확장자는 `.pxm`이다. 내부 테이블 이름은 파일명과 관계없이 `Script`를 사용하고, `Spawn()`, `Update(DTime)`을 정의한 뒤 `return Script`로 반환한다. 시작 파일인 `main.lua`와 구분한다.
- 모든 Entity는 Scene에 속한다. Scene을 바꿔도 기존 객체가 자동 이동하지 않는다. Scene의 `Start`는 전환 시 실행되지만 Lua `Release` 호출은 현재 비활성화되어 있다.
- 스크립트가 연결된 객체를 생성하면 `Spawn()`이 반환 전에 한 번 실행된다. 풀에서 재사용할 때는 다시 실행되지 않는다. 객체의 기본 필드와 컴포넌트는 해당 스크립트의 `Spawn()`에서 준비한다.
- 객체 파괴 후 ID·테이블·컴포넌트 참조가 남아 있는지 확인한다. 비활성화는 삭제가 아니다. 비활성 객체의 Lua Update와 ECS 충돌·애니메이션은 중단된다.

## `.pxm` 기본 구조

파일명과 관계없이 테이블 이름은 `Script`다. 새 객체 스크립트는 다음 구조에서 필요한 코드만 추가한다.

```lua
local Script = {}

function Script:Spawn()
    self.transform = Transform.Add(self.ID)
end

function Script:Update(DTime)
end

return Script
```

## Lua API 종류

- `Entity`, `Scene`, `Input`처럼 전역에서 사용하는 API가 있다.
- `Transform`, `Renderer2D`처럼 Entity에 추가하거나 조회해 사용하는 컴포넌트가 있다.
- `PVector2`, `PVector3`처럼 위치와 방향 등의 값을 표현하는 자료형이 있다.
