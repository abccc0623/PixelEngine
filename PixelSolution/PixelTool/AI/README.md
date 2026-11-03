# PixelTool AI Chat

The **AI Chat** document opens beside the Lua editor. The toolbar's **AI Chat (Codex)** button reopens or activates the same view.

## Account connection

Install the official native Codex CLI separately; it is not bundled in the PixelTool installer. The editor searches PATH and the current Windows user's Codex installation. You can select `codex.exe` manually.

Click **계정 연결**. The editor starts `codex app-server`, initializes its stdio connection and checks `account/read`. Existing ChatGPT login can be reused. Otherwise, the official browser login opens. **로그인 확인** retries the account check. API-key accounts are not allowed to start a chat turn, and the editor never reads or copies Codex credential files.

This is a new Codex conversation, not a connection to an existing desktop chat. ChatGPT-plan limits still apply. The editor does not collect an API key or implement inference HTTP requests itself.

When this editor's MCP endpoint is running, the child Codex process is connected to it automatically using a private environment variable. The global Codex configuration is not edited. If another editor owns the fixed MCP port, this editor's chat disables the `pixeltool` MCP entry rather than reading another project's state.

## Chat and file changes

- Enter a message and click **전송**, or press Ctrl+Enter. Replies stream into the transcript.
- Initially **프로젝트 파일 수정 허용** is unchecked: turns are read-only and elevation requests are declined.
- Enable it to allow workspace changes inside the selected project. Turns use the `on-request` approval policy, restricted network access, and the selected project as the writable root. Server-issued file/command approvals open a detail dialog. No session-wide approval or automatic extra permissions are granted. Normal sandbox-authorized operations follow Codex policy; this is not a guarantee of one prompt per changed file.
- Save unsaved editor buffers before asking AI to modify those files. Disk changes are handled by the editor's existing external-file detection.
- **응답 중지** interrupts the current turn. Closing the editor disposes the owned Codex process; closing just the tab retains the same conversation for reopening.
- The model selector is populated with `model/list` after ChatGPT login, including pagination. The saved project selection is restored if still present; otherwise the server's recommended default is selected. Changes apply on the next turn, not mid-response. Each turn supplies the selected model and its catalog default reasoning effort. A catalog entry does not guarantee account access; errors are shown rather than silently switching models. The selection is saved with project chat history.
- Three required documents are embedded in `PixelTool.dll`: `AIRestrictions.md` (operating restrictions), `EditorGuide.md` (editor functions), and `LuaApiGuide.md` (Lua APIs). They are supplied through `developerInstructions` on both thread creation and resume, not prepended to user messages. Missing/empty resources stop connection preparation. Update the relevant document and rebuild the tool; they are not copied into EngineDLL or game Asset folders.
- These documents take priority over conflicting project documents, `AGENTS.md`, skills and user messages within PixelTool AI. Other documents may still be read as supplementary context. Platform/managed policies and actual permissions remain higher; instructions are not a security boundary or a guarantee of perfect model compliance. Project API/version mismatches must still be checked and reported. The documents are not secret, model training, or an MCP tool.

## History

The latest thread ID, selected Codex executable and transcript are stored under `%LOCALAPPDATA%\PixelTool\AIChat\<project-path-hash>.json`. Reconnecting resumes that same thread. Resume failure is shown rather than silently creating a new conversation; use **새 대화** if necessary. New conversations archive the previous local transcript and do not delete the old Codex thread. Store these files as private user data; they can contain source code and chat content.

## Verification

`powershell -NoProfile -ExecutionPolicy Bypass -File PixelSolution/Tests/AIChat.Tests.ps1`

This checks source contracts only. Native/editor builds, real account login, inference, file approvals, streaming and restart recovery require a rebuilt editor and an actual Codex connection. No such live requests are performed by this test.

Protocol reference: https://learn.chatgpt.com/docs/app-server
