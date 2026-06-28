- 날짜: 2026-06-26
- 영역: 엔진
- 상태: 미검토
- 출처: [[2026-06-26]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
Lua나 게임 코드에서 이동 보조 모듈을 붙여 쓸 수 있을지 확인하려고 `Movement` 모듈 구현을 따라가 봤다.

## 문제
`PixelSolution/PixelEngine/Export/Module/Movement.h`에는 `MoveToPosition`, `StopMove`, 방향/완료 콜백 등록 같은 API가 드러나 있지만, 실제 구현인 `Movement.cpp`의 `Start()`와 `Update()` 본문은 거의 전부 주석 처리돼 있다. `MoveToPosition()`은 목표 좌표와 비트마스크만 바꾸고, `AddCompleteCallBack()`나 `AddDirectionCallBack()`도 내부 연결 코드가 모두 막혀 있다. 겉으로는 경로 이동 모듈이 준비된 것처럼 보이는데, 실제 런타임에서는 핵심 동작이 빠져 있어 API 존재감과 구현 상태가 크게 어긋난다.

## 제안
아직 미완성이라면 Lua/API 노출 범위를 줄이거나, 로그로 "현재 비활성 구현"임을 명확히 알려 주는 편이 낫다. 계속 유지할 기능이라면 최소한 `MoveToPosition()`이 실제 위치나 물리 속도 변화로 이어지는 가장 작은 경로부터 다시 연결해야 테스트와 문서가 같은 그림을 보게 된다.
