# EditorTools guide

## Available tools

- `get_asset_permissions()` reports whether this turn can edit through PixelTool tools and which project paths are permitted. Asset reads and permitted changes use those tools even while native Codex file access is read-only.
- `get_asset_layout()` is read-only and reports actual observed folders/casing by asset kind and same-kind root placement, excluding entry `main.lua` as flat-layout evidence. Results are bounded: check incompleteness/skipped entries and inspect a relevant narrower path when needed. Empty or partial observations do not authorize edits, force default folder names or prove a layout absent.
- Prefer `read_project_file_lines({requests:[{path,startLine,lineCount,startColumn:0},...]})` in chat: 1-4 ranges, automatically capped at 200 returned lines and 12000 shared content characters, whole-file hashes, per-range continuation, partial/truncated/hasMore flags and current permissions. Follow continuation until null for the requested range, with startColumn=0 initially; compare hashes between pages. Guide requests similarly return at most 6000 characters per page and 16000 per batch, with per-page continuation. Line numbers are not source code. Existing whole-file writes require a current full `read_project_file` in this request; partial reads cannot authorize replacing unseen content.
- Large transmission context (96 Ki serialized JSON characters, not a token count) is compacted using the selected model and `responses/compact` at closed tool-call boundaries. Canonical compact output replaces only the old prefix without pruning individual items; the latest complete response/tool batch stays verbatim. Full-file edit authorization is retained only for that preserved batch; read older files again before replacement. Application policies are sent separately every time. Display/history records are not deleted. Compaction itself uses a request; unavailable/failed compaction preserves original context, warns once and is not retried in that turn.
- Project text listing/reading/writing; writes check expected file hashes. In-scope writes use path, hash and unsaved-buffer checks without a separate file dialog.
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

When the user requests validation after Asset changes, save, reimport, inspect new errors, and check the requested work items against their checklist methods. Report checks that cannot be run; never count them as passed.
- Save accessible editor changes before reimport; report inability to preserve/inspect unsaved changes. Disk changes are detected by the editor.
- `reimport_project` clears the current log window/history, then uses existing Reload on the render thread; the engine rereads Asset/main.lua, resets runtime, and returns new logs/cursor after the reload frame. Manual `Ctrl+R` does not automatically clear logs.
- Refuses unsaved buffers, debugger break/shutdown or non-running engine. Reimport needs no separate action dialog.
- `read_editor_logs(afterCursor, limit)`: continue through cursor/hasMore; account for truncated/cleared history. Reimport clears previous logs; use its `afterCursor` for the new log window.
- Treat `level >= 2` as an error. A save or reload alone is not success. Inspect all new pages; reimport again only after a correction. Do not clear logs separately or repeat an unchanged import for a clean-looking result. Stop after three identical failures without progress, and report incomplete verification.
- Cleared/partial logs or a completed reload do not prove error-free gameplay. On reload timeout/cancellation, initialization may already have run: do not assume rollback or blindly retry; inspect the cause.
