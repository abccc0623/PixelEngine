# PixelTool MCP

PixelTool hosts the official MCP HTTP server inside the editor process. No extra executable or AI API key is required.

## Connect Codex

1. Rebuild PixelTool and open a project. The endpoint starts at `http://127.0.0.1:41237/mcp`.
2. Open `%LOCALAPPDATA%\PixelTool\Mcp\codex-config.toml`.
3. Copy its `[mcp_servers.pixeltool]` section into Codex `config.toml`, preserving other entries. Do not add the same section twice.
4. Reconnect/restart Codex as needed, then ask: "PixelTool에서 연 프로젝트 정보를 알려줘."

The generated configuration includes a private bearer token. Do not commit or share it. The credentials directory allows only the current Windows user. The token is reused across restarts.

## Tools and boundaries

- `Editor_Project_Info`: the currently opened project root and the three folder access rules. Reconnect after opening a different project.
- `Editor_ProjectNotes_Read`: reads `Editor/AINavigation.md` as untrusted navigation hints; never creates it.
- `Asset_File_Search`: finds project file and folder names; an empty query lists one directory. Default path is Asset.
- `Asset_FileContent_Search`: searches one saved project text file for a literal phrase and returns 1-based line and column positions.
- Lua API lookup: `Asset_File_Search(query="Entity", path="Engine", extensions=".lua")`, then `Asset_FileContent_Search(path="Engine/Entity.lua", query="Create")`. `Editor_LuaGuide_Read` and `Editor_LuaGuide_Search` provide usage reference, not installed declaration proof.
- `Asset_Layout_Read`: inspects bounded Asset folders and file kinds without modifying files.
- `Asset_File_Create`: creates one new `Asset/` `.lua` or `.pxm` file. Empty `.pxm` uses the bundled `Module.lua` template; a different extension is a different filename.
- `Asset_File_Delete`: moves one `Asset/` file to the Windows Recycle Bin. It rejects unsaved Lua buffers and checks for changes during the call; optional `expectedHash` guards a previously inspected file.
- `Asset_File_ReadLines`: reads bounded saved text lines and the whole-file SHA256. Follow `nextStartLine`/`nextStartColumn` when truncated.
- `Asset_File_Patch`: replaces one unique exact text span using that SHA256. It retains the existing `Editor/AINavigation.md` exception.
- `Editor_Log_Read` and `Editor_Log_Clear`: inspect filtered log pages or explicitly clear the log window and captured history.
- `Editor_Game_PlayMode`, `Editor_Game_EditorMode`, `Editor_Game_KeyInput`: change toolbar mode or send bounded game key input.
- `Editor_Lua_Highlight`: opens a Lua/PXM file and highlights a line range without changing content.

Every tool returns `success:true` with a `data` object on success, or `success:false` with `error` and `message` on expected failure. File mutation failures also include `path`. The returned `data` keeps each tool's original fields. Unexpected transport or process failures may be reported by MCP itself.

Project paths are an immutable startup snapshot. AI file operations resolve against that root. Engine is AI read-only. Editor is read-only except hash-checked edits to `Editor/AINavigation.md` and host-managed append-only records in `Editor/AIWorkLog.md`. Editor internals may still maintain their own state. Editor context/logs are thread-safe captures; highlighting, file creation, patching and Recycle Bin deletion dispatch to WPF. HTTP handlers never call the native engine. MCP does not expose arbitrary code execution or scene inspection.

Only one editor can use port 41237. Startup failure shows a warning and lets the editor continue without MCP. Closing the editor stops the endpoint. Unauthenticated and browser-origin requests are rejected.

Release installers include ASP.NET Core through the self-contained publish. Framework-dependent development builds require the ASP.NET Core 8 runtime (included with the .NET SDK).

## Navigation workflow

`Editor_ProjectNotes_Read` → `Asset_File_Search` under Asset → `Asset_FileContent_Search` on the chosen file → verify the definition before a change. Scripts may be anywhere inside Asset. Example path lookup: `query="Player", path="Asset", extensions=".lua,.pxm", limit=20`; then exact-file lookup: `path="Asset/Script/Player.lua", query="Player.Speed"` (only if that path was actually found).

Path search returns relative paths and `file`/`folder` kinds. File content search returns 1-based line and character column, snippets and incomplete flags. Empty extensions selects supported text files; `*` allows all file names without reading contents. File content search is literal/case-insensitive, not regex. Binary content, secret/hidden paths and symlinks/junctions are excluded. Engine is omitted from broad root searches unless path explicitly starts with Engine. Limits: 50 matches, 240 characters per snippet, 12000 combined path/snippet characters, 5000 entries, 4 MiB read, 3 seconds and 256 KiB per file. Check `truncated`, `stopReason`, `skippedFiles` and `snippetTruncated`; narrow incomplete searches.

PixelTool creates `Editor/AINavigation.md` only when missing on project creation/open, preserving existing notes. Its 8 KiB cap keeps it concise. The embedded CLI AI can maintain verified hints through existing permission/approval/hash-checked file tools; MCP only reads the note. Rebuild/restart the editor, reconnect MCP and start a new embedded AI conversation to register new dynamic tools.

## Verification

- `powershell -NoProfile -ExecutionPolicy Bypass -File PixelSolution/Tests/Mcp.Tests.ps1`: static contracts, not a live MCP handshake.
- `powershell -NoProfile -ExecutionPolicy Bypass -File PixelSolution/Tests/ProjectNavigation.Tests.ps1`: behavioral/path-boundary fixtures plus CLI/MCP wiring. Does not launch the editor, invoke inference or modify game content.
- After rebuilding, verify authentication, `initialize`, `tools/list` and `tools/call` from an MCP client.

Implementation reference: https://github.com/modelcontextprotocol/csharp-sdk/tree/v2.3.0
