---
id: TASK-1247
title: >-
  Boiler info: sister models (rebadged BDR Thermea boilers) in profile
  recognition and naming
status: To Do
assignee: []
created_date: '2026-10-18 18:15'
labels:
  - boiler-info
  - profile
dependencies: []
priority: low
ordinal: 248000
---

## Description

<!-- SECTION:DESCRIPTION:BEGIN -->
The OTGW boiler matrix (fetched 18-10-2026, Documentatie/Vendoronderzoek/ketelmatrix_id127_id131_2026-10-18.json with raw pages in ketelmatrix_details_html_2026-10-18/) shows the same ID127 under several brands of member ID 11 (BDR Thermea): 23/1 under Remeha Tzerra Ace (7 logs), De Dietrich NANEO EMC-S and De Dietrich MCR 24; 27/1 under Remeha Tzerra Ace-Matic (3 logs) and Baxi Platinum iCompact 26/26F. These are likely rebadged sister models sharing one controller (user, 18-10-2026), not proven. The Ace-Matic profile recognises on member 11 + ID127 27/1 without ID131, so a Baxi Platinum iCompact would be shown as "Remeha Tzerra Ace-Matic": tables probably right, name wrong. Matrix names are user-entered (De Dietrich MCR 24 has 7/25, 7/22, 13/17, 11/11 and 23/1).
<!-- SECTION:DESCRIPTION:END -->

## Acceptance Criteria
<!-- AC:BEGIN -->
- [ ] #1 Manuals of De Dietrich NANEO EMC-S and Baxi Platinum iCompact are located (or recorded as not found) with source, and their status/substatus/fault tables are compared with Remeha Tzerra Ace and Tzerra Ace-Matic; the result (identical, partly, different) is recorded per table
- [ ] #2 A profile that recognises without ID131 shows a platform name that does not claim one brand or model when sister models exist (e.g. 'Remeha Tzerra Ace-Matic (or sister model)'), and its tooltip lists the sister models seen in the matrix with source
- [ ] #3 A sister model is only treated as using the same tables when the comparison proves it; otherwise its values stay raw ('Meaning not yet confirmed')
- [ ] #4 test_otBrandTextUi.js and the boilerInfo host tests cover the platform name and the sister-model tooltip; python build.py exits 0; python evaluate.py --quick shows no new failures
<!-- AC:END -->
