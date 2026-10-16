# Conceptprofielen

Profielen uit handleidingen waarvan de ketelherkenning (ID3/ID127) nog niet bevestigd is.
Ze staan bewust **niet** in `src/OTGW-firmware/data/` en niet in `kBoilerModelProfiles`
(`BoilerProfiles.h`): ze kosten geen LittleFS-ruimte en worden nooit op een ketel toegepast.

| Bestand | Model | Handleiding | Herkenning |
|---|---|---|---|
| `tzerra-m.json` | Remeha Tzerra M 24c CW3 / 28c CW4 / 39c CW5 | 7600997-06 (110713) | ID127 onbekend |
| `tzerra-ace.json` | Remeha Tzerra Ace 24C / 28C / 39C | 7680248 v.11 (26-06-2025) | ID127 vermoedelijk 23/1, onbevestigd |

Een concept wordt pas actief na een bevestigde uitlezing van een echte ketel (ID3, ID127,
en waar van toepassing ID10/11 en ID131). Dan: bestand naar `data/` verplaatsen, het `id`
en de bestandsnaam in de tabel opnemen, en de hosttests draaien.

Bij de Tzerra Ace staan de instellingen onder `settings` (codes zoals CP010) en de
storingscodes onder `codes` (A/H/E). De handleiding koppelt geen van beide aan OpenTherm
(TSP-index of ID5); de UI gebruikt ze daarom niet.

Handleidingversies verschillen inhoudelijk (bij de M Plus: v05 met P22, v.10 zonder).
Elk profiel noemt daarom het documentnummer en de versie waaruit het komt.
