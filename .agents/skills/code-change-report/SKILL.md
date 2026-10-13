---
name: code-change-report
description: Report changed files, concrete changes, and validation when completing code modifications in the PixelEngine engine, editor, or Survival game. Use for implementation and fixes, not for conceptual answers or read-only investigations.
---

# 코드 변경 결과 보고

## Lua 함수 작성 형식

- PixelEngine 및 Survival의 Lua 함수를 만들거나 수정할 때 함수 선언, 본문, `end`를 반드시 여러 줄로 작성한다. 빈 함수도 한 줄로 축약하지 않는다.
- 빈 함수는 아래 예시처럼 선언과 `end` 사이에 빈 줄 두 개를 둔다. 이름 있는 함수와 익명 함수 모두 여러 줄 형식을 사용한다.
- 이 규칙은 함수 정의에 적용한다. 함수 호출은 가능한 한 한 줄로 유지한다.

```lua
function Graph.Fail()


end
```

## 결과 보고

코드를 수정한 작업의 최종 답변에는 결과만 말하지 말고 다음 내용을 한국어로 짧게 포함한다.

- 수정 파일: 실제 변경한 파일을 클릭 가능한 절대 경로로 안내하고, 각 파일에서 무엇을 바꿨는지 설명한다. 같은 역할의 파일은 묶어서 설명해도 된다.
- 사용 방법: 새 API나 기능이 있다면 필요한 최소 사용 예시를 제공한다.
- 검증 결과: 실제 수행한 확인과 수행하지 않은 빌드·실행 검증을 구분한다. 정적 확인만으로 정상 실행을 보장하지 않는다.
- 남은 조건: 재빌드가 필요하거나 해결되지 않은 문제가 있으면 명시한다.

기존 사용자 변경은 이번 작업의 수정 내역으로 보고하지 않는다. 단순한 변경에는 불필요하게 긴 보고서를 만들지 않는다. 이 스킬은 빌드나 추가 수정에 대한 권한을 부여하지 않는다.
