# PixelEngine 구현 기록

이 문서는 `PixelSolution`의 엔진·에디터 소스 작업에만 참고한다. 실제 구현과 다르면 현재 소스를 따른다. 공통 작업 규칙은 `AGENTS.md`에 있다.

## 소스 구조와 빌드

- 솔루션: `PixelSolution/PixelSolution.slnx`. Visual Studio에서 연다.
- `PixelEngine`: 엔진 구현. `PixelGraphics`: 그래픽스 구현. `PixelMeta`: 메타데이터. `Generate`: Lua 바인딩 생성. `PixelTool`: C# WPF 에디터. `Client`: 엔진 실행 진입점.
- 네이티브 C++ 프로젝트는 Visual Studio MSBuild로 빌드한다. 구성과 플랫폼은 해당 프로젝트 설정에서 확인한다.

## Lua 바인딩

- 엔진 Lua API와 컴포넌트는 `PixelEngine`, `PixelMeta`, `Generate`의 실제 등록·생성 경로를 확인한다. 생성된 `Engine/*.lua`만으로 네이티브 구현 완료를 단정하지 않는다.
- LuaJIT FFI 데이터 구조를 바꿀 때는 메타데이터, 네이티브 구조체 레이아웃과 생성 결과의 일치를 확인한다. 특히 정렬과 padding이 있는 타입은 소스를 기준으로 검증한다.

## PixelTool 구현

- 프로젝트 루트는 `ProjectPathService`가 관리한다. `AssetPath`, `EnginePath`, `EditorPath`는 선택된 루트에서 계산한다.
- AI 자비스의 배포 문서와 도구 구현은 `PixelTool/AI/`, `PixelTool/Mcp/`에 있다. 이 문서는 그 배포 지침을 대신하지 않는다.
- WPF와 LSP 네임스페이스를 함께 쓰는 파일에서는 `Color` 타입을 명시적으로 한정해 충돌을 피한다.
- AvalonDock 탭 상태를 스타일링할 때는 실제 컨트롤 타입과 바인딩 속성을 확인한다. 일반 WPF `TabItem` 속성을 그대로 가정하지 않는다.
- 새 대화상자는 기존 `PixelMessageBox`, `PixelPromptDialog`와 공유 스타일을 확인한 뒤 구현한다.
