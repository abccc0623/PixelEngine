- 날짜: 2026-06-20
- 영역: 엔진
- 상태: 미검토
- 출처: [[2026-06-20]]

## 결정
- [ ] 채택
- [ ] 폐기
폐기 사유:

## 상황
Lua 쪽에서 총알이나 이펙트처럼 반복 생성되는 오브젝트를 `Pool.CreateEntity`로 만들고, 이후 `PoolManager`를 통해 재사용되는 흐름을 테스트하려고 함.

## 문제
현재 `Scene::CreatePoolEntity`가 `0`을 반환하고 있어, Pool API가 실제 Entity 생성과 Pool 등록까지 이어지는지 확인할 수 없다. 이 상태에서는 Lua 사용자가 Pool을 써도 일반 생성과 어떤 차이가 있는지 알기 어렵다.

## 제안
`Scene::CreatePoolEntity`에서 `CreateEntity`로 id를 만들고 `PoolManager`에 등록하는 최소 흐름을 먼저 구현하면 좋음. 이후 활성/비활성 정책, Group 연동 여부, Release 시점 처리는 별도 단계로 확장하면 된다.
