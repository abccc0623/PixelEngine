# PixelTool AI connections

The current editor UI uses `CodexEditorClient` → `CodexChatClient` → Codex CLI app-server. See `EditorGuide.md` for current behavior. Navigation uses `Editor/AINavigation.md`, `read_project_notes` and bounded `search_project_files`, shared with the external MCP adapter. Rebuild/restart to activate changes. The UI migrates older tool-schema conversations to a fresh CLI thread while preserving displayed messages and old thread IDs.

## Host-managed workflow

`CodexWorkflowTools` and `CodexWorkSession` enforce three planning depths: Small (`simple`, manager direct, no plan approval), Middle (`middle`, short manager plan and one approval), and Big (`planned`, detailed manager plan and one approval). A separate planner or programmer is optional for a useful independent bounded task; the default is direct manager editing. All edits still use project path scope, permission, SHA256, unsaved-buffer, cancellation, reimport and final-log checks. If selected, `CodexProgrammer` uses one isolated conversation for read-only analyze followed by approved implement. Researcher/reviewer workers remain optional and read-only.

Big work may use `planner_task` when separate detailed design helps. The planner returns numbered feature sections and a host-issued proposal ID; only that stored proposal is shown for approval. Requested revisions go to the same planner and require a new approval. Manager-authored Middle/Big plans use an empty proposal ID and are revised by the manager. Cancel is terminal for the request. A changed tool schema starts a fresh CLI conversation while preserving visible local history.

Host enforcement covers no preapproval writes, planner read-only identity/phase gates, exact proposal identity, requested-section-only replacement, path scope, bounded calls, invalidation and cancellation. Semantic capability accuracy, quality of manager review, cross-feature consistency and behavior within approved files remain model judgments. Planning state is request-local and is not a durable project plan or resumable after restart. No external-reference browser or automated gameplay verification tool was added. CodexPlanning.Tests.ps1 uses a local fake CLI and real built host adapter to verify dispatch/revision/approval/write/cancellation paths without network inference or editor UI; real model quality remains unverified.

`CodexUsageTracker` consumes actual cumulative CLI snapshots and subtracts the full category baseline for each request. The UI separately shows input, cached input (already included in input), output, request total and main-conversation cumulative counts. Independent worker usage is included once. Missing baselines, invalid counters and overflow remain unknown; billing is not inferred. Usage/context display and dispatch instructions add no model calls.

The editor child process sets `features.code_mode.direct_only_tool_namespaces=["pixeltool"]`. PixelTool tools use direct protocol calls instead of code-mode `exec`/`wait`, so a pending choice or approval stays unresolved until its event arrives. Existing edit toggles, plan/file approvals, path checks and cancellation are unchanged. Verified with official CLI 0.167.0 and gpt-6.1-sol: a 40-second held tool reply produced no polling or usage events, then completed after its reply. Other CLI versions still require compatibility verification. This does not disable native security controls or change global CLI configuration.

Programmer model and reasoning effort are independently selected and persisted from the live `model/list` catalog. Unset settings prefer an available 5.x coding model, then an available 5.x model. Missing saved model/effort choices visibly block rather than silently falling back. The manager model is unchanged. Each request uses at most eight serialized programmer phases, each bounded to five minutes/40 tools. Failed/canceled sessions are not replaced automatically; already completed tool changes must be inspected.

## Planning classification and operational scope

The manager classifies whether gameplay/behavior/design choices remain unresolved, not file count, size, system count or implementation effort. Clear multi-file changes and user-supplied or explicitly settled designs use the programmer directly; ambiguous one-file behavior and explicit planning requests use the planner/user-approval flow. This is model judgment, not a deterministic natural-language classifier. The host removes the former two-file limit from simple while keeping a common 20-path resource bound, exact scope, programmer analysis, edit permissions, hashes, cancellation and verification.

The scoped mode preserves existing operational approval for folders, new parent folders, additional read-only roles and path expansion without inventing a need for gameplay planning. It displays the concrete scope and any existing approved design, keeps that design unchanged, requires user approval, and deduplicates identical accepted scope. It cannot replace a forced/user-requested planning gate or pending plan revision. Schema version 9 migrates older CLI sessions to expose this distinction. Existing-value guide shortcuts are retrieval optimizations, not design-routing thresholds.

## Progressive guide retrieval

`EngineGuide.Load` always includes the complete embedded `AIRestrictions.md`. It supplies a section index and mandatory retrieval instructions for EngineCapabilities.md and the complete `EditorGuide.md`, `LuaApiGuide.md` and `Ponytail.md`, rather than sending every reference up front. `read_engine_guide` returns bounded exact text pages; `search_engine_guide` returns short literal matches and offsets. Both are shared with the external MCP adapter and only access the five fixed embedded resources. Relevant complete sections must be read before acting; search snippets alone are insufficient. This ordering is model guidance, not a new host state machine. Existing host permission and completion checks remain enforced independently. Actual project `Engine/*.lua` declarations must still be checked for API/version differences.

The embedded-guide tests report exact full-guide and bootstrap character/byte sizes. These are not token or billing estimates; later required retrieval still consumes context. Tool schema migration starts a fresh CLI thread while retaining displayed messages and prior thread IDs.

`ProjectWorkLog` appends concise what/why/observed paths/verification receipts to each project's `Editor/AIWorkLog.md`. Existing navigation and history are preserved. Search/read only relevant notes when later questions refer to past work. No separate memory agent or automatic full-note injection.

## New asset organization

Before creation, read project navigation hints and `get_asset_layout`, then verify the relevant real folders. Reuse existing casing and custom/feature-based layouts; never move existing files merely to conform. Blank projects use only the needed `Asset/Script`, `Scene`, `Texture`, `Table`, `Audio` and `Font` folders, with `Asset/main.lua` at the root. The project generator still creates only Asset/Engine/Editor; task-approved writes/folder operations create needed subfolders on demand.

The shared creation tools check new categorized files placed directly under Asset. Unless that category already has non-main root files, they reject root placement and return folder guidance. Existing file edits, main.lua and custom subdirectories remain valid; this is a root-clutter guard, not an automatic classifier or renamer. `allowRootPlacement` is normally false. When the user explicitly requests a conflicting root path, true requests a visible placement-exception approval even in automatic mode; it never grants permission or bypasses the edit toggle, cancellation, planned scope or stale-file checks. Existing flat layouts are preserved, including a category deliberately established by an approved root exception.

Layout discovery is bounded and read-only, shared with external MCP. Partial/blocked scans never establish that a project is blank. Project notes remain hints and are updated only with verified folder roles/paths through normal approved note writes; no new policy file or bulk note migration is introduced. The external MCP endpoint still exposes no file-creation tools. Tool schema version 4 uses the existing conversation migration for the new tool/arguments.

## Request scope and capability guidance

Broad/abstract game requests first route to the on-demand `EditorGuide.md` scope section. The manager extracts requested features; the programmer reads EngineCapabilities.md status rules and relevant capability sections, then uses its existing read-only analyze phase to inspect only relevant current Engine Lua declarations, guide semantics, project integration and assets. Findings distinguish supported, partial, unknown and unavailable, cite evidence/limits, separate missing user assets, and lead to a feasible first scope before implementation task breakdown. Existing simple-value shortcuts stay small. Capability assessment itself adds no cache or whole-project scan; the subsequent planned-design stage uses the dedicated planner described above. EngineCapabilities.md is the single engine/editor-owned semantic catalog, embedded in PixelTool rather than copied to games. BindManager and existing generated Engine Lua declarations remain the signature source. Source-backed primitives are distinguished from proposed game Lua logic and actual runtime checks. The existing tool-schema migration advances to 7 so resumed chats can use the added guide enum; this is not engine/DLL version management.

This is model guidance, not a deterministic capability detector. API declarations do not prove runtime behavior; incomplete searches do not prove absence. Static tests verify retrieval/routing, source anchors, known stubs and platformer/ambiguous-request guidance contracts, not classification accuracy or live model compliance. Existing host analysis/implementation isolation, scope/plan approval and file safeguards remain the enforcement boundary. An assessment-only request makes no edits; implementation with unresolved design choices or explicit planning requests still waits for planned approval; clear settled designs do not need redundant planning. Rebuild/restart PixelTool and reconnect to load the updated embedded guidance; no gameplay test was added or implied.

## Refactoring guidance

Refactoring uses the existing workflow and tools, not a new agent or schema. The always-loaded restrictions route to the complete on-demand `EditorGuide.md` refactoring section. It requires focused behavior/caller/indirect-reference inspection, preservation of game values and public contracts, programmer-implemented small changes, and planned approval for unresolved behavior/design choices or explicit user planning requests, including one-file changes; broad behavior-preserving implementation can remain direct. The manager classifies semantic risk; the host cannot infer it from Lua or prove behavior equivalence. Existing scope/approval/worker/log gates remain unchanged.

Before/after comparisons and any feasible authorized tests must be distinguished from reimport and clean logs. No test runner, shell or gameplay-control tool is added. Existing `complete_project_work` fields carry concise what/preserved and why/checks/untested details; these are manager-reported, while changed paths and final log status are host-observed. History is preserved and retrieved only as relevant. Code analysis and implementation now use the selected programmer model; optional read-only roles remain on demand. After rebuilding PixelTool, restart the editor and reconnect to load the updated embedded instructions; the current tool schema migrates old conversations without deleting displayed history or previous thread IDs.

## Legacy direct-connection implementation

The following section describes the retained direct-connection code, not the active editor UI.

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
- The same mandatory bootstrap and progressive guide index are sent as Responses `instructions` on every inference request. They are not prepended to user text. Project documents may supplement but not override them.
- Local tools list a directory, read/write text, create Asset folders, inspect/copy existing image/audio/font files, import a user-picked external asset, and create simple RGBA PNGs. Text assets include Lua/pxm/scene/JSON and SVG, but SVG engine rendering is not assumed. PNGs are procedural rectangle drawing, not AI image generation: dimensions up to 1024x1024, 4096 rectangles and a bounded pixel-work budget. Copy/import supports files up to 16MB. New binary assets never overwrite existing files; outside files must be explicitly selected by the user in the editor picker.
- Read-only rejects all writes in both modes. **Always ask** shows text changes or an asset operation description for approval; **automatic approval** skips that dialog for requested ordinary creation/replacement when edits are enabled. External import still requires the file picker. New histories and histories missing the preference default to automatic approval; explicitly saved choices are preserved. The 256KB text cap, project/Asset path boundaries, reparse-point/credential-name checks and stale-text SHA256 checks remain enforced. These are not an OS sandbox or a defense against concurrent malicious filesystem substitution.
- `reimport_project` dispatches through WPF into the render queue, calls existing Reload before UpdateEngine, and completes after that frame processes the main.lua import. It resets runtime, refuses unsaved Lua buffers or paused/shutting-down debugging, uses the selected approval mode and requires edit permission. A 15-second deadline prevents indefinite waiting; a timed-out or canceled operation may already have started and is never automatically retried.
- `read_editor_logs` reads a thread-safe sequence-numbered capture of up to 2000 recent entries, including native logs still pending UI display. Pages contain up to 100 entries with cursor, hasMore, truncated and cleared flags. Existing UI logs are not cleared by AI reimport. Manual log clearing also clears the AI capture without resetting its sequence. Known credential patterns are redacted, and returned message lengths are capped. Partial/cleared/truncated logs are not proof of no errors.
- **No arbitrary shell, permanent deletion, installation, build, new game/editor launch, network tool or MCP execution is exposed to the model.** Asset file/folder creation, moves/renames and recycle-bin deletion follow current edit and approval settings. The Asset root, outside/reparse/secret paths, stale source trees, unsaved buffers and destination overwrite are rejected. Moving a folder checks its full subtree and updates open editor paths; game references still need explicit updates. The narrowly scoped existing-editor reimport is not a build or a new process.
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
