# Lua API 문서 색인

API 이름이나 컴포넌트를 여기서 찾은 뒤 필요한 문서만 읽는다. 정확한 이름·인수·반환값은 Tool 설치 경로의 `LLMWiki/API/Lua/*.lua` (`read_engine_guide` / `Editor_LuaGuide_Read`: `Lua/<module>.lua`)에서 다시 확인한다. 이 색인은 설치된 엔진 버전의 API 존재를 보장하지 않는다.

## 컴포넌트

| 이름 | 용도 | 상세 문서 (`read_engine_guide`) |
| --- | --- | --- |
| `Transform` | 위치·회전·크기·계층 | `Doc/Component/Transform.md` |
| `Camera` | 장면 투영·표시 범위 | `Doc/Component/Camera.md` |
| `Renderer2D` | 월드 이미지 | `Doc/Component/Renderer2D.md` |
| `Rigidbody2D` | 물리·충돌 | `Doc/Component/Rigidbody2D.md` |
| `Animation2D` | 스프라이트 애니메이션 | `Doc/Component/Animation2D.md` |
| `UIImage` | 화면 이미지 | `Doc/Component/UIImage.md` |
| `UIText` | 화면 글자 | `Doc/Component/UIText.md` |
| `UISilder` | UI 슬라이더 | `Doc/Component/UISilder.md` |

`UISilder`는 현재 등록된 철자다. 컴포넌트 선언은 선택한 Tool 설치 경로에서 해당 이름의 `LLMWiki/API/Lua/*.lua` (`read_engine_guide` / `Editor_LuaGuide_Read`: `Lua/<module>.lua`) 파일을 찾아 확인한다.

## 그 밖의 Lua API

| 이름·검색어 | 용도 | 상세 문서 (`read_engine_guide`) |
| --- | --- | --- |
| `Entity` | 객체 생성·수명 | `Doc/Other/Entity.md` |
| `Scene` | 장면 전환 | `Doc/Other/Scene.md` |
| `Asset` | 파일·폴더 가져오기 | `Doc/Other/Asset.md` |
| `Input` | 키·마우스 입력 | `Doc/Other/Input.md` |
| `Event` | 이벤트 바인딩·호출 | `Doc/Other/Event.md` |
| `Time` | 시간·일시정지 | `Doc/Other/Time.md` |
| `Tween` | 시간에 따른 값 변화 | `Doc/Other/Tween.md` |
| `Pool` | 객체 재사용 | `Doc/Other/Pool.md` |
| `Group` | 객체 묶음 | `Doc/Other/Group.md` |
| `PVector2` | 2차원 벡터 | `Doc/Other/PVector2.md` |
| `PVector3` | 3차원 벡터 | `Doc/Other/PVector3.md` |
| `Debug` | 로그·디버그 선 | `Doc/Other/Debug.md` |
| `Engine` | 배경색·창 크기 | `Doc/Other/Engine.md` |

색인에 없는 API는 없다고 단정하지 않는다. 이름이나 기능으로 현재 프로젝트의 Engine 선언을 좁혀 검색한다.
