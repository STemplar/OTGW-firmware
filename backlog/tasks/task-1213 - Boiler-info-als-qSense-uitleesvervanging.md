---
id: TASK-1213
title: Boiler info als qSense-uitleesvervanging
status: Done
assignee:
  - '@codex'
created_date: '2026-10-16 15:34'
updated_date: '2026-10-16 16:03'
labels: []
dependencies: []
ordinal: 242000
---

## Description

<!-- SECTION:DESCRIPTION:BEGIN -->
Implementeer het met de gebruiker uitgewerkte Boiler info-ontwerp in STemplar/OTGW-firmware, branch otgw-1.x.x. Behoud bestaande app-informatiebalk. Volledige identificatieteksten eenmalig uitlezen en op aanvraag verversen; profielgebonden read-only status/TSP/FHB/OEM-weergave en LCD-notatie. De gebruiker heeft ontwerp en implementatie in deze chat geautoriseerd.
<!-- SECTION:DESCRIPTION:END -->

## Acceptance Criteria
<!-- AC:BEGIN -->
- [x] #1 Identificatieteksten ID93–95 hebben buffers, initiële uitleesreeks, begrensde fouten en handmatige refresh
- [x] #2 Boiler info toont profielgebonden waarden met letterlijke bronbetekenis en segmentnotatie; onbevestigde mappings blijven raw
- [x] #3 Actieve reads gebruiken passende PIC-route en ketelondersteuning; geen ongeautoriseerde writes
- [x] #4 Host- en UI-tests slagen; firmware en LittleFS worden samen gebouwd
- [x] #5 Diff en Nederlandse documentatie beschrijven libraries, werking en hardwarevalidatie
<!-- AC:END -->

## Implementation Plan

<!-- SECTION:PLAN:BEGIN -->
1. Controleer fork en ondersteunde PIC-leesroute. 2. Integreer identificatiebuffers en niet-blokkerende uitleescontroller. 3. Voeg read-only profiel/API en letterlijke UI met LCD-weergave toe. 4. Test foutpaden en UI; bouw firmware en LittleFS samen. 5. Documenteer diff, commit en push op de gevraagde branch.
<!-- SECTION:PLAN:END -->

## Implementation Notes

<!-- SECTION:NOTES:BEGIN -->
Branch otgw-1.x.x; coding agent Codex. Ontwerp en implementatie zijn eerder in deze chat door gebruiker geautoriseerd. Eerste create schreef de taak maar de automatische Git-stap faalde in de sandbox; autoCommit is voor deze werkcheckout uitgezet.

PIC-versie door gebruiker bevestigd: 6.8. Officieel otgw-6.8.tgz gecontroleerd: gateway.asm StringCharacter (regels 3399 e.v.) loopt ID93/94/95 automatisch af na PM. Geen vrije indexparameter nodig en geen PIC-aanpassing vereist. TSP/FHB-traversal via PM=10/12. Firmware bevat uitsluitend leesacties.

Validatie: hosttests voor OTBrandText en BoilerInfo slagen met g++ C++11, -Wall -Wextra -Werror. JS-syntax en UI-tests slagen. Bestaande OT-bus-liveness-regressie: 25 checks, 0 fouten. Echte webpagina gecontroleerd met synthetische REST-data in licht/donker, breed en 360/320 px; labels stapelen, lange tekst scrolt lokaal, geen pagina-overloop. Definitieve gecombineerde build met core 2.7.4 geslaagd: 772188 bytes flash, 57104 bytes globale RAM, 24816 bytes vrij voor heap/stack. Firmware, LittleFS, ELF, buildlog, screenshots en hashes lokaal in build/boiler-info-beta.2. Geen hardware geflasht. Praktijktest met PIC 6.8 en ketel nog uit te voeren; onzekere mappings blijven raw.

Aanvullende optionele UBSan-run niet beschikbaar: Fedora mist libubsan.so.1.0.0. De reguliere C++11-hosttests met -Werror zijn groen; geen sanitizer-resultaat geclaimd. De projectbuild heeft alle standaard versiecomments en data/version.hash automatisch naar beta.2 gesynchroniseerd. Er zijn geen functionele themawijzigingen buiten Boiler info.
<!-- SECTION:NOTES:END -->

## Final Summary

<!-- SECTION:FINAL_SUMMARY:BEGIN -->
Boiler info toegevoegd in STemplar/OTGW-firmware op otgw-1.x.x, prerelease 1.7.8-beta.2. PIC 6.8 ondersteunt de volledige tekenreeks via StringCharacter na één PM per identificatieveld. Drie RAM-buffers, initiële leesronde, handmatig verversen, begrensde leescontroller en retained MQTT-teksten zijn geïmplementeerd. De responsieve pagina heeft profielgebonden letterlijke Tzerra-beschrijvingen en 7-segmentnotatie. Alleen waargenomen OEM-read-ID’s komen na vendorherkenning in aanmerking; onzekere service-/FHB-mappings blijven raw. Geen ketel-writes. Host-/UI-/liveness-tests en gecombineerde firmware/LittleFS-build geslaagd. Echte hardwarevalidatie met PIC 6.8 en ketel is nog niet uitgevoerd en staat beschreven in de Nederlandse documentatie.
<!-- SECTION:FINAL_SUMMARY:END -->
