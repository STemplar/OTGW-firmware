---
id: TASK-1241
title: 'Boiler info: wait for PIC substitution slots instead of fixed timeouts'
status: In Progress
assignee:
  - '@claude'
created_date: '2026-10-17 10:42'
updated_date: '2026-10-17 10:42'
labels:
  - boiler-info
  - bug
dependencies: []
priority: medium
ordinal: 243000
---

## Description

<!-- SECTION:DESCRIPTION:BEGIN -->
The PIC sends a PM=<id> request only in a slot where the boiler answered UNKNOWN-DATAID to a thermostat request; it keeps one PM at a time, and a new PM replaces the old one. BoilerInfo.h gives each ID a fixed time (15 s; 90 s for ID93-95; 300 s for TSP/FHB; 360 s per read). Measured on a Remeha Tzerra M 28c Plus (17-10-2026, firmware 1.7.8-beta.2+d51c91f): idle cycle has one slot every ~23 s; after a PIC restart there were no slots for ~85 s (the PIC learns an unsupported ID after 3 UNKNOWN-DATAID replies). Result: ID125 was never sent (PM=125 replaced by PM=93 after 15 s), ID131 arrived after the 15 s timeout, and the read reported 'missing replies'. Design choice B (user): end waiting on observed slots, not wall time.
<!-- SECTION:DESCRIPTION:END -->

## Acceptance Criteria
<!-- AC:BEGIN -->
- [ ] #1 A host test replays the 17-10 identification read (slot every ~23 s) and fails on the old BoilerInfo.h: ID125 is not requested before PM=93 replaces it
- [ ] #2 The same host test passes on the fix: every identification ID is requested and either answered or marked unsupported, and the result is 'read finished'
- [ ] #3 Waiting for a PM reply ends when a gateway-substituted request (R frame) for another ID is seen, so a missing reply costs one slot, not a fixed time
- [ ] #4 A safety time limit remains for a bus without slots, and it is longer than the ~85 s no-slot period after a PIC restart
- [ ] #5 A reply that arrives after its wait ended does not leave read_status at 'missing replies' when all IDs were answered
- [ ] #6 Parameters (ID10/11) read completes on the bench with the measured slot rate, or the task notes record why it cannot and a follow-up task exists
- [ ] #7 test/host/test_boilerInfo.cpp passes; python build.py --firmware exits 0; python evaluate.py --quick shows no new failures
- [ ] #8 Bench re-run on the real boiler with an OT log capture shows ID125 answered and read_status 'read finished'
<!-- AC:END -->
