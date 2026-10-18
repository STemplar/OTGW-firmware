---
id: TASK-1245
title: 'Boiler info: OpenTherm member ID names from the official OTA list'
status: To Do
assignee: []
created_date: '2026-10-18 11:06'
labels:
  - boiler-info
  - ui
dependencies: []
priority: low
ordinal: 247000
---

## Description

<!-- SECTION:DESCRIPTION:BEGIN -->
Member IDs (ID2 master, ID3 slave, ID74 ventilation, ID103 solar; LB) come from one register managed by the OpenTherm Association (OT 4.2 §5.1/§5.3.2). The official list 'Assignment of Member ID Codes' (OTA, last update 13-06-2024, file Member-ID-Codes-2025.pdf, supplied by the user) gives the company per code. User decision: the names from this PDF are leading. Add them to docs/boiler-info/oem-vendors.json and the vendor research in Documentatie/Vendoronderzoek, and show the company next to the member IDs on the Boiler info page.
<!-- SECTION:DESCRIPTION:END -->

## Acceptance Criteria
<!-- AC:BEGIN -->
- [ ] #1 docs/opentherm specification/member-ids/ota-member-id-codes-2024-06-13.json holds every assigned code 1-251 with the company name exactly as in the PDF (asterisk kept as a flag), plus source and date
- [ ] #2 docs/boiler-info/oem-vendors.json and Documentatie/Vendoronderzoek/VENDOR_MEMBER_IDS_ACTUEEL.md name the company per member ID from that list; observations stay unchanged
- [ ] #3 The Boiler info page shows the company next to Slave and Master member ID (e.g. '11 · BDR Thermea Group B.V.', '140 · Plugwise B.V.'); 0 and unassigned codes show only the number
- [ ] #4 test_otBrandTextUi.js covers a known code, 0 and an unassigned code; python build.py exits 0; python evaluate.py --quick shows no new failures
<!-- AC:END -->
