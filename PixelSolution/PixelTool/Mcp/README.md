# PixelTool MCP

PixelTool hosts the official MCP HTTP server inside the editor process. No extra executable or AI API key is required.

## Connect Codex

1. Rebuild PixelTool and open a project. The endpoint starts at `http://127.0.0.1:41237/mcp`.
2. Open `%LOCALAPPDATA%\PixelTool\Mcp\codex-config.toml`.
3. Copy its `[mcp_servers.pixeltool]` section into Codex `config.toml`, preserving other entries. Do not add the same section twice.
4. Reconnect/restart Codex as needed, then ask: "PixelTool에서 연 프로젝트 정보를 알려줘."

The generated configuration includes a private bearer token. Do not commit or share it. The credentials directory allows only the current Windows user. The token is reused across restarts.

## Tools and boundaries

- `get_project_info`: the currently opened project root and the three folder access rules. Opening a different project starts a new editor session; reconnect MCP to receive its new root.
- `read_project_notes`: reads `Editor/AINavigation.md` as untrusted, potentially stale navigation hints. Verify source before changes. This read never creates the file.
- `search_project_files`: finds file and folder names under the selected project; each result identifies `kind` as `file` or `folder`. An empty query lists one directory (up to `limit` entries). Extension filters apply to nonempty searches. Default scope is Asset.
- `search_file_content`: searches one project-relative text file returned by the path finder for a literal phrase; returns each match's 1-based line and character column with a bounded snippet. A directory path is rejected. It reads the saved file, so an unsaved editor buffer may differ.
- Lua API lookup uses those same search tools: find the declaration file with `search_project_files(query="Entity", path="Engine", extensions=".lua")`, then search that exact result with `search_file_content(path="Engine/Entity.lua", query="Create")`. `read_engine_guide` supplies usage rules; it does not verify installed declarations.
- `create_script_file`: creates one new `Asset/` `.lua` or `.pxm` file with the AI's complete content. Empty `.lua` content stays empty; empty `.pxm` content uses the same bundled `Module.lua` template as the toolbar. It never overwrites a file, checks the editor AI edit/confirmation settings, and refreshes the Asset browser.
- `read_project_file_lines`: reads bounded saved text lines and returns the whole-file SHA256. Follow `nextStartLine`/`nextStartColumn` when truncated; editor unsaved content may differ.
- `patch_project_file`: replaces one unique exact text span using that SHA256. It rejects changed files and unsaved Lua buffers, preserves the rest of the file, and follows editor AI edit/confirmation settings. Only `Asset/` text and `Editor/AINavigation.md` are writable; `Engine/` is read-only.
- `read_editor_logs`: read with `level="errors"` (errors), `"warnings"` (warnings and errors), or `"all"` (everything, default). `limit` bounds scanned entries, so a filtered page may be empty while `hasMore` is true. Continue from `cursor`; check truncation/clear flags before claiming no errors.
- `highlight_lua_lines`: opens/selects an existing project Lua/pxm line range through the WPF dispatcher without changing file content.

Project paths are an immutable startup snapshot. AI file operations resolve against that root. Asset writes require the editor's edit permission; Engine is AI read-only. Editor is read-only except hash-checked edits to `Editor/AINavigation.md` and host-managed append-only records in `Editor/AIWorkLog.md`. Editor internals may still maintain their own state. Editor context/logs are thread-safe captures; highlighting, file creation and patching dispatch to WPF. HTTP handlers never call the native engine. MCP exposes new script creation and hash-checked text patches; it does not expose deletion, arbitrary code execution or scene inspection.

Only one editor can use port 41237. Startup failure shows a warning and lets the editor continue without MCP. Closing the editor stops the endpoint. Unauthenticated and browser-origin requests are rejected.

Release installers include ASP.NET Core through the self-contained publish. Framework-dependent development builds require the ASP.NET Core 8 runtime (included with the .NET SDK).

## Navigation workflow

`read_project_notes` → `search_project_files` under Asset → `search_file_content` on the chosen file → verify the definition before a change. Scripts may be anywhere inside Asset. Example path lookup: `query="Player", path="Asset", extensions=".lua,.pxm", limit=20`; then exact-file lookup: `path="Asset/Script/Player.lua", query="Player.Speed"` (only if that path was actually found).

Path search returns relative paths and `file`/`folder` kinds. File content search returns 1-based line and character column, snippets and incomplete flags. Empty extensions selects supported text files; `*` allows all file names without reading contents. File content search is literal/case-insensitive, not regex. Binary content, secret/hidden paths and symlinks/junctions are excluded. Engine is omitted from broad root searches unless path explicitly starts with Engine. Limits: 50 matches, 240 characters per snippet, 12000 combined path/snippet characters, 5000 entries, 4 MiB read, 3 seconds and 256 KiB per file. Check `truncated`, `stopReason`, `skippedFiles` and `snippetTruncated`; narrow incomplete searches.

PixelTool creates `Editor/AINavigation.md` only when missing on project creation/open, preserving existing notes. Its 8 KiB cap keeps it concise. The embedded CLI AI can maintain verified hints through existing permission/approval/hash-checked file tools; MCP only reads the note. Rebuild/restart the editor, reconnect MCP and start a new embedded AI conversation to register new dynamic tools.

## Verification

- `powershell -NoProfile -ExecutionPolicy Bypass -File PixelSolution/Tests/Mcp.Tests.ps1`: static contracts, not a live MCP handshake.
- `powershell -NoProfile -ExecutionPolicy Bypass -File PixelSolution/Tests/ProjectNavigation.Tests.ps1`: behavioral/path-boundary fixtures plus CLI/MCP wiring. Does not launch the editor, invoke inference or modify game content.
- After rebuilding, verify authentication, `initialize`, `tools/list` and `tools/call` from an MCP client.

Implementation reference: https://github.com/modelcontextprotocol/csharp-sdk/tree/v2.3.0
