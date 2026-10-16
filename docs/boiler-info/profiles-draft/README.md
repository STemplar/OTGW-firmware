# Conceptprofielen

Profielen uit handleidingen waarvan de ketelherkenning (ID3/ID127) nog niet bevestigd is.
Ze staan bewust **niet** in `src/OTGW-firmware/data/` en niet in `kBoilerModelProfiles`
(`BoilerProfiles.h`): ze kosten geen LittleFS-ruimte en worden nooit op een ketel toegepast.

| Bestand | Model | Handleiding | Herkenning |
|---|---|---|---|
| `tzerra-m.json` | Remeha Tzerra M 24c CW3 / 28c CW4 / 39c CW5 | 7600997-06 (110713) | ID127 onbekend |
| `tzerra-ace.json` | Remeha Tzerra Ace 24C / 28C / 39C | 7680248 v.11 (26-06-2025) | ID127 vermoedelijk 23/1, onbevestigd |
| `avanta-ace.json` | Remeha Avanta Ace 24c / 28c / 35c (installateur) | 7745710-07 (12-04-2023) | matrix: member-ID 11, ID127 20/1, OT 3.00 |
| `avanta-ace-gebruiker.json` | Remeha Avanta Ace (gebruiker) | 7745712-06 (30-11-2022) | idem |
| `avanta-gebruiker.json` | Remeha Avanta 24c / 28c / 35c (gebruiker) | 121239 v.13 (24-07-2020) | matrix: member-ID 11, ID127 5/17, 5/20, 5/24, OT 2.20/3.00 |
| `calenta-ace-matic.json` | Remeha Calenta Ace-Matic 35ds / 30c/CCS | 7861541-05 (24-02-2025) | onbekend |
| `calenta-ace-matic-40l.json` | Remeha Calenta Ace-Matic 40L | 7900207-03 (05-11-2025) | onbekend |
| `calenta-ace-gebruiker.json` | Remeha Calenta Ace 25ds / 28c / 35ds / 40c (gebruiker) | 7685808 v.08 (25-08-2022) | matrix: member-ID 11, ID127 20/1 (OT 3.00) en 17/1 (OT 4.00) |
| `quinta-ace-45-115.json` | Remeha Quinta Ace 45 / 65 / 90 / 115 (CU-GH08) | 7684353 v.14 (21-03-2024) | onbekend |
| `quinta-ace-135-160.json` | Remeha Quinta Ace 135 / 160 (CU-GH06) | 7698979 v.10 (14-11-2025) | onbekend |
| `quinta-ace-160-hmi.json` | Remeha Quinta Ace 160, HMI S-Control (CU-GH06) | 7621955 v.10 (29-11-2018) | onbekend |
| `quinta-pro.json` | Remeha Quinta Pro 45 / 65 / 90 / 115 (dF/dU, P-nummers) | 121803 v.12 (07-11-2018) | onbekend |
| `quinta-25s-35c.json` | Remeha Quinta 25s / 30s / 28c / 35c (oude besturing) | versie AH (2009) | onbekend |
| `quinta-45-65.json` | Remeha Quinta 45 / 65 (oude besturing) | versie AE (2009) | onbekend |
| `quinta-85.json` | Remeha Quinta 85 (oude besturing) | versie AD (2009) | onbekend |
| `quinta-115.json` | Remeha Quinta 115 (oude besturing) | versie AJ (2009) | onbekend |

**ID3 in de matrix.** Avanta, Avanta Ace, Calenta Ace en Calenta melden member-ID 11
(Remeha). De vlagbyte is 0x41 (OT 4.2 §5.3.2: bit 0 DHW aanwezig, bit 6 remote water
filling niet beschikbaar) of 0x49 (plus bit 3 DHW-boilervat). Bit 3 hangt af van de
installatie, niet van het model, dus ID3 onderscheidt deze ketels niet. Eén Calenta Ace-log
(2024-10-25) toont ID3 0/0 en ID5 0/0; dat is geen Remeha-antwoord en wordt niet gebruikt.

**ID127 in de matrix (member-ID 11):**

| Vermelding | ID127 | OT |
|---|---|---|
| Remeha Avanta | 5/17, 5/20, 5/24 | 2.20, 3.00 |
| Remeha Avanta Ace | 20/1 | 3.00 |
| Remeha Calenta | 10/17, 10/18, 10/19, 10/20; één log 5/24 (2023-11-27) | 3.00 |
| Remeha Calenta Ace | 20/1, één log 17/1 | 3.00, 4.00 bij 17/1 |

**ID127 is niet uniek per model.** De Avanta Ace en de Calenta Ace melden allebei 20/1,
en 5/24 staat zowel onder Avanta als onder Calenta (mogelijk een verkeerd benoemde log,
mogelijk dezelfde besturing; uit de matrix niet te beslissen). Een profiel op ID127 alleen
zou ze verwisselen. Voor deze ketels is een extra kenmerk
nodig voordat een model-specifiek profiel actief kan worden, bijvoorbeeld het member-ID
(ID3) in combinatie met de merk-/typeteksten (ID93–95, alleen bij OT 4.x) of een
OEM-ID dat per model verschilt. Gedeelde betekenissen (zoals de CU-GH-statuslijst) zouden
wel per platform kunnen, als blijkt dat ze gelijk zijn.

Nieuwe handleidingen in dezelfde opbouw: `python3 tools/boiler-manual-extract.py
handleiding.pdf uit.json --id ... --name ...`. Het script legt bij elke regel de tabeltitel
en de pagina vast; controleer een pagina met het oog tegen de PDF.

Een concept wordt pas actief na een bevestigde uitlezing van een echte ketel (ID3, ID127,
en waar van toepassing ID10/11 en ID131). Dan: bestand naar `data/` verplaatsen, het `id`
en de bestandsnaam in de tabel opnemen, en de hosttests draaien.

Bij de Tzerra Ace en Avanta Ace staan de instellingen onder `settings` (codes zoals CP010) en de
storingscodes onder `codes` (A/H/E). De handleiding koppelt geen van beide aan OpenTherm
(TSP-index of ID5); de UI gebruikt ze daarom niet.

Handleidingversies verschillen inhoudelijk (bij de M Plus: v05 met P22, v.10 zonder).
Elk profiel noemt daarom het documentnummer en de versie waaruit het komt.

**Drie besturingsgeneraties.** (1) Oude Quinta (2009, Comfort Master): één bedrijfscode
0–9/b/H/L, blokkeringen bXX, parameters met één teken, ketelvariant via parameter P.
(2) Tzerra M/M Plus, Quinta Pro: status/substatus, Su:xx, E:xx, P-nummers, dF/dU.
(3) CU-GH (Ace-reeks): AP/CP/DP/GP/PP-codes, A/H/E.xx.xx, CN1/CN2. De oude Quinta's
en de Quinta Pro zijn handmatig van de pagina's overgenomen (codes zijn afbeeldingen);
waar een displayteken niet eenduidig is, staat dat in een `note`.
