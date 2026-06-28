- 날짜: 2026-06-26
- 영역: 엔진
- 상태: 미검토
- 출처: [[2026-06-26]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
프로젝트 루트를 `EngineInitialize(..., root)`로 넘겨 놓은 상태에서 Clear 이후 재시작 흐름도 같은 루트 기준으로 돌아가는지 엔진 코드를 확인했다.

## 문제
`PixelSolution/PixelEngine/PixelEngine.cpp`는 `Initialize()`에서 전달받은 `assetPath`를 `EngineRootFolderPath`에 저장하지만, `EngineUpdate()`에서 `IsClear`가 켜진 뒤 다시 시작할 때는 `Import("./Asset/main.lua")`를 하드코딩해서 루트를 무시한다. `PixelSolution/PixelEngine/PhysManager.cpp`가 레이어 파일을 `Engine->GetEngineRootFolderPath() + "/Engine/LayerMatrix.json"`로 읽는 것과 비교하면, 메인 스크립트만 상대경로에 묶여 있는 셈이다. 작업 디렉터리를 우연히 프로젝트 루트로 맞춰 둔 환경에서는 지나가지만, Clear 재시작 자체는 설정된 프로젝트 경로보다 현재 프로세스 위치에 더 강하게 의존한다.

## 제안
Clear 이후 메인 스크립트 재로드도 `EngineRootFolderPath`를 기준으로 통일하는 편이 낫다. 엔진이 이미 프로젝트 루트 개념을 들고 있으니, `main.lua` 경로도 같은 정책으로 계산해야 재시작 동작이 에디터/클라이언트 실행 위치에 흔들리지 않는다.
