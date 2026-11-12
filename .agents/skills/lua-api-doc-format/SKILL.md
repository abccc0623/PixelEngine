---
name: lua-api-doc-format
description: Update PixelEngine Lua API documentation in a consistent four-part format. Use when the user asks to add, revise, or standardize Lua API documentation; not for ordinary Lua implementation or API explanations.
---

# Lua API 문서 양식

사용자가 Lua API 문서 업데이트를 요청하면, 요청한 API 항목만 아래 형식으로 작성하거나 고친다. 기존 문서의 다른 항목은 유지한다.

```md
## 기능명

### 언제 사용하는지
사용 상황을 한두 문장으로 설명합니다.

### 최소 예제
(검증한 실제 Lua 호출을 담은 짧은 코드 블록)

### 주의사항
- 반환값, 호출 순서, 지원 범위 등 실수하기 쉬운 점만 적습니다.
```

- 기능명은 실제 Lua API 이름을 쓴다. 각 기능에 네 항목을 같은 순서로 둔다.
- 현재 엔진의 바인딩 등록·구현과 생성된 `Engine/*.lua` 선언을 대조한다. 이름, 매개변수, 반환값을 추측하지 않는다.
- 예제는 해당 기능만 보여주는 최소 코드로 작성한다. 문서만으로 확인되지 않는 동작은 검증 사실처럼 쓰지 않는다.
- 주의할 점이 없으면 `- 확인된 추가 주의사항 없음.`처럼 짧게 적고, 가상의 제한을 만들지 않는다.
- 문서만 수정했다면 빌드는 생략하고 양식·링크·변경 범위를 확인한다.
