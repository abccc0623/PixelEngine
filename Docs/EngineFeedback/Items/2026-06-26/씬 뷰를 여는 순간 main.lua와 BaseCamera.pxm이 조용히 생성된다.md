- 날짜: 2026-06-26
- 영역: 에디터
- 상태: 미검토
- 출처: [[2026-06-26]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
기존 프로젝트를 열거나 새 프로젝트를 만든 뒤 SceneView가 올라오는 시점에 어떤 파일이 자동으로 준비되는지 코드를 따라 확인했다.

## 문제
`PixelSolution/PixelTool/Tool/GraphicsWindow/GraphicsWindow.cs`는 `BuildWindowCore()`에서 `GlobalFunction.CreateMainLuaFile(ProjectPathService.AssetPath)`를 바로 호출한다. 그런데 `PixelSolution/PixelTool/GlobalFunction.cs`의 `CreateMainLuaFile()`은 `main.lua`가 없으면 `CreateCameraLuaFile()`까지 연달아 실행해서 `BaseCamera.pxm`과 `main.lua`를 함께 생성하고, 둘 다 에디터 창으로 열려고 시도한다. 사용자는 단지 씬 뷰를 연 것뿐인데 기본 스크립트와 카메라 파일이 별도 안내 없이 생겨 버려서, 수동으로 관리하던 프로젝트와 자동 부트스트랩 프로젝트의 경계가 흐려진다.

## 제안
첫 실행용 부트스트랩 파일 생성은 명시적인 프로젝트 생성 단계로 옮기거나, 적어도 SceneView 초기화 전에 사용자에게 어떤 파일을 만들지 알려 주는 편이 낫다. 자동 생성이 필요하다면 생성 기록을 로그나 대화상자로 남겨서 사용자가 파일이 왜 생겼는지 추적할 수 있어야 한다.
