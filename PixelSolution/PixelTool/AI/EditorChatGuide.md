# EditorChat guide

## Chat and settings

- Open the AI chat (AI 자비스) from the toolbar beside the Lua editor. It connects through the installed Codex CLI app-server and its ChatGPT login.
- `Codex CLI 설치` runs the official installer. `Continue with ChatGPT` uses the CLI's system-browser login; no separate API key is required. Account selection/addition/reauthentication uses CLI sessions, shared with Art Studio.
- Settings contains model and optional plan-approval controls; Account Management contains login controls. Restore saved connections on startup without automatically opening a browser; show account controls if restoration/authentication fails.
- The selected project allows in-scope AI file changes without a file-by-file dialog. The optional RequirePlan setting applies to the next request; required Middle/Big plan and scoped approvals still apply.
- A user change request and registered work scope are still required. File access alone does not authorize unrelated changes.
- `ask_choice` presents one question and 2–5 buttons plus custom text. Continue after the answer; save question/answer in history. Stop/exit cancels waiting. Historical questions are records, not live buttons.
- Results remain visible; actual saved code diffs are initially collapsed. Failures, stops, timeouts, disconnects and refused/failed tools appear as saved `작업 알림` notices; routine internal system/approval records stay hidden. Failure does not roll back applied changes.
- Live elapsed time and final total duration include user-answer/approval waits.
- New Conversation archives local history. Closing the tab preserves it. Editor exit cancels pending requests and stops only its own Codex app-server child process. Legacy direct-connection transcripts remain visible but start a new CLI conversation.
- No fixed tool-round cap. The same tool/target/error repeated 3 times without actual file changes stops stalled work. Normal reads do not increase failure counts; actual mutations reset them. Time limits and user cancellation remain.

## Distribution

Current ChatGPT integration is for local personal development. Before distributing the editor/installer, recheck integration distribution eligibility/terms and notify the user. A game-only distribution must exclude editor AI and authentication data.
