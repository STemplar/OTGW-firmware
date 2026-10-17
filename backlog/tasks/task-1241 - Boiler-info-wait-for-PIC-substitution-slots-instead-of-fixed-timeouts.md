---
id: TASK-1241
title: 'Boiler info: wait for PIC substitution slots instead of fixed timeouts'
status: In Progress
assignee:
  - '@claude'
created_date: '2026-10-17 10:42'
updated_date: '2026-10-17 10:48'
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

## Implementation Plan

<!-- SECTION:PLAN:BEGIN -->
1. observe(): remember the ID of the last T frame. An R frame with a different ID is a substitution slot. While waiting: a slot for our request (target; ID11/13 during TSP/FHB traversal) is progress; a slot for any other ID more than 1 s after our PM (PIC echo is ~10 ms) means the PM was not used -> stop waiting, mark missing.
2. next(): replace the fixed waits (15/90/300 s) and the 360 s per-read cap with one safety limit: 180 s without progress (send, our slot, or our reply). Covers the ~85 s no-slot window after a PIC restart and loss of transport.
3. Host test: replay the measured identification read (slot every 23 s, ID3 answered passively by the thermostat) and a 46-slot parameters run; run against the old BoilerInfo.h (must fail) and the fix (must pass). Cover: same-ID override R is not a slot, a foreign slot within 1 s of PM is ignored, 85 s without slots keeps waiting. Update existing asserts that encode the old fixed waits.
4. Gates: host test, python build.py --firmware, python evaluate.py --quick.
5. Bench: identification + parameters read with OT log capture.
No 2.0.0 port: Boiler info exists only on otgw-1.x.x.
<!-- SECTION:PLAN:END -->

## Implementation Notes

<!-- SECTION:NOTES:BEGIN -->
Old-code measurements, 17-10-2026, firmware 1.7.8-beta.2+d51c91f, OT log via WebSocket port 81 (passive):
- Idle: one substitution slot every ~23.4 s (thermostat ID19 answered UNKNOWN-DATAID; PIC fills it with R, else its own ID116-123 list).
- PM has priority over the PIC list; the PIC holds one PM, a new PM replaces it.
- Identification read 12:20:31: PM=125 sent 12:20:45, no slot before the 15 s timeout, PM=93 at 12:21:00 took the 12:21:06 slot -> ID125 never sent. PM=131 sent 12:21:55, answered 12:22:16 (21.5 s), after the timeout -> "read finished with missing replies" although ID131 arrived.
- PIC restart 12:32:37: the PIC forwards ID19 three times (B7 to thermostat) before it substitutes again (12:34:01): ~85 s without slots.
- Parameters read 12:38:39: PM=10 in the first slot, then the PIC itself requested ID11 index 0..44, one slot per index (~23 s). Controller gave up at 12:44:39 (360 s cap); PIC kept going, values still stored passively (P1-P22 by 12:47). Full run = 46 slots, ~17.5 min.
- A same-ID R frame is a PIC override (TT/CS), not a slot: a slot is an R whose ID differs from the preceding T.
<!-- SECTION:NOTES:END -->
