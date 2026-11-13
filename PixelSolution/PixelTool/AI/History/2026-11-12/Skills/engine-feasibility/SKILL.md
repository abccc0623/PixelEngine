---
name: engine-feasibility
description: Decide whether requested game behavior has a current PixelEngine implementation route. Use for broad game requests, unfamiliar APIs or uncertain engine support before implementation task breakdown.
---

# Engine feasibility

1. Extract only the observable behaviors relevant to the request from user-provided or inspected references. Separate explicit requirements from a proposed interpretation.
2. Read `EngineCapabilities.md` status rules and the relevant sections, then check the selected project's matching `Engine/*.lua` declarations and existing Asset logic. Inspect a focused source path only when the evidence conflicts or the route remains unclear.
3. For each behavior, state the current primitive, Lua or asset work needed, limit, and one of supported, partial, unknown or unavailable. A declared API is not a complete game feature or runtime proof.
4. Turn supported or partial routes into bounded implementation items with checks. Give unknown routes a specific investigation question and exit condition. Keep unavailable routes out of implementation and explain a smaller alternative when one is evidenced.

Do not copy an entire reference game into scope or invent APIs, art, audio, networking or observed test results.
