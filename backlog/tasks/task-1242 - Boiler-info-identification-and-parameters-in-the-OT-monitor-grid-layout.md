---
id: TASK-1242
title: 'Boiler info: identification and parameters in the OT monitor grid layout'
status: In Progress
assignee:
  - '@claude'
created_date: '2026-10-17 11:46'
updated_date: '2026-10-17 11:57'
labels:
  - boiler-info
  - ui
dependencies: []
priority: medium
ordinal: 244000
---

## Description

<!-- SECTION:DESCRIPTION:BEGIN -->
The Boiler info page shows identification and parameters as one-column Device-info tables (label | value | meaning). The user wants them in the multi-column grid of the main OT monitor page (.otmontable/.otmonrow, CSS columns of 350 px) so all parameters fit on one screen. Design choice A (user, 17-10-2026): each cell shows 'P<n> <label>' and the value with unit; the option text and the factory setting move to a tooltip; a value that differs from the factory setting is highlighted. Ventilation member ID (ID74) and Solar member ID (ID103) are dropped from the page and from the REST response (never requested by the firmware, so no slots involved).
<!-- SECTION:DESCRIPTION:END -->

## Acceptance Criteria
<!-- AC:BEGIN -->
- [ ] #1 Identification (slave/master member ID, OpenTherm version, product type/version) renders as .otmonrow cells in an .otmontable grid on the Boiler info page
- [ ] #2 Parameters render as .otmonrow cells: 'P<n> <label>' and the value with unit; parameters without a profile entry show 'TSP <n>' and the raw value
- [ ] #3 The option text and the factory setting of a parameter are in a tooltip that opens on hover and on tap
- [ ] #4 A parameter whose value differs from the profile's factory setting is visually highlighted
- [ ] #5 Ventilation member ID and Solar member ID no longer appear on the page; value_74/support_74/value_103/support_103 are gone from /api/v2/boiler/info
- [ ] #6 Hidden and unsupported parameters stay hidden as before (tsp_unsupported_value, signed/allows255 rules unchanged)
- [ ] #7 test_otBrandTextUi.js and the boilerInfo host tests pass; python build.py exits 0; python evaluate.py --quick shows no new failures
- [ ] #8 Checked in a browser on the real OTGW (wide screen and phone width) with a screenshot
- [ ] #9 Operating status (status, substatus, fault) and OEM fields render as .otmonrow cells in the same grid; their meaning text is in the tooltip, as for parameters
- [ ] #10 Wide screen: Identification and Operating status at the top of columns 1-3, Parameters below them flowing over columns 1-3, OEM information last, in column 4 over the full height; narrow screen: blocks stack in that order (OEM last)
- [ ] #11 The parameter columns 1-3 hold an equal number of rows where possible (difference at most 1, e.g. 11/10/10 for 31 parameters), filled top to bottom per column
- [ ] #12 Fault history sits in column 4 below OEM; a profile with "fault_history": false hides the block and its read button; tzerra-m-28c-plus.json sets it (ID12/13 UNKNOWN-DATAID, matrix evidence); without a profile or key the block stays
<!-- AC:END -->

## Implementation Plan

<!-- SECTION:PLAN:BEGIN -->
1. index.html (Boiler info section): wrap the blocks in one container .boiler-grid with four areas: identification and operating status (top, columns 1-3), parameters (below, columns 1-3), OEM + fault history (column 4, full height). Buttons stay under their block.
2. index.css: .boiler-grid = CSS grid, 4 columns on wide screens; below the existing narrow breakpoint one column in the order identification, operating, parameters, OEM, fault history. Parameter block = inner grid, 3 columns, grid-auto-flow: column, rows = ceil(n/3) set from JS (difference at most 1 per column; 1 column on narrow screens).
3. index.js: boilerRow() renders .otmonrow / .otmoncolumn1 (label) / .otmoncolumn2 (value) like the main page; value column widened for boiler rows (texts like "dF 36 dU 1"). Meaning, option text and factory setting go into the row tooltip (title) and open on tap (row toggles a small line below the cell). Parameter value != profile default gets class boiler-changed (highlight). boilerFinishTable() no longer needed for the meaning column.
4. Identification: drop Ventilation/Solar member ID rows; brand/version/serial rows move into the same grid style.
5. Profile key "fault_history": false hides the fault history block and its button; set in tzerra-m-28c-plus.json.
6. OTGW-Core.ino sendBoilerInfoV2(): remove 74 and 103 from ids[].
7. Tests: update test_otBrandTextUi.js (row classes, tooltip text, highlight, hidden rows, fault_history, equal columns); host tests; build.py (firmware + filesystem); evaluate.py --quick.
8. Screenshots: headless chromium of the real OTGW page at wide and phone width after flashing (firmware + LittleFS together).
<!-- SECTION:PLAN:END -->

## Implementation Notes

<!-- SECTION:NOTES:BEGIN -->
Layout chosen by user 17-10-2026: parameters over columns 1-3, OEM last in column 4 ("achteraan"). Fault history block: not discussed yet; it shows "Not supported" on the Tzerra M Plus.
<!-- SECTION:NOTES:END -->
