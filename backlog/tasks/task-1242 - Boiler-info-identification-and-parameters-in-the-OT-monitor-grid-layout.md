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

## Implementation Notes

<!-- SECTION:NOTES:BEGIN -->
Layout chosen by user 17-10-2026: parameters over columns 1-3, OEM last in column 4 ("achteraan"). Fault history block: not discussed yet; it shows "Not supported" on the Tzerra M Plus.
<!-- SECTION:NOTES:END -->
