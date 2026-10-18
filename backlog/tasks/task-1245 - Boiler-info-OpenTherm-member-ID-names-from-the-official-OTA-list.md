---
id: TASK-1245
title: 'Boiler info: OpenTherm member ID names from the official OTA list'
status: In Progress
assignee:
  - '@claude'
created_date: '2026-10-18 11:06'
updated_date: '2026-10-18 11:17'
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
- [x] #1 docs/opentherm specification/member-ids/ota-member-id-codes-2024-06-13.json holds every assigned code 1-251 with the company name exactly as in the PDF (asterisk kept as a flag), plus source and date
- [x] #2 docs/boiler-info/oem-vendors.json and Documentatie/Vendoronderzoek/VENDOR_MEMBER_IDS_ACTUEEL.md name the company per member ID from that list; observations stay unchanged
- [ ] #3 The Boiler info page shows the company next to Slave and Master member ID (e.g. '11 · BDR Thermea Group B.V.', '140 · Plugwise B.V.'); 0 and unassigned codes show only the number
- [ ] #4 test_otBrandTextUi.js covers a known code, 0 and an unassigned code; python build.py exits 0; python evaluate.py --quick shows no new failures
- [ ] #5 The member ID tooltip lists the company's brands where a source is recorded (BDR Thermea, codes 11 and 53: Baxi, De Dietrich, Remeha, Brötje, Chappée, Baymak - from the BDR Thermea website, supplied by the user 18-10-2026); codes without a recorded source show only the company
<!-- AC:END -->

## Implementation Notes

<!-- SECTION:NOTES:BEGIN -->
- d022e475a: official list as JSON (188 codes, 17 with asterisk; counts checked against the PDF), company added to 323 member_id entries in oem-vendors.json (observations unchanged), section "Officiële bedrijfsnamen" in Documentatie/Vendoronderzoek/VENDOR_MEMBER_IDS_ACTUEEL.md (outside the repo), data/member-ids.json for the page.
- Observed IDs not assigned by the OTA list: 170, 211, 255; mismatches with matrix brands: 56 (Ti-Green vs Baxi/De Dietrich), 64 (WeHeat vs Intergas/Nefit). Observations left as is; page shows only the number for unassigned codes.
- Pending: AC3 (on the real OTGW) and the gates already green in the commit (UI test, evaluate 0 failed, build OK).

User 18-10-2026: tooltip shows the brand portfolio from the company (source: company website), not matrix observations; brands are brand names, not product models (e.g. Siemens, not Siemens QAA73).
<!-- SECTION:NOTES:END -->
