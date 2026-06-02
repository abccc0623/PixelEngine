# CODEX.md

This file is for Codex-specific project notes and working rules.

## Project

- Workspace root: `D:\PixelEngine`
- Main solution: `PixelSolution\PixelSolution.slnx`
- Primary projects:
  - `PixelSolution\Client`
  - `PixelSolution\Generate`
  - `PixelSolution\PixelEngine`
  - `PixelSolution\PixelGraphics`
  - `PixelSolution\PixelMeta`
  - `PixelSolution\PixelTool`

## Codex Working Notes

- Prefer reading the existing project structure before making changes.
- Keep edits small and focused on the user's request.
- Do not revert user changes unless the user explicitly asks.
- Use the existing Visual Studio solution and project layout as the source of truth.
- When unsure about build or run steps, inspect the solution/project files first.

## Build Notes

- Open `PixelSolution\PixelSolution.slnx` in Visual Studio.
- Common configurations appear to include `Debug`, `Editor_d`, and `Release`.
- Prefer `x64` unless the user asks for another platform.

## TODO

- Add exact build/run commands after they are confirmed.
- Add project-specific coding conventions as they become clear.

## User Instructions

1. 대답은 최대한 간결하게 말하고 아부하는 말은 금지한다.
2. 파일을 쓰기 전에는 항상 사용자에게 물어보고, 사용자가 허락해야 파일을 쓸 수 있다.
3. 사용자의 명령을 수행하다가 궁금한 점이 있으면 사용자에게 물어보고, 사용자의 답변 내용을 이 md 파일에 계속 추가로 기록한다.

## Project Direction

- The final goal is to build a 3D game engine.
- The current first milestone is to build the engine as a 2D engine and use it to make the first game.
- Game logic will always be written in Lua.
- The project uses Lua 5.1 and LuaJIT for faster execution.
- The current priority is writing Lua APIs needed for games.
- The engine uses an ECS pattern, and Lua-facing components are being created around that design.

## Project Roles

- `Client` is the engine runtime entry point.
- `Generate` extracts Lua-related functions and generates Lua files.
- `PixelTool` is a C# WPF editor project.
- `PixelTool` is intended to make Lua writing and asset creation easier, but it is not urgent right now.
- `Editor_d` is intended as an editor debug configuration, but it is not actively used yet.

## Lua Generation

- Components in `PixelEngine` are organized in a C-style API based on namespaces.
- `Generate` exists to convert those APIs into Lua files automatically.
- Generated Lua files can be found under `D:\PixelEngine\PixelSolution\bin\Debug\Asset\Engine`.

## Coding Preferences

- Do not rename existing functions or variables when modifying code.
- Prefer analyzing engine code instead of changing it.
- Avoid engine code changes unless the user explicitly asks for them.
- When the user asks for Client Lua logic, analyze the engine and generate Lua code based on that analysis.
- Focus mainly on analysis for writing Lua code.

## 토큰 절약 규칙

- 기본 답변은 짧게 한다.
- 많은 파일을 읽기 전에 사용자의 목표를 기준으로 범위를 좁힌다.
- 질문이 너무 넓으면 먼저 대상 기능이나 파일을 물어본다.
- 이미 정리된 프로젝트 정보는 다시 묻지 말고 `CODEX.md`를 참고한다.
- 코드 분석은 관련 있는 파일부터 최소한으로 읽는다.
- 긴 코드나 명령 출력은 그대로 붙이지 말고 요약한다.
- 수정 요청이 아니면 코드를 변경하지 않는다.
- 먼저 결론을 말하고, 필요한 경우에만 이유를 짧게 덧붙인다.
- 같은 설명을 반복하지 않는다.
- 사용자가 요청한 범위 밖의 리팩토링은 제안만 하고 수행하지 않는다.
- 파일 전체를 읽기보다 `rg`, `Select-String` 등으로 필요한 부분을 먼저 찾는다.
- 큰 작업은 바로 전체 구현하지 말고 작은 단계로 나눠 확인한다.
- 코드 예시는 필요한 최소 길이로 작성한다.
- 테스트나 빌드 결과는 핵심 성공/실패만 요약한다.
- 사용자가 명시적으로 요청하지 않으면 빌드하지 않는다.
