- 날짜: 2026-06-26
- 영역: 에디터
- 상태: 미검토
- 출처: [[2026-06-26]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
프로젝트를 바꿔 가며 머티리얼을 만들 때 텍스처 선택 창이 지금 열어 둔 프로젝트의 Asset 폴더에서 바로 시작되는지 확인했다.

## 문제
`PixelSolution/PixelTool/Tool/MaterialWIndow/MaterialWindow.xaml.cs`의 `FindTextureButton()`은 시작 경로를 `AppDomain.CurrentDomain.BaseDirectory/Asset`으로 잡는다. 프로젝트 루트 체계는 이미 `ProjectPathService.AssetPath`로 분리돼 있는데, 머티리얼 텍스처 선택만 실행 폴더 기준을 계속 써서 다른 프로젝트를 열었을 때 탐색 시작 위치가 어긋난다.

## 제안
텍스처 찾기 대화상자의 시작 경로를 `ProjectPathService.AssetPath`로 맞추고, 선택 결과도 현재 프로젝트 Asset 기준 상대 경로로 정리하는 편이 좋다. 그래야 머티리얼 작업이 프로젝트 전환 후에도 같은 감각으로 이어진다.
