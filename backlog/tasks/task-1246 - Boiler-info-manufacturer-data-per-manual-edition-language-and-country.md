---
id: TASK-1246
title: 'Boiler info: manufacturer data per manual edition (language and country)'
status: In Progress
assignee:
  - '@claude'
created_date: '2026-10-18 11:48'
updated_date: '2026-10-18 11:48'
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
