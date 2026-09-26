# 프로젝트 기술 기록

공통 작업 규칙은 `AGENTS.md`에 있다. 이 문서는 관련 작업을 할 때 참고하는 프로젝트 기록이다.

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

## Build Notes

- Open `PixelSolution\PixelSolution.slnx` in Visual Studio.
- Common configurations appear to include `Debug`, `Editor_d`, and `Release`.
- Prefer `x64` unless the user asks for another platform.

## Project Direction

- The final goal is to build a 3D game engine.
- The current first milestone is to build the engine as a 2D engine and use it to make the first game.
- Game logic will always be written in Lua.
- The project uses Lua 5.1 and LuaJIT for faster execution.
- The current priority is writing Lua APIs needed for games.
- The engine uses an ECS pattern, and Lua-facing components are being created around that design.

## Current Game Concept

- Tile artwork source images must be created at `1254 x 1254`. The in-game cell size may still be `64 x 64`; downscaling/export is a separate step.

- 현재 제작 중인 게임은 뱀파이어 서바이버류 전투와 공장 자동화 퍼즐을 결합한 다크 판타지 도트 게임이다.
- 플레이어가 레벨업하면 물질을 가공하는 장치 카드 3개 중 하나를 선택한다.
- 선택 카드 UI는 앞서 제작한 어두운 중세 판타지 픽셀 카드 스타일을 기준으로 한다.
- 선택한 장치는 `N x N` 보드의 64 x 64 칸에 설치한다.
- 보드에는 항상 시작 지점과 끝 지점이 존재한다.
- 시작 지점에서 물질이 생성되고, 컨베이어 벨트 또는 전자 회로와 비슷한 운반 칸을 통해 끝 지점까지 이동한다.
- 운반 과정에서 설치 장치가 물질을 분할해 수량을 늘리거나 파워를 강화하는 등 물질의 속성을 변경한다.
- 칸 하나에는 장치 또는 운반 요소 하나만 설치할 수 있으며 서로 겹치지 않는다.
- 보드와 설치 칸의 시각 방향은 마법과 기계가 결합된 룬 회로 스타일이다.
- 이후 제작하는 64 x 64 보드 타일, 시작·끝 지점, 운반 회로, 가공 장치는 같은 룬 회로 아트 방향을 유지한다.

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
