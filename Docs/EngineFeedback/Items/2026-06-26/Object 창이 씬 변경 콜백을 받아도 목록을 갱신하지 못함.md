- 날짜: 2026-06-26
- 영역: 에디터
- 상태: 미검토
- 출처: [[2026-06-26]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
PixelTool에서 씬을 바꾸거나 새 오브젝트가 생긴 뒤 Object 창에서 현재 씬 계층이 갱신되길 기대하고 확인했다.

## 문제
`PixelSolution/PixelTool/Tool/ObjectWindow/ObjectWindow.xaml.cs`에서는 `RegisterSceneObjectChange` 콜백만 등록하고, 실제 목록을 채우는 `UpdateScene()` 본문은 전부 주석 처리돼 있다. 엔진 헤더 쪽 `GetAllSceneObjects`도 주석 상태라서 현재 구조만으로는 Object 창이 씬 변경 사실을 받아도 목록을 다시 만들지 못한다.

## 제안
씬 계층을 다시 가져오는 최소 API를 먼저 확정하고, 그 전까지는 Object 창에 "현재 씬 목록 동기화 미구현" 같은 안내를 보여 주는 편이 낫다. 콜백만 살아 있고 목록 갱신이 비어 있는 상태를 줄여야 디버깅 흐름이 덜 헷갈린다.
