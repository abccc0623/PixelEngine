# PixelTool editor guide

Feature reference only; AIRestrictions.md governs authority. Report version mismatches rather than assuming unsupported behavior. These guides are embedded in PixelTool, not copied into game Asset or engine DLLs.

## Project layout

Editor-selected root:
- `Asset/main.lua`: fixed execution entry point.
- `Asset/Scene/`: `.scene` files.
- `Asset/Script/`: `.pxm` objects and `.lua` game logic; optional Player/Monster/Inventory/Build/UI subfolders.
- `Asset/Table/`: global designer-editable data such as ItemList/MonsterList. Data first; allow only trivial one-body-line lookup helpers, still written as multiline functions. Object-local state/behavior stays in its script.
- `Asset/Texture/`: images/sheets, optionally Character/UI.
- `Asset/Audio/`: optionally BGM/SFX.
- `Asset/Font/`: fonts.
- `Engine/`: generated engine APIs/settings. Engine Update refreshes missing/stale APIs; hand edits may be overwritten.
- `Editor/`: editor-only project data.

The folders above are examples, not a required layout. For new projects, suggest separating by file kind when helpful. Follow the user's existing structure and casing; avoid needless nesting or placeholder names. Do not reorganize an existing project without a request; ask when layout conflicts require changes. Preserve source sheets and do not delete assets whose use is unknown. Check Asset.Import, Entity.Create, Scene.Change, resource references and name-only loading collisions on moves/renames.

## Game resource organization (optional guidance)

When a user asks how to organize game resources, suggest grouping by asset kind and then by in-game role (for example, images for Player, Monster, Items, effects and UI). Treat this as a starting point, not a required project layout. Keep the project's existing names and folders when they already serve its needs; do not recommend numeric folder prefixes or impose a five-level limit or fixed naming scheme. Add deeper folders only when they make assets easier to find. Name new resources by their role and use when useful, and adapt date or version suffixes to the user's workflow. For unused originals or archived work, suggest a location outside runtime-imported folders only after checking how that project loads assets. Never move, rename or archive existing assets solely to follow this guidance; follow the user's chosen structure and check references before any requested change.

## Chat and settings

- Open the AI chat (AI 자비스) from the toolbar beside the Lua editor. It connects through the installed Codex CLI app-server and its ChatGPT login.
- `Codex CLI 설치` runs the official installer. `Continue with ChatGPT` uses the CLI's system-browser login; no separate API key is required. Account selection/addition/reauthentication uses CLI sessions, shared with Art Studio.
- Settings contains model/approval controls; Account Management contains login controls. Restore saved connections on startup without automatically opening a browser; show account controls if restoration/authentication fails.
- Model, edit permission and approval mode persist per project. New projects default to editing off and Always Confirm. Changes during a response apply to the next message; the current turn keeps its captured settings.
- Editing permission alone is not a change request. Automatic mode removes extra confirmation only for requested, permitted operations.
- `ask_choice` presents one question and 2–5 buttons plus custom text. Continue after the answer; save question/answer in history. Stop/exit cancels waiting. Historical questions are records, not live buttons.
- Results remain visible; actual saved code diffs are initially collapsed. Failures, stops, timeouts, disconnects and refused/failed tools appear as saved `작업 알림` notices; routine internal system/approval records stay hidden. Failure does not roll back applied changes.
- Live elapsed time and final total duration include user-answer/approval waits.
- New Conversation archives local history. Closing the tab preserves it. Editor exit cancels pending requests and stops only its own Codex app-server child process. Legacy direct-connection transcripts remain visible but start a new CLI conversation.
- No fixed tool-round cap. The same tool/target/error repeated 3 times without actual file changes stops stalled work. Normal reads do not increase failure counts; actual mutations reset them. Time limits and user cancellation remain.

## Available tools

- Prefer `read_project_file_lines(path,startLine,lineCount)` for inspection: 1-based lines, 1–200 lines, at most 12000 output characters, whole-file hash, partial/truncated/hasMore flags. Line numbers are not source code. Existing whole-file writes require a current full `read_project_file` in this request; partial reads cannot authorize replacing unseen content.
- Large transmission context (96 Ki serialized JSON characters, not a token count) is compacted using the selected model and `responses/compact` at closed tool-call boundaries. Canonical compact output replaces only the old prefix without pruning individual items; the latest complete response/tool batch stays verbatim. Full-file edit authorization is retained only for that preserved batch; read older files again before replacement. Application policies are sent separately every time. Display/history records are not deleted. Compaction itself uses a request; unavailable/failed compaction preserves original context, warns once and is not retried in that turn.
- Project text listing/reading/writing; writes check expected file hashes. Always Confirm previews before/after; Automatic + editing enabled performs in-scope writes without another dialog.
- Asset folder creation; file/folder moves/renames and Recycle Bin deletion under current approval settings:
  - `create_asset_folder(path)`
  - `move_asset(path, destination, reason)`: exact new path; parent must exist; no overwrite or moving a folder into itself.
  - `delete_asset_file(path, reason)`
  - `delete_asset_folder(path, reason)`: includes descendants.
- Asset root/Engine/Editor/outside moves/deletion, permanent deletion, reparse/secret paths, stale contents and unsaved buffers are blocked. Directory operations validate descendants. Moves update open editor paths and Asset browser, but the AI must update game references separately. Report affected paths and recovery.
- Inspect/copy project image/audio/font assets; import external assets only via the user's file-picker selection. No automatic outside-directory scan or model-supplied external paths. Copy/import limit 16 MB; never overwrite.
- Procedural PNG creation: RGBA background/rectangles, transparency supported; 1–1024 pixels per dimension, ≤4096 rectangles. For simple placeholders/icons, not generative image AI. Follow the image-choice rule before using it.
- Create `.pxm`, `.scene`, Lua, JSON and other supported text via file-writing tools. SVG text may be saved, but do not assume SVG rendering; prefer PNG/imported textures.
- No arbitrary shell, installation, build or new game/editor launch tool. Never claim unsupported actions. Saving an asset does not register it with the engine or prove it renders.

## Reimport and logs

Follow AIRestrictions.md's save/reimport/fix loop; no additional gameplay testing unless requested.
- Save accessible editor changes before reimport; report inability to preserve/inspect unsaved changes. Disk changes are detected by the editor.
- `reimport_project` uses existing Reload on the render thread; the engine rereads Asset/main.lua, resets runtime, and returns captured logs/cursor after the reload frame.
- Refuses unsaved buffers, debugger break/shutdown or non-running engine. Always Confirm uses a dialog; Automatic follows edit permission.
- `read_editor_logs(afterCursor, limit)`: continue through cursor/hasMore; account for truncated/cleared history. Reimport does not clear logs.
- Cleared/partial logs or a completed reload do not prove error-free gameplay. On reload timeout/cancellation, initialization may already have run: do not assume rollback or blindly retry; inspect the cause.

## Distribution

Current ChatGPT integration is for local personal development. Before distributing the editor/installer, recheck integration distribution eligibility/terms and notify the user. A game-only distribution must exclude editor AI and authentication data.
