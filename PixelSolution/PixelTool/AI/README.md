# PixelTool ChatGPT direct connection

The AI Chat tab opens beside the Lua editor. **Continue with ChatGPT** uses the official local-personal/open-source Sign in with ChatGPT flow. No installed `codex.exe`, API key, client secret, or partner API key is used. The legacy `CodexChatClient.cs` remains but is not instantiated by the editor UI.

## Sign-in

- Starts a loopback-only callback at `http://127.0.0.1:<port>/auth/callback` before opening the system browser. Uses per-attempt state, nonce, PKCE S256 and a stable host ID. Initial dynamic registration receives an issued client ID, not an API key.
- Validates RS256 ID-token signature using OpenAI's discovered JWKS, issuer, audience, nonce and expiry. Other signing algorithms fail closed. Responses access requires the granted `chatgpt.tokens.use.direct` scope.
- Saves each issued client/account separately under `%LOCALAPPDATA%/PixelTool/ChatGPT`, with a current-user ACL and DPAPI encryption. No tokens are put in project files, chat history, command lines or source control. Account selection, adding another account and reauthorization are supported.
- After connection, account details collapse behind **계정 관리**. A saved account is restored once when the chat view first loads; this may refresh tokens and fetch models but never starts inference or opens the login browser automatically. Restore errors reveal the account controls for manual reconnection.
- Refreshes near expiry and saves replacement tokens. A local file lock serializes refresh across editor processes. Failed HTTP requests are not automatically retried. There is no API-key fallback.
- No sign-out/revocation UI yet: disconnect through ChatGPT settings when needed. Closing the editor cancels local requests, not the renewable account grant.

## Inference and tools

- Fetches the current account's visible models from `GET https://api.openai.com/v1/models`. Sends the selected slug to `POST https://api.openai.com/v1/responses` with OAuth, `store:false`, `stream:true`. Only `response.completed` means success. Failed, incomplete and interrupted responses are distinct.
- The three required embedded guides are sent as Responses `instructions` on every inference request. They are not prepended to user text. Project documents may supplement but not override them.
- Local tools list a directory, read/write text, create Asset folders, inspect/copy existing image/audio/font files, import a user-picked external asset, and create simple RGBA PNGs. Text assets include Lua/pxm/scene/JSON and SVG, but SVG engine rendering is not assumed. PNGs are procedural rectangle drawing, not AI image generation: dimensions up to 1024x1024, 4096 rectangles and a bounded pixel-work budget. Copy/import supports files up to 16MB. New binary assets never overwrite existing files; outside files must be explicitly selected by the user in the editor picker.
- Read-only rejects all writes in both modes. **Always ask** shows text changes or an asset operation description for approval; **automatic approval** skips that dialog for requested ordinary creation/replacement when edits are enabled. External import still requires the file picker. Missing preference defaults to ask. The 256KB text cap, project/Asset path boundaries, reparse-point/credential-name checks and stale-text SHA256 checks remain enforced. These are not an OS sandbox or a defense against concurrent malicious filesystem substitution.
- `reimport_project` dispatches through WPF into the render queue, calls existing Reload before UpdateEngine, and completes after that frame processes the main.lua import. It resets runtime, refuses unsaved Lua buffers or paused/shutting-down debugging, uses the selected approval mode and requires edit permission. A 15-second deadline prevents indefinite waiting; a timed-out or canceled operation may already have started and is never automatically retried.
- `read_editor_logs` reads a thread-safe sequence-numbered capture of up to 2000 recent entries, including native logs still pending UI display. Pages contain up to 100 entries with cursor, hasMore, truncated and cleared flags. Existing UI logs are not cleared by AI reimport. Manual log clearing also clears the AI capture without resetting its sequence. Known credential patterns are redacted, and returned message lengths are capped. Partial/cleared/truncated logs are not proof of no errors.
- **No arbitrary shell, deletion, installation, build, new game/editor launch, network tool or MCP execution is exposed to the model.** The narrowly scoped existing-editor reimport is not a build or a new process.
- Save editor buffers first. Tools work with disk files, not unsaved editor content. There is a 16-response tool loop cap and no automatic inference retries.

## History

Display history remains at `AIChat/<project-hash>.json`. Full direct input/output items are saved in separate project/account-bound `chatgpt-<id>` files for replay with `store:false`. New conversations preserve old files. Switching from Codex preserves the transcript but starts new model context, without silently transmitting the old transcript. Account selection starts new context, and resume rejects account mismatches.

The edit-permission checkbox and task approval choice save immediately in project history and restore on restart. New/older histories without the edit-permission field default to read-only. Restoring an explicitly saved permission does not change another project's settings.

Interrupted/failed turns are not saved into replay context; already approved file changes may still exist. The UI warns about that. History may contain source code and is private user data. Authentication tokens are stored separately.

## Verification and distribution

`powershell -NoProfile -ExecutionPolicy Bypass -File PixelSolution/Tests/AIChat.Tests.ps1`

These are static checks, not live authentication/inference tests. Rebuilt-editor browser login, plan entitlement, streaming, file approval, refresh, restart and shutdown need runtime verification. No login, inference, editor launch or build is executed by this test.

The target is currently a personal local tool. Recheck eligibility before sharing the closed-source editor or packaging AI for other users. Game-only distribution must not include editor credentials.

Official references:
- https://developers.openai.com/siwc/token-sharing-open-source/sign-in
- https://developers.openai.com/siwc/token-sharing-open-source/models-and-inference
- https://developers.openai.com/siwc/token-sharing-open-source/profiles-and-sessions
