---
id: TASK-1244
title: 'Boiler info: show Remeha service message A/B/C from ID132 via the profile'
status: To Do
assignee: []
created_date: '2026-10-17 13:12'
labels:
  - boiler-info
  - ui
dependencies: []
priority: low
ordinal: 246000
---

## Description

<!-- SECTION:DESCRIPTION:BEGIN -->
ID132 (Remeha service message) is shown raw (0x0101) with 'mapping not yet confirmed'. Verified 17-10-2026 with a Remeha qSense and PIC response overrides (SR=132, CR=132 after): 0x0101 = Clear A, 0x0102 = Clear B, 0x0103 = Clear C, 0x0104 = Clear without letter, 0x0000/0x0001 = no service code. User decision: only 0x0101-0x0103 are valid values (service message A, B, C); every other value stays raw and uninterpreted. Details in docs/boiler-info/OVERDRACHT.md open point 2.
<!-- SECTION:DESCRIPTION:END -->

## Acceptance Criteria
<!-- AC:BEGIN -->
- [ ] #1 tzerra-m-28c-plus.json holds the ID132 meanings for exactly 0x0101, 0x0102 and 0x0103 (A, B, C) with source note
- [ ] #2 With that profile the OEM row 'Service message' shows 'A', 'B' or 'C' for 0x0101-0x0103 and no longer says 'mapping not yet confirmed'
- [ ] #3 Any other ID132 value (e.g. 0x0104, 0x0001, 0x0000) is shown raw in the chosen Hex/Binary format, without interpretation
- [ ] #4 Without a profile entry the row behaves as before
- [ ] #5 test_otBrandTextUi.js covers 0x0101-0x0103, 0x0104 and 0x0000; python build.py exits 0; python evaluate.py --quick shows no new failures
<!-- AC:END -->
