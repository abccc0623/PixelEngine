- 날짜: 2026-06-28
- 영역: 엔진
- 상태: 미검토
- 출처: [[2026-06-28]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
프로젝트 루트가 `EngineInitialize(..., root)`로 전달된 뒤 첫 SceneView 실행이 그 루트를 일관되게 쓰는지 확인하려고 `PixelTool/Tool/GraphicsWindow/GraphicsWindow.cs`와 `PixelEngine.cpp`를 같이 읽었다.

## 문제
SceneView 시작 시 `PixelEngineNative.EngineInitialize(..., root)`에는 선택한 `ProjectRootPath`가 들어가지만, 바로 뒤에서 첫 스크립트 로드는 `PixelEngineNative.Import("./Asset/main.lua")`로 고정 상대경로를 사용한다. 그래서 프로젝트 루트 개념은 엔진에 이미 전달됐는데도 첫 import만 현재 작업 디렉터리에 묶여 있어서, 다른 실행 위치나 다중 프로젝트 흐름에서는 시작 스크립트 해석 기준이 흔들릴 수 있다.

## 제안
첫 import도 `ProjectPathService.AssetPath`나 엔진의 `EngineRootFolderPath`를 기준으로 통일하면 좋겠다. 초기 실행, Clear 이후 재시작, 물리 설정 JSON 로드가 같은 경로 정책을 따라야 프로젝트 루트 전환과 테스트 환경 변경에 덜 취약해진다.
