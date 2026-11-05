# PixelTool MCP (minimal)

PixelTool hosts the official MCP HTTP server inside the editor process. No extra executable or AI API key is required.

## Connect Codex

1. Rebuild PixelTool and open a project. The MCP endpoint starts at `http://127.0.0.1:41237/mcp`.
2. Open `%LOCALAPPDATA%\PixelTool\Mcp\codex-config.toml`.
3. Copy its `[mcp_servers.pixeltool]` section into your Codex `config.toml`, preserving other entries. Do not add the same section twice.
4. Reconnect/restart Codex as needed, then ask: "PixelTool에서 연 프로젝트 정보를 알려줘."

The generated configuration includes a private bearer token. Do not commit or share it. The credentials directory allows only the current Windows user. The token is reused across restarts.

## Scope and limits

- `get_project_info` returns project/Asset/Engine/Editor paths and the editor process ID.
- `get_editor_context` returns the current Lua file, caret, selection lines/text and modified state so an AI can inspect the focused code first.
- No engine commands, file edits, code execution, scene inspection or chat UI.
- Project data is an immutable startup snapshot. No WPF or engine calls run on HTTP threads.
- Only one editor can use port 41237 at a time. If startup fails, the editor shows a warning and continues without MCP.
- Closing the editor stops the endpoint. Unauthenticated and browser-origin requests are rejected.
- Release installers include ASP.NET Core through the existing self-contained publish. Ordinary framework-dependent development builds also require the ASP.NET Core 8 runtime (included with the .NET SDK).

Implementation reference: https://github.com/modelcontextprotocol/csharp-sdk/tree/v2.3.0

Static checks: `powershell -NoProfile -ExecutionPolicy Bypass -File PixelSolution/Tests/Mcp.Tests.ps1`
These checks are not an HTTP handshake or runtime test. After rebuilding, verify authentication, `initialize`, `tools/list`, and `tools/call` from an MCP client.
