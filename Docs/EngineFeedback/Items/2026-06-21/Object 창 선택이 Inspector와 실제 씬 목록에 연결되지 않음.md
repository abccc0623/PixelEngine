- 날짜: 2026-06-21
- 영역: 에디터
- 상태: 미검토
- 출처: [[2026-06-21]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
에디터에서 씬 오브젝트를 선택하고 Inspector에서 컴포넌트를 조정하는 흐름이 현재 어디까지 연결돼 있는지 보려고 `PixelTool/Tool/ObjectWindow/ObjectWindow.xaml.cs`와 `PixelTool/Tool/InspectorWindow/InspectorWindow.xaml.cs`를 확인했다.

## 문제
`ObjectWindow`의 `UpdateScene()`와 선택 변경 처리 코드는 전부 주석 처리돼 있고, `InspectorWindow`도 `ModuleContainer.ItemsSource`와 `RefreshInspector` 경로가 비활성화돼 있다. 창은 도킹 레이아웃에 존재하지만 실제로는 씬 계층 목록과 선택 오브젝트 편집이 이어지지 않아, 사용자는 패널이 아직 비활성 상태인지 단순히 현재 씬이 비어 있는지 구분하기 어렵다.

## 제안
최소한 Object 창에 현재 미구현 상태를 명시하거나, 씬 오브젝트 목록과 Inspector 갱신 중 하나만이라도 먼저 살아 있는 경로로 연결하면 좋겠다. 편집 불가 상태를 숨기기보다 명확히 보여주는 편이 에디터 신뢰도를 지키기 쉽다.
