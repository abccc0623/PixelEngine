---
name: pixeltool-mcp-create
description: Create a tool in PixelTool's deployed MCP server when the user asks to "MCP 만들어줘" or to add a PixelTool MCP tool. Require a discovery keyword before implementation. Not for ordinary AI chat tools or Lua API documentation.
---

# PixelTool 배포용 MCP 도구 만들기

1. 새 도구의 **키워드**를 반드시 정한다. Asset 창에서 다루는 파일·폴더·Lua/PXM·이미지 등의 기능은 `Asset`, 에디터 기능(게임 모드 전환, 로그 등)은 `Editor`를 쓴다. 세부 동작을 키워드로 삼지 않는다. 이 두 범주에 속하지 않는 기능은 키워드 후보를 제안해 사용자에게 확인한 뒤 구현한다.
2. 기존 `PixelSolution/PixelTool/Mcp/PixelToolMcpServer.cs`의 등록·호출 방식을 확인한다. 같은 기능의 도구가 있으면 중복 도구 대신 기존 도구를 확장할지 판단한다.
3. MCP에 노출하는 함수 이름(`McpServerTool`의 `Name`)은 반드시 **`키워드_대상_행동`** 순서로 짓는다. 예: `Asset_File_Delete`, `Asset_Folder_Delete`, `Editor_Game_PlayMode`, `Editor_Game_EditorMode`, `Editor_Log_Read`. 첫 부분은 1번에서 확정한 키워드와 대소문자까지 일치시킨다. C# 메서드 이름과 혼동하지 않는다.
4. 구현 전에 도구 이름, 확정된 키워드, 입력값(이름·형식·필수 여부), 허용 경로와 부작용을 짧게 정한다. **성공 조건과 실패 조건**, 각 경우의 반환 형식·필드·예시를 필수로 지정한다. 예를 들어 파일 생성은 `Asset/` 하위이고 같은 **확장자 포함 전체 파일명**이 없으면 성공하며, `Player.lua`가 있어도 `Player.pxm`은 생성할 수 있다. Asset 밖 경로나 동일한 전체 파일명이 있으면 실패한다. 실패를 예외로 전달한다면 MCP 오류 응답과 오류 문구를 명시한다. 반환 계약이 정해지지 않았다면 구현하지 않는다. 사용자 요청에서 확정할 수 없는 필수 입력·동작만 질문한다.
5. MCP 도구의 설명에 `Keywords: <확정된 키워드>`와 성공·실패 반환 형식을 명시한다. 키워드 표기는 검색용 메타데이터일 뿐이며, 별도의 키워드 검색 구현이 없으면 자동 검색 기능이 생긴다고 주장하지 않는다. 함수 매개변수와 실제 반환값은 설명한 계약과 일치시킨다.
6. 기존 프로젝트 경로 검사, 권한, 해시, 미저장 편집, 에디터 스레드 제약을 재사용한다. 필요한 코드와 영향 범위만 수정하고, 빌드로 컴파일을 확인한다. 실제 MCP 호출을 실행하지 않았다면 실행 검증으로 보고하지 않는다.
7. 오류를 나중에 재현할 수 있도록 MCP 호출 로그를 남긴다. 도구명, 요청 ID, 안전한 입력값 요약, 호출 대상, 성공·실패, 실패 단계, 오류 코드·원인, 변경 여부를 기록한다. 파일 내용 전문, 인증 정보, 비밀 값은 로그에 남기지 않는다. 로그는 PixelTool 내부에서 기록하며 별도 LLM 호출을 만들지 않는다.

결과에는 키워드, 도구 이름, 입력값, 성공·실패 반환 예시, 검증 결과를 짧게 적는다.
