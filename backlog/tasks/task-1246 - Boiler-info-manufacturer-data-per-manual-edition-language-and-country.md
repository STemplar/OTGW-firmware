---
id: TASK-1246
title: 'Boiler info: manufacturer data per manual edition (language and country)'
status: In Progress
assignee:
  - '@claude'
created_date: '2026-10-18 11:48'
updated_date: '2026-10-18 15:52'
labels:
  - boiler-info
  - ui
  - adr
dependencies: []
priority: low
ordinal: 248000
---

## Description

<!-- SECTION:DESCRIPTION:BEGIN -->
Model profiles hold manufacturer data from one manual edition: descriptions, status/fault texts, parameter labels, setting ranges and factory values per gas type. These depend on country and manual language (the NL manual 7618573 v.10 has G25 (L-gas) as factory gas, Tab.2 lists Netherlands only). The user wants Boiler info selectable for several languages/countries. Fixed page texts stay English (OpenTherm protocol terms, user decision 18-10-2026); member ID company names stay as in the official OTA list. Architecture first: ADR-099 (Proposed) must be accepted by the user before code.
<!-- SECTION:DESCRIPTION:END -->

## Acceptance Criteria
<!-- AC:BEGIN -->
- [ ] #1 ADR-099 is reviewed and accepted by the user (status Accepted set only after explicit approval)
- [ ] #2 The firmware and page select the profile edition as decided in ADR-099; the current Tzerra M 28c Plus profile becomes the NL edition without content changes
- [ ] #3 Without a matching edition the page falls back as decided in ADR-099 and never shows another country's factory values as if they applied
- [ ] #4 Host/UI tests cover edition selection and fallback; python build.py exits 0; python evaluate.py --quick shows no new failures
<!-- AC:END -->

## Implementation Notes

<!-- SECTION:NOTES:BEGIN -->
ADR-099 drafted (Proposed, commit above). Local bin/adr-lint reports "missing Alternatives Considered" for MADR files, same as Accepted ADR-098 (older lint does not know the MADR heading "Considered Options"). Waiting for user review.
<!-- SECTION:NOTES:END -->

## Final Summary

<!-- SECTION:FINAL_SUMMARY:BEGIN -->
Dropped by the user (2026-10-18) for lack of information; nothing implemented.

- No en-GB manual of the Tzerra M Plus exists; the model has a Dutch manual only for the Netherlands.
- Belgian manuals found and kept locally (nl-BE 7601052 v.11/v.13, fr-BE 7601050 v.11): texts equal to NL, factory values differ (P1 75, P2 55, P5 2, P31 2 vs NL 80/60/1/1), factory gas G20/G25 vs NL G25. Belgian models cannot be recognised from the data at hand.
- ADR-099 set to Rejected (commit d9c6299ad). Profile stays tzerra-m-28c-plus.json (NL 7618573 v.10).
<!-- SECTION:FINAL_SUMMARY:END -->
