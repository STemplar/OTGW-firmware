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

**ID3 in de matrix.** Avanta, Avanta Ace, Calenta Ace en Calenta melden member-ID 11
(Remeha). De vlagbyte is 0x41 (OT 4.2 §5.3.2: bit 0 DHW aanwezig, bit 6 remote water
filling niet beschikbaar) of 0x49 (plus bit 3 DHW-boilervat). Bit 3 hangt af van de
installatie, niet van het model, dus ID3 onderscheidt deze ketels niet. Eén Calenta Ace-log
(2024-10-25) toont ID3 0/0 en ID5 0/0; dat is geen Remeha-antwoord en wordt niet gebruikt.

**ID127 is niet uniek per model.** De Avanta Ace en de Calenta Ace melden allebei 20/1.
Een profiel op ID127 alleen zou ze verwisselen. Voor deze ketels is een extra kenmerk
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
