# PixelTool MCP

PixelTool hosts the official MCP HTTP server inside the editor process. No extra executable or AI API key is required.

## Connect Codex

1. Rebuild PixelTool and open a project. The endpoint starts at `http://127.0.0.1:41237/mcp`.
2. Open `%LOCALAPPDATA%\PixelTool\Mcp\codex-config.toml`.
3. Copy its `[mcp_servers.pixeltool]` section into Codex `config.toml`, preserving other entries. Do not add the same section twice.
4. Reconnect/restart Codex as needed, then ask: "PixelTool에서 연 프로젝트 정보를 알려줘."

The generated configuration includes a private bearer token. Do not commit or share it. The credentials directory allows only the current Windows user. The token is reused across restarts.

## Tools and boundaries

- `get_project_info`: the open project's Asset/Engine/Editor paths and editor process ID.
- `get_editor_context`: current Lua file, caret, selection lines/text and modified state.
- `read_project_notes`: reads `Editor/AINavigation.md` as untrusted, potentially stale navigation hints. Verify source before changes. This read never creates the file.
- `search_project_files`: literal filename or UTF-8 content search using the same project path validation as editor AI tools. Default scope is Asset; never assume a Script layout.
- `read_editor_logs`: the existing thread-safe captured log stream. Follow cursor/hasMore, distinguish old/new errors and check truncation/clear flags. Partial/unavailable logs cannot prove error-free completion.
- `highlight_lua_lines`: opens/selects an existing project Lua/pxm line range through the WPF dispatcher without changing file content.

Project paths are an immutable startup snapshot. Editor context/logs are thread-safe captures; highlighting dispatches to WPF. HTTP handlers never call the native engine. No file writes, arbitrary code execution, scene inspection or chat UI are exposed through MCP.

Only one editor can use port 41237. Startup failure shows a warning and lets the editor continue without MCP. Closing the editor stops the endpoint. Unauthenticated and browser-origin requests are rejected.

Release installers include ASP.NET Core through the self-contained publish. Framework-dependent development builds require the ASP.NET Core 8 runtime (included with the .NET SDK).

## Navigation workflow

`read_project_notes` → filename search under Asset → focused source read → exact-symbol content search → verify the definition before a change. Scripts may be anywhere inside Asset. Example arguments: `query="Player", path="Asset", mode="filename", extensions=".lua,.pxm", limit=20`.

Search returns relative paths, 1-based content line numbers, snippets and incomplete flags. Empty extensions selects supported text types; `*` is allowed only for filename search without reading contents. Content search is literal/case-insensitive, not regex. Binary content, secret/hidden paths and symlinks/junctions are excluded. Engine is omitted from broad root searches unless path explicitly starts with Engine. Limits: 50 matches, 240 characters per snippet, 12000 combined path/snippet characters, 5000 entries, 4 MiB read, 3 seconds and 256 KiB per file. Check `truncated`, `stopReason`, `skippedFiles` and `snippetTruncated`; narrow incomplete searches.

PixelTool creates `Editor/AINavigation.md` only when missing on project creation/open, preserving existing notes. Its 8 KiB cap keeps it concise. The embedded CLI AI can maintain verified hints through existing permission/approval/hash-checked file tools; MCP only reads the note. Rebuild/restart the editor, reconnect MCP and start a new embedded AI conversation to register new dynamic tools.

## Verification

- `powershell -NoProfile -ExecutionPolicy Bypass -File PixelSolution/Tests/Mcp.Tests.ps1`: static contracts, not a live MCP handshake.
- `powershell -NoProfile -ExecutionPolicy Bypass -File PixelSolution/Tests/ProjectNavigation.Tests.ps1`: behavioral/path-boundary fixtures plus CLI/MCP wiring. Does not launch the editor, invoke inference or modify game content.
- After rebuilding, verify authentication, `initialize`, `tools/list` and `tools/call` from an MCP client.

Implementation reference: https://github.com/modelcontextprotocol/csharp-sdk/tree/v2.3.0
