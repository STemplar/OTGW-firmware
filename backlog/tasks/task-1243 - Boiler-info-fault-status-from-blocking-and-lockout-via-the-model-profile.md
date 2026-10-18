---
id: TASK-1243
title: 'Boiler info: fault status from blocking and lockout via the model profile'
status: Done
assignee:
  - '@claude'
created_date: '2026-10-17 12:24'
updated_date: '2026-10-18 17:39'
labels:
  - boiler-info
  - ui
dependencies: []
priority: medium
ordinal: 245000
---

## Description

<!-- SECTION:DESCRIPTION:BEGIN -->
The Operating status row 'Fault status' only looks at ID0 bit 0 (lockout). Remeha Tzerra M Plus manual 7618573 v.10 §11.1 distinguishes normal operation, blocking (tijdelijke blokkering, Tab.44/45: Su:0-Su:25, green blink) and lockout (vergrendeling, Tab.46/48: E:00-E:41, red blink, needs reset). The profile tzerra-m-28c-plus.json already holds the blocking codes as substatus 9:0-9:25 (matches Tab.45) and the E-codes as faults (Tab.48). That a blocking arrives as ID115 status 9 with substatus = Su-code is derived from manual and profile, not observed; it is verified against a Remeha qSense thermostat before any code is written. Thermostats seen 17-10-2026: the regular one sends ID2 0x008C (member ID 140, MasterVersion 143/1); the qSense, connected for a few minutes after 13:59, sends ID2 0x010B (Smart Power, member ID 11).
<!-- SECTION:DESCRIPTION:END -->

## Acceptance Criteria
<!-- AC:BEGIN -->
- [x] #1 Verified against the qSense with a passive OT log: qSense status/substatus equals ID115 HB/LB during normal operation (at least two different states recorded with time)
- [x] #2 Without a profile the row behaves as before (ID0 bit 0 and raw ID5)
- [x] #3 test_otBrandTextUi.js covers the three states with and without profile; python build.py exits 0; python evaluate.py --quick shows no new failures
- [x] #4 Blocking verification closed by user decision 18-10-2026: two gas tests with the Plugwise Anna (PM=115) showed flame loss -> restart attempts (status 2) -> lockout E:14, never ID115 status 9; the blocking path is built from manual and profile and stays marked as not observed in the profile
- [x] #5 With a profile that names its blocking and lockout status (fault_status), Fault status shows '<lockout label> · E:xx <meaning>' when ID0 bit 0 is set, '<blocking label> · Su:xx <meaning>' when ID115 status is the blocking status, else 'No blocking or lockout'; labels come from the profile's status table (Tzerra: 'Vergrendeling', 'Blokkering'), fixed text stays English
<!-- AC:END -->

## Implementation Notes

<!-- SECTION:NOTES:BEGIN -->
Thermostat facts 17-10-2026 (OT log via WebSocket 81):
- Regular thermostat: ID2 0x008C (flags 0, member ID 140), ID126 MasterVersion 143/1. Brand not yet known.
- Remeha qSense (connected 14:31-14:36): ID2 0x010B (Smart Power flag, member ID 11), ID126 MasterVersion 35/12; label on the PCB: v12, Soft 1.2 (user). Uses Smart Power: PIC reports "* Medium power" / "* Low power".
- Boiler info "Master member ID" shows the last ID2 seen: 11 after the qSense, 140 with the regular thermostat. Not a firmware fault.

Verification 17-10-2026 (qSense connected, OT log via WebSocket 81, firmware 5882f68):
- Lockout: gas tap closed by the user, heat demand at 15:24:58 (ID0 slave 0x02 CH mode). 15:26:01 ID0 slave 0x01 (fault bit), 15:26:04 ID5 = 0x000E. qSense showed "E014" with the warning triangle; boiler blinked red (lockout). => lockout = ID0 bit 0 + ID5 LB = E-code, confirmed (E:14 "5 mislukte branderstarts", profile faults["14"]).
- Blocking: the user saw Su:22 on the qSense before the lockout, but no ID115 frame was on the bus then (OTGW had just restarted after flashing 5882f68 at ~15:21, and the qSense did not request ID115). Blocking via ID115 status 9 / sub xx still unverified (AC2 open).
- Power cycle 14:45: after power-on ID115 = status 17 (Ontluchten) 14:47:39-14:50:06; no status 9 seen (ID115 read only occasionally).

- Boiler status signalling during the E:14 lockout: 3x red = Storingscode 3 "Ontstekingsfout" (manual v.10 Tab.46 lists E:14 under group 3). Boiler, qSense, OT log and manual agree.

- Correction (user): Su:22 was the user's expectation, not shown on the qSense. No blocking was observed before the E:14 lockout; the earlier note "the user saw Su:22" is wrong. Blocking (ID115 status 9) remains unverified.

AC1 verified 18-10-2026 (capture OTGW-captures/2026-10-18_1157_qsense-st-su.log, persistent folder outside the repo): with the qSense Informatiemenu open on St/Su the qSense read ID115 four times in 2 min; the user confirmed the qSense showed the same values: 11:57:12 0x031E = St 3 / Su 30 (Branden op CV-bedrijf / Temperatuurregeling), 11:58:33 0x0529 = 5 / 41 (Branderstop / Naventileren), 11:58:56 0x041E = 4 / 30 (SWW-bedrijf actief / Temperatuurregeling). => ID115 HB = St, LB = Su.
Note: the raw captures of 17-10 were lost (stored under /tmp); facts above in these notes remain.
qSense gives no free slots (requests only IDs the boiler knows; unsupported ones get DATA-INVALID, not UNKNOWN-DATAID): out of scope (user). Regular thermostat Plugwise Anna (member ID 140) goes back; AC2 (blocking) will use PM=115 with the Anna plus the boiler green blink code.

Test 18-10-2026 with the Plugwise Anna (capture OTGW-captures/2026-10-18_1849_anna-blocking-su22.log, outside the repo; PM=115 every 20 s via POST /api/v2/otgw/commands, read only; firmware 091d2f9). Gas tap closed by the user while the burner was burning:
- 18:58:55 ID0 CH+flame; 18:59:05 ID115 0x031E (3/30 burning CH).
- 18:59:19 ID0 0x00 (flame lost); 18:59:21 ID5 HB bit0 set, OEM 255 (transient).
- 18:59:28 ID115 0x0212 (2/18 main ignition); 18:59:51 and 19:00:15 0x0214 (2/20 tussenventileren).
- 19:00:29 ID0 fault bit; 19:00:31 ID5 OEM fault 14 (E:14, 5 mislukte branderstarts); 19:01:01 ID115 0x0A00 = status 10 / sub 0: NOT in manual Tab.43 (lists 0-6, 8, 9, 17). Lockout has its own status 10 (user hypothesis: lockout takes precedence).
- User saw green blinking during the restart attempts, red at the end; opened gas and reset. 19:01:17 ID0 0x00 and ID5 bit0 again transient; 19:01:25 ID115 3/37; 19:01:40 CH+flame.
- Result: no status 9 (blocking) on the bus. Flame loss went straight to ignition attempts (status 2) and then lockout. Green blinking here = restart attempts, not a Su-code blocking. AC2 stays open: blocking via ID115 status 9 not observed.

- Gap: ID115 was not sampled between 18:59:05 and 18:59:28; flame loss at 18:59:19 falls in that gap, so a short Su:22 cannot be excluded. During the restart attempts and the lockout there was no status 9. During lockout one ID115 sample: substatus 0, not the E-code (the E-code is only in ID5).
- Su:22 is not marked (1) in Tab.45, so it would be stored in the boiler fault memory. The Tzerra M Plus has no display; the memory is readable only with Recom/the service tool or a suitable controller (manual §8, §11). The qSense has been removed (user), so this check is not possible now.
- Profile: status 10 "Vergrendeling" added with status_note (observed, not in the manual), commit a138164, UI test (fails on the old profile, passes on the new), build exit 0, evaluate 0 failures, flashed LittleFS+firmware; device serves the profile with status 10.

Test 2, 18-10-2026, gas tap half closed (captures OTGW-captures/2026-10-18_1916_anna-halfgas-su22.log, _1913_ and _1924_ counters; PM=115 every 20 s):
- Lamp "1x green" while the bus said 3/30 + flame, no fault flags: manual §14.2 Gebruikersinstructiekaart: "Ketel in bedrijf" = onderbroken groen (CV-warmte / warm tapwater); blocking = knipperend groen. The green seen was most likely normal operation, also at 18:59.
- 19:21:12 ID0 0x00 (flame lost); 19:21:14 ID5 HB bit0, OEM 255; ID115 19:21:21 2/15, 19:21:44 2/20, 19:22:08 2/18, 19:22:31 0x0529 5/41 (branderstop/naventileren); lamp off meanwhile.
- 19:22:45 ID0 fault bit, 19:22:47 ID5 OEM 14 (E:14), lamp red. No ID115 sample during this lockout. Reset ~19:23:10: 19:23:12 ID0 still fault bit but ID5 HB bit0 set and OEM 255; 19:23:33 ID0 0x00; 19:23:42 3/37; 19:23:56 flame.
- Counters: ID113 33 (12:00) -> 37 (19:13) -> 42 (19:25); ID114 7 -> 8 -> 9. Each flame loss counts once in ID114; failed restarts in ID113.
- Again no status 9 and no OEM code 22 in ID5. ID5 HB bit0 ("service request" in OT 4.2) set on the first ID5 after each flame loss and after each reset (4 of 4), never during normal burning or during lockout: candidate Remeha signal for flame loss/restart, not confirmed.
- Conclusion for AC2: a flame loss does not produce a Su-code blocking on this boiler; it goes straight to restart attempts (status 2) and after the failed starts to lockout E:14. Blocking via ID115 status 9 is still not observed.

Implementation 0162c4a: profile key fault_status {blocking: 9, lockout: 10} plus fault_status_note; index.js shows one Fault status row: lockout (ID0 bit 0) "<status[10]> · E:xx <faults meaning>", else blocking (ID115 HB == 9) "<status[9]> · Su:xx <Tab.45 meaning>", else "No blocking or lockout". Without fault_status unchanged. Status 10 supported by the Ace-Matic manual ("Permanente storing (handmatige reset nodig)"). Tests: new UI assertions fail on the old code and pass on the new; boilerInfo host test pass; build exit 0; evaluate 0 failures; flashed LittleFS+firmware 0162c4a; device shows "Fault status | No blocking or lockout" with the profile recognised.
<!-- SECTION:NOTES:END -->

## Final Summary

<!-- SECTION:FINAL_SUMMARY:BEGIN -->
Fault status on the Boiler info page now distinguishes lockout and blocking through the model profile.

Changes:
- Profile key fault_status names the blocking and lockout status; the row reads "<lockout> · E:xx <meaning>" when ID0 bit 0 is set (E-code from ID5 LB, meaning from the profile faults table), "<blocking> · Su:xx <meaning>" when ID115 reports the blocking status, else "No blocking or lockout". A lockout goes before a blocking. Without fault_status (generic, Ace-Matic) the row is unchanged (ID0 bit 0 and raw ID5).
- Tzerra M 28c Plus profile: status 10 "Vergrendeling" (observed during E:14, not in manual Tab.43; Ace-Matic manual lists it as permanent fault), fault_status {9, 10}, notes on what is observed.

Evidence (OTGW-captures, outside the repo): qSense St/Su = ID115 HB/LB (AC1); two gas tests with the Plugwise Anna and PM=115: flame loss -> restart attempts (status 2) -> lockout E:14 (ID0 bit 0, ID5 0x000E, ID115 0x0A00); a blocking (status 9) was never observed and the blocking path follows manual Tab.45 (user decision 18-10-2026). Counters ID113/114 rise with each test. ID5 HB bit0 appears on each flame loss and reset: noted, not used.

Tests: test_otBrandTextUi.js (lockout, blocking, lockout before blocking, none, without profile) fail on the old code and pass; test_boilerInfo.cpp pass; build exit 0; evaluate 0 failures; checked on the real OTGW (0162c4a).
<!-- SECTION:FINAL_SUMMARY:END -->
