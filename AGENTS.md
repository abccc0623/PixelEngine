# PixelEngine 소스 작업 규칙

## 범위

- 이 규칙은 `PixelSolution`의 엔진·PixelTool 에디터 구현에 적용한다. 게임 프로젝트와 게임 Asset의 콘텐츠 규칙은 포함하지 않는다.
- `PixelSolution/Client`는 엔진 실행 진입점이다. 게임 콘텐츠를 뜻하는 클라이언트와 구분한다.
- 배포된 PixelTool AI 자비스의 지침은 `PixelSolution/PixelTool/AI/`에서 관리한다. 이 루트 문서는 자비스의 실행 규칙이 아니다.
- 사용자가 배포 AI 지침 변경을 요청하면 해당 AI 문서와 실제 도구 호출 경로를 확인한다. 새 엔진 API나 동작을 설명하기 전에도 현재 구현을 확인한다.

## 조사와 수정

- 먼저 관련 프로젝트·파일·함수로 범위를 좁히고 필요한 코드만 읽는다. 긴 로그, 생성물, 바이너리와 저장소 전체 검색은 필요할 때만 확인한다.
- 아래 구조와 구현 메모는 길잡이일 뿐이다. 현재 소스와 다르면 현재 소스를 따른다.
- 기존 사용자 변경을 보존하고 요청 범위 밖의 정리·기능·문서를 추가하지 않는다.
- 코드 변경은 영향에 맞게 검증한다. 문서만 바꾼 경우 빌드하지 않는다. 정적 검사, 빌드, 실제 실행 결과를 구분한다.
- 결과는 한국어로 변경 파일, 확인한 검증, 남은 제한만 짧게 보고한다. 토큰 절약을 이유로 필요한 작업을 생략하지 않는다.

## 소스 구조와 빌드

- 솔루션은 `PixelSolution/PixelSolution.slnx`다. `PixelEngine`은 엔진, `PixelGraphics`는 그래픽스, `PixelMeta`는 메타데이터, `Generate`는 Lua 바인딩 생성, `PixelTool`은 C# WPF 에디터다.
- 네이티브 C++ 프로젝트는 Visual Studio MSBuild로 빌드한다. 구성과 플랫폼은 해당 프로젝트 설정에서 확인한다.

## 구현 메모

- 엔진 Lua API와 컴포넌트는 `PixelEngine`, `PixelMeta`, `Generate`의 등록·생성 경로에서 확인한다. 생성된 `Engine/*.lua` 선언만으로 네이티브 구현을 단정하지 않는다.
- LuaJIT FFI 데이터 구조를 바꿀 때는 메타데이터와 네이티브 구조체의 레이아웃, 정렬, padding을 대조한다.
- 프로젝트 루트는 `ProjectPathService`가 관리한다. `AssetPath`, `EnginePath`, `EditorPath`는 선택된 루트에서 계산한다.
- WPF와 LSP 네임스페이스를 함께 쓰면 `Color` 타입을 명시한다. AvalonDock 탭은 실제 컨트롤과 바인딩 속성을 확인한다.
- 새 대화상자는 기존 `PixelMessageBox`, `PixelPromptDialog`와 공유 스타일을 확인한다.
