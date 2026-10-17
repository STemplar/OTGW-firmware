---
id: TASK-1241
title: 'Boiler info: wait for PIC substitution slots instead of fixed timeouts'
status: Done
assignee:
  - '@claude'
created_date: '2026-10-17 10:42'
updated_date: '2026-10-17 11:57'
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
- [x] #1 A host test replays the 17-10 identification read (slot every ~23 s) and fails on the old BoilerInfo.h: ID125 is not requested before PM=93 replaces it
- [x] #2 The same host test passes on the fix: every identification ID is requested and either answered or marked unsupported, and the result is 'read finished'
- [x] #3 Parameters (ID10/11) read completes on the bench with the measured slot rate, or the task notes record why it cannot and a follow-up task exists
- [x] #4 test/host/test_boilerInfo.cpp passes; python build.py --firmware exits 0; python evaluate.py --quick shows no new failures
- [x] #5 Bench re-run on the real boiler with an OT log capture shows ID125 answered and read_status 'read finished'
- [x] #6 Waiting for a PM reply ends only on that reply or on the safety limit; there is no fixed wait per ID
- [x] #7 The safety limit is 300 s without progress (a sent request, a reply to it, or a next TSP/FHB index), above the ~85 s no-slot period plus PIC admin messages after a PIC restart
- [x] #8 read_status reports 'missing replies' only when the safety limit expired
<!-- AC:END -->

## Implementation Plan

<!-- SECTION:PLAN:BEGIN -->
Approved by user 17-10-2026 (replaces the earlier slot-detection plan).
1. BoilerInfo.h next(): drop the fixed waits (15/90/300 s) and the 360 s per-read cap; keep waiting until the reply arrives. One safety limit: 300 s without progress.
2. Progress = PM sent, reply to the target, or a new TSP/FHB index answered during PIC traversal.
3. No slot detection and no ESP retry: the PIC keeps the PM until a response with that ID and retries it in the next slot itself (OTGW 5.8 gateway.asm HandleResponse/SendPriorityMsg).
4. Host test: replay the 17-10 identification read and a 46-slot parameters run; fails on old code, passes on the fix. Update asserts that encode the old fixed waits.
5. Gates: host test, python build.py --firmware, python evaluate.py --quick. Bench re-run with OT log capture.
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

PIC source (OTGW 5.8 gateway.asm; user runs 6.8, behaviour matched the captures):
- One priority message (prioritymsgid); SetPrioMessage overwrites it and clears TSPIndex/TSPCount.
- PriorityMsg stays set until any response with that ID (HandleResponse), also a reply to a thermostat request for the same ID; no response -> retried in the next slot.
- InitCmdLoop: PIC admin messages (remote params, DHW/maxCH bounds, setpoints) take slots before the PM after a PIC reset.
- statusreturn: a new boiler fault sets PM=5 itself, overwriting ours (rare; covered by the safety limit).
- TSP traversal index 0..count-1 (TSPReadEntry). Tzerra M 28c Plus: count 45, index 0 DATA-INVALID, P1-P44 = manual 7618573 v.10 range; nothing missed.
- Full parameters run measured 12:38:42-~12:56 (46 slots).

Implementation 17-10-2026:
- BoilerInfo.h next(): fixed waits (15/90/300 s) and 360 s per-read cap replaced by one limit, 300 s without progress (progressAt: PM sent, reply to target, TSP/FHB entry). Expiry while waiting = that ID missing, next ID; expiry between requests (transport lost) = "read timed out".
- New test/host/test_boilerInfoReplay.cpp: emulates thermostat/PIC/boiler per OTGW 5.8 gateway.asm; 4 scenarios (identification 23.4 s slots with passive ID3, parameters 46 slots, first slot after 200 s, no slots).
- Old vs fix, same test: old fails all 4 (ID125 missing: PM=125@12s replaced by PM=93@27s, same as the 12:20:45 -> 12:21:00 capture; parameters stop at 300 s; restart loses ID125/131; no-slot gives up after 15 s). Fix passes all 4: identification read finished in 129 s, parameters in 1056 s.
- test_boilerInfo.cpp: timestamps after PM=95 shifted to the 300 s limit; assertions unchanged, one added (no send at 310999 ms).
- Gates: both host tests PASS, test_otBrandText PASS, test_otBrandTextUi.js PASS, evaluate.py --quick 41 checks 0 failed, build.py --firmware OK (772680 B flash, 24644 B free RAM).
- Open: AC3/AC5 need the bench run on the boiler.

Bench, firmware 1.7.8-beta.2+2c0a0be + matching LittleFS, PIC 6.8, OT log via WebSocket 81:
- Boot identification read started by itself 13:36:46 (right after reboot, after PR=M). One PM per ID, each in its own slot (~23 s): PM=125 13:37:15 -> R125 13:37:38, READ-ACK 0x0300 (OT 3.00), 23 s after the PM (old code gave up at 15 s). ID93-95 UNKNOWN-DATAID, ID131 0x2401. Profile remeha-tzerra-m-28c-plus. read_status "read finished" at 13:39:14 (2.5 min).
- Parameters read started 13:39:31 (result pending).

- Parameters read on the fix: PM=10 13:39:33, PIC traversal R11 index 0..44 one slot each, last R11 index 44 at 13:56:49; read_status "read finished" at 13:56:49 (17 min 16 s); tsp_entries P1-P44 complete, none missing.
<!-- SECTION:NOTES:END -->

## Final Summary

<!-- SECTION:FINAL_SUMMARY:BEGIN -->
Boiler info reads now wait for the PIC reply instead of giving up after fixed timeouts.

Why: the PIC sends a PM only in a free slot (a thermostat request the boiler answers with UNKNOWN-DATAID), keeps it until a response with that ID arrives and retries it itself; a new PM replaces it. On the Tzerra M 28c Plus there is one slot every ~23 s and none for ~85 s after a PIC restart. The 15 s wait per ID made the controller send the next PM and lose the pending one (ID125 was never sent), and the 360 s cap stopped a 46-slot parameters read after a third.

Changes:
- BoilerInfo.h next(): fixed waits (15/90/300 s) and the 360 s per-read cap replaced by one limit, 300 s without progress (PM sent, reply, next TSP/FHB index). Expiry while waiting marks that ID missing; expiry between requests (transport lost) ends the read as "read timed out".
- New test/host/test_boilerInfoReplay.cpp: thermostat/PIC/boiler model after OTGW 5.8 gateway.asm, four scenarios (23.4 s slots with a passive ID3, 46-slot parameters run, first slot after 200 s, no slots).
- test_boilerInfo.cpp: timestamps after PM=95 moved to the new limit; assertions unchanged plus one.
- docs/boiler-info/OVERDRACHT.md: read timing and the replay test.

Evidence (same reproduction, old vs fix):
- Replay test: old BoilerInfo.h fails all four scenarios (PM=125 replaced by PM=93 after 15 s, as in the 12:20 capture); the fix passes all four.
- Bench, 1.7.8-beta.2+2c0a0be, PIC 6.8, OT log via WebSocket: boot identification read right after reboot, PM=125 answered 23 s later (OT 3.00), "read finished" in 2.5 min; parameters read P1-P44 complete, "read finished" in 17 min 16 s. Before the fix both reads ended with "missing replies".
- Gates: host tests (boilerInfo, boilerInfoReplay, otBrandText, otBrandTextUi.js) pass; evaluate.py --quick 0 failed; build.py OK (772680 B flash, 24644 B free RAM).

Follow-up: TASK-1242 (grid layout of the Boiler info page).
<!-- SECTION:FINAL_SUMMARY:END -->
