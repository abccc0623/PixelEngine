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

## LLM Coding Guidelines

- Think before coding. State assumptions explicitly, ask when the request is unclear, and surface meaningful tradeoffs before choosing an approach.
- Prefer the simplest implementation that solves the requested problem. Do not add speculative features, one-off abstractions, or unnecessary configurability.
- Make surgical changes. Touch only files and lines that directly support the user's request, and avoid unrelated cleanup or refactoring.
- Match the existing project style even when another style seems preferable.
- If unrelated dead code or cleanup opportunities are found, mention them instead of changing them.
- Define success criteria before non-trivial work. For multi-step tasks, use a short plan with a verification step for each item.
- For non-trivial tasks, maintain a checklist and context notes when useful so future sessions can continue without rediscovering decisions.
- When writing Korean output, end Korean sentences with `.`, `?`, or `!` instead of a trailing colon.
- Prefer one meaningful commit per logical change when committing is appropriate. Do not mix unrelated edits into a single commit.
- When something fails, read the actual error message, log output, or stack trace before applying a fix.

## Build Notes

- Open `PixelSolution\PixelSolution.slnx` in Visual Studio.
- Common configurations appear to include `Debug`, `Editor_d`, and `Release`.
- Prefer `x64` unless the user asks for another platform.

## TODO

- Add exact build/run commands after they are confirmed.
- Add project-specific coding conventions as they become clear.

## User Instructions

0. 사용자의 규칙, 선호, 결정처럼 다음 대화에도 유지되어야 하는 내용은 기본적으로 이 md 파일(`CODEX.md`)에 저장한다.
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
- `PixelTool` uses AvalonDock for editor docking windows.
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
- Before changing any engine file, ask the user for permission and wait for approval.
- When the user asks for Client Lua logic, analyze the engine and generate Lua code based on that analysis.
- Focus mainly on analysis for writing Lua code.

## PixelTool Design Reference

- When the user requests a PixelTool UI or tool design change, use the user-provided dark WPF dashboard reference image as the primary visual direction.
- Preserve the editor's docking layout and game-engine workflow instead of copying the dashboard layout literally.
- Use layered charcoal and blue-gray surfaces, with clear visual separation between the application background, panels, and raised controls.
- Use vivid yellow as the primary accent color for active, selected, hovered, and important states.
- Prefer rounded panels and buttons, simple monochrome icons, clean typography, and generous internal spacing.
- Keep normal controls restrained; reserve yellow for emphasis so the interface does not become visually noisy.
- Maintain consistent toolbar heights, margins, border colors, corner radii, and interaction states across Lua Editor, Asset, Log, and future tool windows.

## PixelTool WPF and LSP Type Names

- PixelTool files may use both `System.Windows.Media` and `Microsoft.VisualStudio.LanguageServer.Protocol`.
- In files importing both namespaces, do not use the unqualified `Color` type because both namespaces define it.
- Use `System.Windows.Media.Color` explicitly for WPF colors, including calls such as `System.Windows.Media.Color.FromRgb(...)`.

## PixelTool AvalonDock Styling

- `LayoutDocumentTabItem` and `LayoutAnchorableTabItem` are AvalonDock controls, not standard WPF `TabItem` controls.
- Do not use an `IsSelected` property trigger in their styles because these controls do not expose that property.
- Style their active state with a `DataTrigger` bound to `Model.IsActive`, using `RelativeSource Self`.

## PixelTool Product Decisions

- Persist the AvalonDock editor layout automatically when PixelTool closes and restore it on the next launch.
- Always provide a user-facing action that restores the default editor layout.
- Scene editing will eventually require move, rotate, and scale tools, but implementation depends on engine support.
- The editor play control will live in the main toolbar.
- An Inspector window is planned. PixelTool and PixelEngine should communicate through an explicit request/response boundary rather than directly depending on each other's internal types.
- Keep toolbar menus hover-driven; a click-to-pin mode is not currently needed.
- Apply the established charcoal, blue-gray, and yellow PixelTool theme to future tool and settings windows.
- Apply the same PixelTool theme to every user-facing notification, confirmation, warning, error, and text-input dialog.
- Do not introduce the native WPF `MessageBox` or `Microsoft.VisualBasic.Interaction.InputBox` in new PixelTool code.
- Use `PixelMessageBox` for notifications and confirmations, and `PixelPromptDialog` for text input such as Lua script, asset, and folder names.
- New dialogs should reuse the shared PixelTool brushes and control styles, with charcoal and blue-gray surfaces, yellow accents, rounded corners, balanced spacing, and consistent button interaction states.
- PixelTool must show a blocking project-path window before the main editor opens.
- The startup window offers the last valid project root or creation of a new project under a selected parent directory.
- Closing the startup project-selection window with the title-bar `X` exits PixelTool.
- Asset browsing, Lua file creation, and LSP workspace paths must use `ProjectPathService.AssetPath` instead of a hardcoded `./Asset` path.
- `ProjectPathService.ProjectRootPath` is the named project directory selected or created by the user.
- `AssetPath`, `EnginePath`, and `EditorPath` must always be computed as `ProjectRootPath/Asset`, `ProjectRootPath/Engine`, and `ProjectRootPath/Editor`.
- A newly created PixelTool project must contain three folders directly under its project root: `Asset`, `Engine`, and `Editor`.
- `Asset` stores game-specific asset files and project content.
- `Engine` stores files provided and managed by the engine.
- `Editor` stores editor-only files, including editor state and save data that should not be used by the game runtime.
- When implementing or updating new-project creation, create all three folders together under the selected project directory.
- New-project creation flow: select a parent directory, enter a project name, create the named project directory, then create `Asset`, `Engine`, and `Editor` inside it.

## Engine Feedback Automation

- The engine feedback workflow is now local-first.
- Local feedback files are generated under `D:\PixelEngine\Docs\EngineFeedback`.
- Local feedback generation runs through:
  - `D:\PixelEngine\Tools\FeedbackDashboard\generate_feedback_once.cmd`
  - `D:\PixelEngine\Tools\FeedbackDashboard\generate_startup_feedback.py`
- A Windows Startup launcher was registered at:
  - `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\PixelEngineDailyFeedback.cmd`
- On Windows login, the startup launcher runs the generator once.
- The generator skips generation if `Docs\EngineFeedback\Daily\YYYY-MM-DD.md` already exists for the current day.
- The generator is rule-based and does not use OpenAI API billing.
- Current generator quality expectation:
  - Good for recurring engine usability/code-structure feedback.
  - Less flexible than Codex-generated feedback.
  - Rules should be refined when repeated or low-value feedback appears.
- Local dashboard files are under:
  - `D:\PixelEngine\Tools\FeedbackDashboard`
- Run the local dashboard with:
  - `D:\PixelEngine\Tools\FeedbackDashboard\run_dashboard.cmd`
- The local dashboard is available at:
  - `http://127.0.0.1:8765`
- The local dashboard reads Markdown feedback files and lets the user accept, discard, or reset feedback.
- Local feedback generation and dashboard files are ignored by Git through `.gitignore`:
  - `Tools/FeedbackDashboard/`
- Feedback Markdown itself is public and tracked:
  - `Docs/EngineFeedback/...`
- Public GitHub Pages feedback log files are at the repository root:
  - `index.html`
  - `app.js`
  - `styles.css`
  - `feedback-data.json`
- The public feedback log is updated by exporting Markdown to JSON:
  - `python Tools\ExportFeedbackSite\export_feedback_site.py`
- In this environment, use bundled Python if `python` is not on PATH:
  - `C:\Users\abcc0\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe`
- GitHub Pages should be configured as:
  - Source: `Deploy from a branch`
  - Branch: `main`
  - Folder: `/ (root)`
- Public feedback site URL:
  - `https://abccc0623.github.io/PixelEngine/`
- GitHub Actions/OpenAI API automation was tried and then removed because API quota/billing was required.
- Removed GitHub workflow:
  - `.github/workflows/daily-engine-feedback.yml`
- The local workflow is:
  1. PC login runs startup generator.
  2. Generator creates daily feedback if missing.
  3. User opens local dashboard when needed.
  4. User accepts/discards feedback locally.
  5. To publish public updates, export feedback site JSON and push `Docs`, root site files, and exporter changes.

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

## 컴포넌트 Lua 생성 방향

- `BindJit` 수동 문자열 방식은 장기적으로 제거한다.
- 컴포넌트 LuaJIT FFI 정의는 `PixelMeta`의 Data struct 멤버 정보로 자동 생성한다.
- `BindManager`는 컴포넌트 namespace/static 함수 등록을 유지한다.
  - 예: `AddComponent`, `GetComponent`, `HasComponent`
- 추가로 `TransformData`, `Renderer2DData`, `Rigidbody2DData` 같은 Data struct를 `PixelMeta`에 등록한다.
- Data struct 등록 시 Lua에 노출할 멤버만 `AddMember`로 등록한다.
- 멤버 등록에는 `MetaFlag::LUABIND` 같은 flag를 사용해 Lua 노출 여부를 구분한다.
- `LuaCreate`는 `컴포넌트명`에서 `컴포넌트명Data` 메타 정보를 찾아 `ffi.cdef`를 자동 생성한다.
- `RenderingData`, `bodyID`, `IsCreate`, `velocityCopy` 같은 엔진 내부 필드는 Lua에 노출하지 않는다.
- `TransformData`는 16바이트 정렬과 padding 필드 순서가 중요하므로 자동 FFI 생성 예외로 둔다.
  - 순서: `position`, `bitmask`, `rotation`, `unused1`, `scale`, `unused2`
  - Lua에서는 주로 `position`, `rotation`, `scale`만 사용한다.
- 타입 변환 규칙을 둔다.
  - `float` -> `float`
  - `int` / `int32_t` -> `int`
  - `bool` -> `bool`
  - `unsigned int` -> `uint32_t`
  - `Pixel::Vector2` -> `Vector2`
  - `Pixel::Vector3` -> `Vector3`
  - `Pixel::Matrix4x4` -> 제외하거나 별도 정의 후 사용
- 새 컴포넌트는 `Data + AddComponent/GetComponent/HasComponent + Data 메타 등록` 구조를 따른다.
