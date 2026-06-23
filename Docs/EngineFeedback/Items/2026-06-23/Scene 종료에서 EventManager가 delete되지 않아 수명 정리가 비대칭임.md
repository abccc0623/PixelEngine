- 날짜: 2026-06-23
- 영역: 엔진
- 상태: 미검토
- 출처: [[2026-06-23]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
Lua 스크립트를 자주 리로드하면서 씬 전환 경로의 정리 코드를 확인하던 중, `Scene::Initialize()`와 `Scene::Release()`의 할당/해제 짝을 비교했다.

## 문제
`Scene::Initialize()`는 `registry`, `system`, `group`, `pool`, `event`를 모두 `new`로 만들지만, `Scene::Release()`에서는 `event->Release()`만 호출하고 `delete event`는 하지 않은 채 `delete this`로 끝난다. 씬을 한 번만 쓸 때는 티가 약하지만, 장시간 리로드와 씬 교체를 반복하는 작업에서는 누적 누수 위치를 찾기 어렵게 만든다.

## 제안
`Scene`이 소유하는 매니저 목록을 한 군데에서 명확히 관리하고, `Initialize()`에서 만든 객체는 `Release()`에서 모두 같은 기준으로 `delete`되도록 정리하면 좋다.
