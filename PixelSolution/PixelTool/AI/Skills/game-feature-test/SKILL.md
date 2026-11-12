---
name: game-feature-test
description: Check implemented game behavior against work-item checklists in the pipeline test stage using PixelTool reimport, bounded play input and logs.
---

# Game feature test

1. Gather the relevant work-item checklists and prerequisites. Choose a small set of observable normal and boundary events; keep expected results separate from observed results.
2. Reimport the changed Asset files. Inspect the new logs and any remaining pages. When input testing helps, enter play mode, send bounded key inputs, and check event order, entity IDs and errors. Use temporary `[AI_TEST]` logs only when existing output cannot show the event path.
3. Return to editor mode if this test changed the mode. Remove temporary logs, reimport and check new errors again.
4. Mark each check passed, failed or not run with evidence. Logs show that a code path ran; use a visual check for appearance and do not claim it from logs. Report unavailable controls or repeated failures without inventing a pass.

Follow the approved scope and existing editor tool limits; this skill does not supply a general test runner.
