# Overdracht: Boiler info (otgw-1.x.x) naar Claude Code

Stand: 18-10-2026. Repo `STemplar/OTGW-firmware`, branch **`otgw-1.x.x`**, HEAD `0162c4adb` (+ deze docs-commit).
Upstream: `rvdbreemen/OTGW-firmware`, branch `otgw-1.x.x` (de fork loopt er alleen op vóór).
Werk **alleen met de fork**; de upstream-repo is voor de gebruiker niet toegankelijk en is niet nodig.

### Ophalen zonder de privé-submodule

`.gitmodules` bevat `other-projects` → `rvdbreemen/OTGW-other-projects`. Die repo is niet openbaar,
dus `git clone --recurse-submodules` faalt daarop. Hij is niet nodig voor bouwen of testen. Gebruik:
```
git clone --branch otgw-1.x.x https://github.com/STemplar/OTGW-firmware.git
cd OTGW-firmware
git submodule update --init src/libraries/OpenTherm src/libraries/SimpleTelnet
```
(`build.py` initialiseert die twee ook zelf als ze ontbreken.) Probeer `other-projects` niet op te halen
en voeg geen `upstream`-remote toe.
Eigenaar en tester: STemplar (Remeha Tzerra M 28c Plus, PIC 6.8, ESP8266 D1 mini).

Lees na dit document ook `CLAUDE.md` (projectregels) en `UITBREIDING_IDENTIFICATIETEKSTEN.md`
(functionele beschrijving van Boiler info).

---

## 1. Wat dit project doet

Een pagina **Boiler info** in de OTGW-web-UI die de ketel via de PIC uitleest en de waarden
toont met de betekenissen uit de handleiding van het ketelmodel (qSense-vervanger):

- identificatie: member-ID (ID3), producttype/-versie (ID127), OT-versie (ID125), merk/versie/
  serienummer (ID93–95), en bij Remeha Tzerra M Plus de typecode dF/dU (ID131);
- bedrijfsstatus (ID115: status/substatus) en storingscode (ID5);
- parameters (TSP, ID10/11) en foutgeheugen (FHB, ID12/13);
- OEM-velden (ID128–255) alleen voor waargenomen fabrikanten.

Alles is **alleen-lezen**. Er worden geen writes naar de ketel gedaan.

## 2. Vaste regels (door de gebruiker gesteld, niet onderhandelbaar)

1. **OpenTherm 4.2 strikt volgen**: `docs/opentherm specification/OpenTherm-Protocol-Specification-v4.2.pdf`.
   - ID5 LB, ID115 en ID128–255 zijn OEM-specifiek: betekenis alleen via een modelprofiel.
   - TSP/FHB: READ-ACK = geldige waarde (ook 255); DATA-INVALID = index ongedefinieerd;
     UNKNOWN-DATAID = niet ondersteund (§5.3.6).
2. **Geen writes.** Ook niet de dF/dU-reset die een qSense via OpenTherm kan doen, niet het
   wissen van de servicemelding (Write-Data(1,255)), geen SCU-detectie.
3. **Een profiel is model-specifiek** (bijv. alleen Tzerra M 28c Plus, niet "Tzerra"), en noemt
   documentnummer + versie van de handleiding. Handleidingversies verschillen inhoudelijk
   (Tzerra M Plus v05 had P22, v.10 niet).
4. **Bewijs uit de OTGW-ketelmatrix** (otgw.tclcode.com/matrix.cgi): alleen een ingevulde waarde
   (ondersteund) en ✖ (UNKNOWN-DATAID) tellen. **Een lege cel is géén bewijs** — de logs zijn vaak
   korte, onvolledige scans. De matrix is vanuit de cloud-sandbox niet bereikbaar; de gebruiker
   levert screenshots.
5. **Niet gokken.** Onbevestigde koppelingen blijven ruw ("Meaning not yet confirmed") of
   staan als concept buiten de firmware.

## 3. Architectuur

| Onderdeel | Bestand | Rol |
|---|---|---|
| Uitleescontroller | `src/OTGW-firmware/BoilerInfo.h` | PM-commando's (PIC 6.8 doet de index-traversal), correlatie request/reply, time-outs, support-status, TSP/FHB-opslag |
| Profieltabel | `src/OTGW-firmware/BoilerProfiles.h` | `kBoilerModelProfiles`: **enige plek** voor modelherkenning (member, ID127-type/versies, optioneel ID131) + bestandsnaam; `boilerOEMReadAllowed()` = waargenomen OEM-read-ID's per member |
| REST | `OTGW-Core.ino` → `sendBoilerInfoV2()`, route `/api/v2/boiler/info`, reads via `POST /api/v2/boiler/read/<groep>` | Levert waarden, support, `profile` en `profile_file` |
| UI | `data/index.html`, `data/index.js` (`renderBoilerInfo`, `renderBoilerDetails`, `boilerRow`, `boilerFinishTable`) | Tabellen in Device info-stijl, 3 kolommen (label/waarde/betekenis) waar nodig |
| Profielen | `data/tzerra-m-28c-plus.json`, `data/tzerra-ace-matic.json` | Betekenissen; de UI laadt alleen het bestand dat de firmware noemt |

### Herkenning (huidige tabel)

| Profiel-ID | Voorwaarde |
|---|---|
| `remeha-tzerra-m-28c-plus` | ID3 LB = 11, ID127 = 13/14 of 13/16, ID131 = 0x2401 (dF 36 / dU 1) |
| `remeha-tzerra-ace-matic` | ID3 LB = 11, ID127 = 27/1 |
| `generic` | al het andere |

ID131 wordt bij het opstarten alleen gelezen als een passende tabelregel `oem131` vraagt.
Nieuw model = één regel in de tabel + profielbestand in `data/` (≤ 30 tekens, LittleFS).
Een hosttest controleert dat elk tabelbestand bestaat en hetzelfde `id` heeft.

### Profielformaat (relevante sleutels)

`id`, `name`, `source`, `models` (dF/dU → typenaam), `oem_dfdu`, `status`, `substatus`
(`"status:sub"`) of `substatus_any` (los van status), `faults`, `tsp_unsupported_value`
(alleen als de ketel afwijkt van §5.3.6), `parameters` (`label`, `unit`, `range`, `default`,
`options`, `signed`, `allows255`).

## 4. Bevestigde kennis

- **Ketel gebruiker**: Remeha Tzerra M 28c Plus, typeplaat dF 36 / dU 1, ID127 13/16, OT 3.00.
  Matrixlogs 2022-10-14 en 2022-10-08 zijn deze ketel.
- **TSP-index = P-nummer** (live uitlezing 16-10). Handleiding 7618573 v.10, Tab.34.
  Ketel antwoordt READ-ACK 255 voor ontbrekende P (P6–P16, P22) → `tsp_unsupported_value: 255`.
  P27/P30 signed (241 = −15, 246 = −10). P17 staat op 15 (fabriek 39).
- **ID131 = HB dF, LB dU** (bevestigd met typeplaat). 0x2501 (dF 37/dU 1) gezien bij 13/14: geen typenaam.
- **ID15** = ingesteld maximaal vermogen, niet het typevermogen (vermogen is instelbaar). Gemeten
  18-10-2026: 6 kW / 73 %, vermoedelijk door P17 = 15.
- **Tzerra-generaties**: 1 Tzerra M, 2 Tzerra M Plus (13/13, 13/14, 13/16), 3 Tzerra Ace
  (23/1, 7 matrixlogs), 4 Tzerra Ace-Matic (27/1, OT 4.00, ID10–13 ✖).
- **ID127 is niet uniek per model**: Avanta Ace en Calenta Ace beide 20/1; 5/24 onder Avanta én Calenta.
  Overzicht in `docs/boiler-info/profiles-draft/README.md`.
- **ID127/ID131 van alle 31 BDR-ketels in de matrix** (opgehaald 18-10-2026):
  `Documentatie/Vendoronderzoek/ketelmatrix_id127_id131_2026-10-18.json` + ruwe pagina's in
  `ketelmatrix_details_html_2026-10-18/` (buiten de repo). ID127 hoort bij een besturingsgeneratie,
  niet bij een model of vermogen. Zelfde ID127 onder meerdere merken: 23/1 (Tzerra Ace, De Dietrich
  NANEO, MCR 24), 27/1 (Tzerra Ace-Matic, Baxi Platinum iCompact) → vermoedelijk zustermodellen,
  TASK-1247 (To Do). ID131 alleen bij de Tzerra Plus: 0x2401 en 0x2501.
- **Modelvermogen** komt alleen uit ID131 dF/dU (36/1 = 28c Plus, van de typeplaat). De handleiding
  heeft geen dF/dU-tabel. Parameters (P18 47/56/78, P21 23/30 voor 24c/28c/39c) zijn instelbaar en
  dus hooguit een controle.
- **Member-ID 2 (Bulex FAS, Quinta 28c) uitgesloten**: hun "OEM-reads" 128–202 zijn een
  geheugendump (LB(n) == HB(n+1)). Vastgelegd in `oem-vendors.json` → `excluded_as_invalid`.
- **Oude Quinta displaycodes** I, II, J, L, u (gebruiker) / U (service): door gebruiker bevestigd.
- **Uitleestempo (TASK-1241)**: de PIC stuurt `PM` alleen in een slot waar de ketel een
  thermostaatverzoek met UNKNOWN-DATAID beantwoordt (bij deze thermostaat ID19, elke ~23 s), houdt
  één `PM` vast tot het antwoord en herhaalt het zelf; een nieuw `PM` vervangt het. Na een
  PIC-herstart ~85 s geen slots. Volledige parameterronde = 46 slots ≈ 18 min. Bron: OT-log via
  WebSocket poort 81 en `Brink OTGW/otgw-5.8.X/gateway.asm`. `BoilerInfo.h` wacht daarom op het
  antwoord met één vangnet van 300 s zonder voortgang.
- **Remeha qSense geeft geen slots** (17/18-10-2026): hij vraagt alleen ID's die de ketel kent; wat de
  ketel niet heeft, krijgt DATA-INVALID (geen UNKNOWN-DATAID). Met de qSense kan de OTGW dus zelf niets
  uitlezen (Boiler info: "missing replies", profiel niet herkend na een herstart). Buiten de scope
  van dit project. De vaste thermostaat (Plugwise Anna, member-ID 140) geeft via ID19 elke ~23 s een slot.
- **ID115 = St/Su op de qSense** (HB = St, LB = Su), bevestigd 18-10-2026 bij 3/30, 5/41 en 4/30.
  Verder gezien (Anna + PM=115, 60 antwoorden): 2/15, 2/18, 2/20, 3/30, 3/37, 4/30, 5/41, 5/44,
  6/60, 8/0, 8/1, 10/0, 17/0. Status 0, 1 en 9 nooit gezien.
- **Storing en blokkering (TASK-1243, 18-10-2026)**, twee gastests met de Anna, PM=115 elke 20 s
  (captures `OTGW-captures/2026-10-18_1849_*` en `_1916_*`, buiten de repo):
  - Vergrendeling = ID0 bit 0 + E-code in ID5 LB (E:14 = `0x000E`, de letter E wordt niet verstuurd)
    + ID115 **status 10 / sub 0** (niet in handleiding Tab.43; de Ace-Matic-handleiding noemt 10
    "Permanente storing"). De ID5-vlaggen (HB) blijven 0.
  - Vlamwegval (kraan dicht of half dicht tijdens branden) → direct herstartpogingen (status 2:
    2/15, 2/18, 2/20, 5/41) → E:14. **Nooit status 9** en nooit code 22 in ID5. Blokkering via
    status 9 / Su-code is dus niet waargenomen; het profiel volgt daar Tab.45.
  - ID5 HB bit 0 ("service request") staat aan op het eerste ID5 na elke vlamwegval en na elke reset
    (4 van 4), nooit tijdens branden of vergrendeling: kandidaat-signaal, niet gebruikt.
  - Tellers: ID113 (mislukte starts) 33 → 37 → 42, ID114 (vlamsignaal te laag) 7 → 8 → 9; elke
    vlamwegval telt één keer in ID114.
  - Lamp: onderbroken groen = ketel in bedrijf (Gebruikersinstructiekaart §14.2), knipperend groen
    (1–6×) = blokkering, rood = storing. De Tzerra M Plus heeft geen display; het storingsgeheugen
    is alleen via Recom/servicetool of een regelaar (qSense) te lezen.
- **Thermostaatcycli (18-10-2026)**:
  - Anna: drietallen `25 17 X`, vaste ronde ~23,4 s (X = 0 3 5 1 14 15 16 17 18 19 24 26 27 28 48 56
    9 100 57 49 6, plus 35 ~2× en 99 soms); 3,32 verzoeken/s; ID17/25 elke ~0,85 s. ID19 is het enige
    ID dat de ketel niet kent → het enige PM-slot; alle 111 PM-verzoeken van 18-10 namen dat slot,
    de Anna krijgt van de PIC hetzelfde Unknown-Data-Id.
  - qSense: vijftallen `17 25 X 0 1` (~5 s), X roteert 56 57 14 5 27 28 16 24; 0,99 verzoeken/s
    (gemeten 11:57–12:02; de eerdere 1,30/s kwam uit een verloren capture). Menuverzoeken (115,
    113–116 …) nemen de X-plek; de lijst schuift één stap op. ID0/1/17/25 blijven in elk vijftal.
  - Het snellere OT-log met de Anna is dus een thermostaateigenschap, geen firmwarewijziging.
  - OT 4.2 §5.2.1: master verplicht ID0, ID1, ID3 (bij opstarten); §4: master minstens elke 1 s
    (+15 %). Anna max 1,12 s; qSense mediaan 1,00 s, 5× > 1,15 s (max 1,93 s).

## 5. Concepten (niet actief)

- `docs/boiler-info/profiles-draft/`: Tzerra M, Tzerra Ace, Avanta (+ Ace), Calenta Ace(-Matic,
  40L), Quinta Ace 45–115/135/160, Quinta Pro, oude Quinta 25s–35c/45/65/85/115. Herkenning
  onbekend of niet uniek → staan niet in de tabel.
- `docs/boiler-info/manual-data/tzerra-ace-matic.json`: volledige instellingen/codes van de
  actieve Ace-Matic (niet op LittleFS; niet aan OpenTherm gekoppeld).
- `tools/boiler-manual-extract.py`: haalt tabellen uit Remeha-handleidingen (CU-GH-stijl);
  noteert tabeltitel en pagina. Oudere handleidingen (codes als afbeelding) handmatig.

## 6. Bouwen en testen

Op de eigen machine: `./build.sh` (of `build.bat`). Daarna firmware **en** LittleFS flashen via
de web-UI (`OTGW-firmware-<versie>.ino.bin` + `OTGW-firmware.<versie>.littlefs.bin`).

Gates uit `CLAUDE.md` vóór een push van firmwarewijzigingen:
```
python build.py --firmware
python evaluate.py --quick        # 17-10: 41 checks, 0 failed
```
Hosttests:
```
g++ -std=c++11 -Wall -Wextra -Werror -o /tmp/tbi test/host/test_boilerInfo.cpp && /tmp/tbi
g++ -std=c++11 -Wall -Wextra -Werror -o /tmp/tbr test/host/test_boilerInfoReplay.cpp && /tmp/tbr
node test/host/test_otBrandTextUi.js
```
Laatste build en flash (firmware + LittleFS via `/update`): `1.7.8-beta.2+0162c4a`, op het apparaat
gecontroleerd 18-10-2026. Screenshots in `OTGW-captures/shots/`.

**Cloud-sandbox zonder downloads.arduino.cc** (alleen nodig als daar opnieuw gebouwd wordt):
arduino-cli van GitHub-release; `network.proxy` instellen; core `esp8266:esp8266@2.7.4` via de
GitHub-index; bibliotheken met `lib install --git-url` op de versies uit `build.py`; Arduino-ctags
uit `github.com/arduino/ctags` bouwen (macro's `__unused__`/`__printf__` hernoemen, botsen met
glibc) en in `arduino/packages/builtin/tools/ctags/5.8-arduino11/` zetten; `build.py` aanroepen
met `--no-install-cli` en `install_dependencies` overgeslagen. `version.h`/`version.hash`
worden door de build gestempeld — niet meecommitten.

## 7. Git-afspraken

- Auteur: `STemplar <23719607+STemplar@users.noreply.github.com>` (GitHub weigert het echte
  e-mailadres door e-mailprivacy).
- Push naar `origin otgw-1.x.x` heeft de gebruiker toegestaan in deze sessie.
- **Openstaand**: `CLAUDE.md` eist een Backlog.md-taak per wijziging. De commits sinds
  `865f4246b` zijn **zonder** backlog-taak gemaakt. Vraag de gebruiker of er achteraf één
  taak (bijv. "Boiler info: modelprofielen en UI") moet komen met deze commits in de notities.

## 8. Open punten en volgende stappen

1. Afgerond 18-10-2026: TASK-1241 (uitleestempo), 1242 (indeling en tooltips), 1243 (Fault status:
   "Vergrendeling · E:xx" / "Blokkering · Su:xx" via `fault_status` in het profiel), 1244 (ID132),
   1245 (bedrijfsnamen member-ID). TASK-1246 / ADR-099 (taaluitgaven) **ingetrokken** door de
   gebruiker: geen en-GB-handleiding, Belgische modellen niet herkenbaar.
2. ID132 (servicemelding), **bevestigd 17-10-2026** met Remeha qSense (v12, soft 1.2), OT-log via
   WebSocket 81. De qSense leest ID131/132/133 bij het openen van het menu (`Read-Data 132, 255/255`).
   De ketel antwoordt `0x0101`, ook na uit/aan. Met de PIC-override `SR=132:<HB>,<LB>` (alleen het
   antwoord aan de thermostaat; de ketel bleef `0x0101` geven, `CR=132` na afloop) toonde de qSense:

   | ID132 | qSense |
   |---|---|
   | `0x0101` | Clear A |
   | `0x0102` | Clear B |
   | `0x0103` | Clear C |
   | `0x0104` | Clear (zonder letter) |
   | `0x0001`, `0x0000` | `----` (geen servicecode) |

   **Alleen `0x0101`–`0x0103` gelden als geldige waarden: servicemelding A, B, C** (besluit gebruiker).
   Alle andere waarden worden niet geïnterpreteerd en blijven ruw. "0/1 = A" uit 2017 klopt niet.
   Geen Clear gedaan (geen writes naar de ketel).
3. Ace-Matic: HB=status/LB=substatus van ID115 nog niet live bevestigd.
4. Tzerra 13/13 en 23/1 identificeren; Avanta Ace vs Calenta Ace onderscheiden
   (kandidaat: ID93–95-teksten bij OT 4.x, of een OEM-ID).
5. Oude Quinta's: welk data-ID hoort bij welke instelling/code — alleen via een echte uitlezing.
6. FHB (ID12/13) is ✖ bij de Tzerra M Plus; het blok wordt via `fault_history: false` verborgen.
7. Voorstellen die op de gebruiker wachten: ruwe ID5-code tonen als die ≠ 255 en ID0 bit 0 uit;
   bij vergrendeling met ID5 = 255 alleen "Vergrendeling"; tussenstap "BDR Thermea … · model not
   yet recognised" zolang ID131 ontbreekt; P21-controle tegen het herkende model.
8. Onderzoek: een echte blokkering (status 9) met ID115 op de bus; betekenis van ID5 bit 0; het
   data-ID waarmee de qSense het storingsgeheugen leest (capture met qSense in dat menu); dF/dU van
   24c/39c Plus; zustermodellen (TASK-1247).
9. Master member ID toont na een OTGW-herstart "---" tot de Anna haar ID2 opnieuw stuurt.

## 9. Commits in deze reeks (oud → nieuw)

`0c2c42d9d` basis (gebruiker) · `865f4246b` parameters v.10, 255 · `3d71fff45` 13/14 ·
`e5a25626c` dF/dU 36/1 · `67d827592` 28c-profiel · `657d8c0e6` member 2 eruit ·
`7d34b5a38` ID131 alleen type 13 · `c8c869d50` OT 4.2 §5.3.6 · `e15e6da4e` Ace-Matic ·
`156a2670d` matrixbewijs · `1bf8c693b` profieltabel · `666c52edd` generaties ·
`6078415a7`…`b0e5fcacd` concepten en bevestigde codes · `b9d06e29d` Device info-tabel ·
`d51c91f15` drie kolommen.
