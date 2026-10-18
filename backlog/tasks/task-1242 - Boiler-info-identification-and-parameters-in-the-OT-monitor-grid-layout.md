---
id: TASK-1242
title: 'Boiler info: identification and parameters in the OT monitor grid layout'
status: Done
assignee:
  - '@claude'
created_date: '2026-10-17 11:46'
updated_date: '2026-10-18 16:33'
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
- [x] #1 Identification (slave/master member ID, OpenTherm version, product type/version) renders as .otmonrow cells in an .otmontable grid on the Boiler info page
- [x] #2 Parameters render as .otmonrow cells: 'P<n> <label>' and the value with unit; parameters without a profile entry show 'TSP <n>' and the raw value
- [x] #3 A parameter whose value differs from the profile's factory setting is visually highlighted
- [x] #4 Ventilation member ID and Solar member ID no longer appear on the page; value_74/support_74/value_103/support_103 are gone from /api/v2/boiler/info
- [x] #5 Hidden and unsupported parameters stay hidden as before (tsp_unsupported_value, signed/allows255 rules unchanged)
- [x] #6 test_otBrandTextUi.js and the boilerInfo host tests pass; python build.py exits 0; python evaluate.py --quick shows no new failures
- [x] #7 Checked in a browser on the real OTGW (wide screen and phone width) with a screenshot
- [x] #8 Brand name, product version and serial number rows (ID93-95) are hidden when the boiler answered UNKNOWN-DATAID (support = unsupported); they show as before otherwise
- [x] #9 Operating status (status, substatus, fault status) and OEM fields render as .otmonrow cells in the same grid; status, substatus and fault status keep their short meaning in the cell (e.g. '8 · Regelstop'); OEM meanings go in the tooltip
- [x] #10 The option text and the factory setting of a parameter are only in the mouse tooltip (title); a click or tap does not expand the row
- [x] #11 Without a profile, once ID10 is known, TSP 0..count-1 are fixed cells ('TSP n'); values fill in as read (raw, READ-ACK 255 included), cells do not move
- [x] #12 The Hex/Binary toggle is removed; raw values are always shown in hex
- [x] #13 A profile with "fault_history": false hides the fault history block and its read button; tzerra-m-28c-plus.json sets it (ID12/13 UNKNOWN-DATAID); without a profile or key the block and button stay
- [x] #14 With a profile, all parameters of the profile (32 for the Tzerra M 28c Plus, manual Tab.34) are predefined cells in manual order; a value appears when it is read, cells do not move while reading
- [x] #15 Layout: columns follow the OT monitor page rule (at least 350px, equal width, as many as fit, no gap); Identification | Operating status | OEM take the first three; below them the parameters in one column as wide as the longest label plus value, without truncation; fault history (when shown) below; read buttons grouped at the bottom; narrow screens stack and may wrap long labels
- [x] #16 Parameter cells read 'P<n> <description> <value with unit>'; the descriptions start at the same position in every cell (P-number in a fixed-width part)
- [x] #17 A cell without received data shows '---' (instead of 'No data received yet', 'Support not yet determined', 'Not observed', 'No entries received', or an empty parameter value)
- [x] #18 The parameter tooltip follows manual Tab.34: description, 'Instelbereik' verbatim from the manual (one line per option, footnotes in parentheses) and 'Fabrieksinstelling 28c Plus'; all 32 profile parameters carry the Tab.34 setting range
- [x] #19 P17-P21 tooltips show the factory setting per gas type from manual Tab.34-41 (G25 L-gas base, G20, G25.3, G31; CLV overpressure for P19/P20) instead of a setting range; the highlight compares with the G25 (L-gas) value; the gas data lives in the profile (edition NL, 7618573 v.10)
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
Layout chosen by user 17-10-2026: parameters over columns 1-3, OEM last in column 4 ("achteraan"); fault history below OEM in column 4, hidden by the profile key fault_history=false (AC12).
AC9 revised with the user: operating status keeps its meaning in the cell.

User 18-10-2026: factory setting / option text only as mouse tooltip, no expand on click/tap (replaces the tap behaviour of the original AC3).

User 18-10-2026: choice 1b (only the profile parameters as predefined cells, not P0-P44) and 2a (parameters over columns 1-3, OEM in column 4). Like the OT monitor page: cells fixed, data filled in when detected.

Principle (user 18-10-2026): tailor-made view when the boiler type is known (profile: fixed manual parameter cells, labels, factory setting tooltip, highlight); generic view when it is not yet known (received TSPs as "TSP n", raw values). Implemented in 4e8232332.

User 18-10-2026: generic view also gets fixed cells from the ID10 count (yes); the Hex/Binary toggle adds nothing and is removed.

User 18-10-2026: 3 columns instead of 4 (more room for text and values); OEM next to operating status; read buttons grouped below the columns so the data columns form one whole, like the OT monitor page. Supersedes the column-4 layout of AC10/AC12.

User 18-10-2026: parameters in one wide column (no truncation) instead of three columns; replaces the equal-columns AC.

User 18-10-2026: tooltip also shows the setting range, laid out as in the manual (Tab.34, 7618573 v.10 p.58-59).

User 18-10-2026: "G25 (L-gas)" in Tab.34 is the gas type of the factory value, not a range (verified in the manual: §7.6.1 p.50, Tab.34-41 p.58-60). Gas data per edition in the profile; multi-language/country editions are a separate task.

Device check 18-10-2026 on the real OTGW (192.168.211.10), Anna connected:
- 2466531 (already flashed by the user): wide 1700px light/dark and 400px phone screenshots, DOM dump of the tooltips. All rows as specified; one defect: at phone width the label "Slave member ID" was truncated ("Slave membe...").
- Fix 5c73e1b: row labels wrap on narrow screens. Built (python3 build.py exit 0), evaluate --quick 0 failures, flashed over /update (LittleFS then firmware), device reports 1.7.8-beta.2+5c73e1b; phone screenshot shows "Slave member / ID" wrapped. After reboot: profile recognised after the identification read, ID93-95 rows hidden, 32 parameter cells with ---, fault history hidden.
- Screenshots: OTGW-captures/shots/2026-10-18_1825_device-2466531-*.png, 2026-10-18_183*_device-5c73e1b-*.png (outside the repo).
<!-- SECTION:NOTES:END -->

## Final Summary

<!-- SECTION:FINAL_SUMMARY:BEGIN -->
Boiler info page reworked to the OT monitor grid and the manual (Tzerra M 28c Plus, 7618573 v.10).

Changes:
- Identification, operating status and OEM information as .otmonrow cells in three equal columns (min 350px, as on the OT monitor page); parameters below in one column as wide as the longest row; fault history below that; all read buttons grouped under the grid. Narrow screens stack and wrap labels.
- Fixed cells: with a profile all profile parameters (32) in manual order, P-number in a fixed-width part so descriptions align; without a profile TSP 0..count-1 from ID10. Values fill in as read, cells do not move; cells without data show ---.
- Tooltips only (no click expand): Instelbereik verbatim from Tab.34, factory setting, and for P17-P21 the factory value per gas type plus CLV overpressure (Tab.34-41). Values differing from the factory (G25) setting are highlighted.
- Ventilation/Solar member IDs removed (UI and REST value/support_74/103). Hex/Binary toggle removed, raw values always hex. ID93-95 rows hidden on UNKNOWN-DATAID. Profile key fault_history:false hides the history block and button.

Tests: test_otBrandTextUi.js, test_boilerInfo.cpp, test_boilerInfoReplay.cpp pass; python3 build.py exit 0; evaluate --quick 0 failures; checked on the real OTGW with 5c73e1b (wide, phone, light, dark).
<!-- SECTION:FINAL_SUMMARY:END -->
