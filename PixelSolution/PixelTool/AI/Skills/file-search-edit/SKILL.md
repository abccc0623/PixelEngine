---
name: file-search-edit
description: Find and edit existing project text files for a concrete code or data change. Use after a target work item is assigned; skip for read-only questions and image-only work.
---

# File search and edit

1. Use the selected project path and the work item's explicit target terms. Prefer an exact path, then a narrow filename search under Asset; use bounded content search only when needed. Inspect the relevant definition and callers, and note incomplete search results.
2. Read only the required text and current hash. Before writing, verify the target path, registered work scope and unsaved-buffer state through the project tools. Preserve unrelated user edits.
3. Apply the smallest hash-checked patch or supported file operation. If the hash changed, reread and rebase instead of forcing the old patch. Do not write to Engine or unrelated Editor files.
4. Reimport after an Asset change and inspect the new error logs. Hand the work item's feature-test checklist to the functional-testing stage; report checks actually run separately from planned checks.

This skill does not bypass required plan/scope approval, path or hash checks.
