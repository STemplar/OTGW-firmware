---
id: TASK-1243
title: 'Boiler info: fault status from blocking and lockout via the model profile'
status: To Do
assignee: []
created_date: '2026-10-17 12:24'
labels:
  - boiler-info
  - ui
dependencies: []
priority: medium
ordinal: 245000
---

## Description

<!-- SECTION:DESCRIPTION:BEGIN -->
The Operating status row 'Fault status' only looks at ID0 bit 0 (lockout). Remeha Tzerra M Plus manual 7618573 v.10 §11.1 distinguishes normal operation, blocking (tijdelijke blokkering, Tab.44/45: Su:0-Su:25, green blink) and lockout (vergrendeling, Tab.46/48: E:00-E:41, red blink, needs reset). The profile tzerra-m-28c-plus.json already holds the blocking codes as substatus 9:0-9:25 (matches Tab.45) and the E-codes as faults (Tab.48). That a blocking arrives as ID115 status 9 with substatus = Su-code is derived from manual and profile, not observed; it is verified against the user's qSense thermostat (member ID 140, MasterVersion 143/1, connected to the OTGW) before any code is written.
<!-- SECTION:DESCRIPTION:END -->

## Acceptance Criteria
<!-- AC:BEGIN -->
- [ ] #1 Verified against the qSense with a passive OT log: qSense status/substatus equals ID115 HB/LB during normal operation (at least two different states recorded with time)
- [ ] #2 Verified against the qSense: a blocking shown as Su:xx on the qSense arrives as ID115 status 9 / substatus xx, or the task notes record that no blocking could be observed and implementation waits
- [ ] #3 With a profile, Fault status shows 'Vergrendeling · E:xx <meaning>' when ID0 bit 0 is set, 'Blokkering · Su:xx <meaning>' when ID115 status is 9, else 'Geen blokkering of vergrendeling'
- [ ] #4 Without a profile the row behaves as before (ID0 bit 0 and raw ID5)
- [ ] #5 test_otBrandTextUi.js covers the three states with and without profile; python build.py exits 0; python evaluate.py --quick shows no new failures
<!-- AC:END -->
