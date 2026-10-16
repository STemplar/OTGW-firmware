# Deelproject 4 — Boiler info in de ESP8266 OTGW-firmware

Uitbreiding in `STemplar/OTGW-firmware`, branch `otgw-1.x.x`, versie
`1.7.8-beta.2`. Dit is de ESP8266-netwerkfirmware bij de OpenTherm Gateway,
met de bestaande PubSubClient. Het is niet de BrinkOT-firmware van deelproject 2.

## Bediening en presentatie

De knop **Boiler info** staat direct links naast **Settings**. De bestaande
informatiebalk, themakeuze en andere pagina’s zijn behouden. De pagina bevat:

- Toestelidentificatie: merknaam, productversie en serienummer, elk maximaal 50 tekens.
- Afzonderlijke slave-, master-, ventilatie- en solar-member-ID’s, indien waargenomen;
  daarnaast de OpenTherm-versie van de slave en producttype/-versie.
- Bedrijfsstatus, substatus en storingsinformatie.
- OEM-informatie met dF/dU, servicemelding en SCU-informatie waar het modelprofiel past.
- Uitklapbare parameters en foutgeheugen.

De algemene UI is Engels. De modelbeschrijvingen zijn overgenomen uit de
Nederlandse qSense-lookup en bijbehorende Tzerra-documentatie. Er worden geen
zelfbedachte Engelse vertalingen gebruikt. Een geverifieerde Engelse brontabel
kan later als afzonderlijk profielbestand worden toegevoegd.

Status, substatus, parameterwaarden en de **hele** notatie `dF: 8 dU: 6`
worden in een licht hellende 7-segmentstijl weergegeven. De betekenis staat
ernaast in gewone tekst. Dit is HTML/CSS met toegankelijke tekstlabels; er zijn
geen nieuwe libraries, fonts of externe downloads voor de segmenten nodig.
sevenSeg.js en SegmentDisplay zijn niet toegevoegd.

Op smalle schermen staat het label boven de waarde. Identificatieteksten blijven
op één regel en hebben zo nodig een eigen horizontale scrollbalk. Een gedeeltelijke
tekst toont de ontvangen tekens en `·` voor ontbrekende posities, met **Reception
incomplete** en de aantallen. Zodra de tekst compleet is, vervalt die statusregel.
Zonder antwoord blijft **Support not yet determined** staan; alleen een werkelijk
waargenomen `UNKNOWN-DATAID` leidt tot **Not supported by this boiler**. De opgegeven
OpenTherm-versie bepaalt deze status niet.

## Uitlezen met PIC-firmware 6.8

De gebruiker heeft PIC-versie **6.8** bevestigd. Het officiële pakket
<https://otgw.tclcode.com/download/otgw-6.8.tgz> is gecontroleerd:
`gateway.asm`, `StringCharacter` (rond regel 3399), `SendPriorityMsg` (3464)
en `SetPrioMessage` (5026). Na `PM=93`, `PM=94` of `PM=95` loopt de PIC zelf de
tekenindexen af. De ESP verzint dus geen extra indexparameter bij PM.
Dit gedrag is anders dan in de eerder gecontroleerde PIC 5.8-broncode.

Eenmalig per ESP-start, na echte ketelcommunicatie en wanneer gatewaymodus,
streamingmodus en de seriële verbinding beschikbaar zijn, start de controller:

1. `PM=3`: slave-identificatie.
2. `PM=127`: producttype en productversie.
3. `PM=125`: OpenTherm-versie slave.
4. `PM=93`, `PM=94`, `PM=95`: telkens wachten op de volledige tekenreeks.
5. `PM=131`: dF/dU, alleen als het member-ID ID131 in de waargenomen
   leeslijst heeft (Remeha: member-ID 11). Nodig voor de modelkeuze.

**Refresh identification** start een nieuwe ronde op aanvraag. Het openen van de
pagina en de REST-poll om de vijf seconden starten geen nieuwe OT-verzoeken.
De buffers staan in RAM en worden niet in LittleFS opgeslagen. Tijdens een
verversing blijft de bestaande tekst zichtbaar totdat nieuwe tekens binnenkomen.

De overige uitleesknoppen zijn expliciete acties. Bedrijfsstatus gebruikt PM=5
en PM=115. Parameters gebruikt PM=10; foutgeheugen PM=12. De PIC vraagt de
bijbehorende indexen zelf op. Er wordt geen PM=0 verstuurd: het masterstatusdeel
van dat bericht bevat bediening. Ontvangen ID0 wordt uitsluitend passief gebruikt
om een actuele storingsindicatie te onderscheiden van een gerapporteerde code.

Er loopt maximaal één eigen uitleesactie tegelijk. Nieuwe PM-commando’s gaan
alleen naar een lege ESP-commandowachtrij, met minimaal twee seconden tussen
commando’s. De controller wacht op ketelantwoorden, niet alleen op de seriële
PIC-commandobevestiging. Hij gebruikt uitsluitend gedocumenteerde leescommando’s.
Hij verandert geen parameters, wist geen servicemelding of foutgeheugen, activeert
geen SCU-detectie en wijzigt geen gateway- of ketelinstellingen.

Een scalar heeft maximaal 15 seconden antwoordtijd, een tekst 90 seconden, een
TSP/FHB-reeks 300 seconden; de hele actie is begrensd op 360 seconden. Na een
ontbrekend scalar-/tekstantwoord gaat de actie binnen die grenzen verder met de
volgende aanvraag. Een time-out betekent nooit automatisch ‘niet ondersteund’.
Er zijn geen onbeperkte automatische herhaalpogingen. Een andere client die
tegelijk eigen PIC-commando’s verstuurt, kan de PIC-prioriteitsreeks onderbreken;
de controller kan de externe client niet vergrendelen en meldt dan ontbrekende
antwoorden of een time-out. Uitlezen is uitgeschakeld in simulatie- en monitormodus.

## Ketelherkenning en profielkeuze

Herkenning staat op één plek: de tabel `kBoilerModelProfiles` in `BoilerProfiles.h`.
Elke regel legt vast:

- `member`: slave member-ID (ID3 LB);
- `productType` en `versions`: ID127 HB en de toegestane LB-waarden;
- `oem131`: indien niet 0, de vereiste dF/dU-waarde in ID131;
- `id` en `file`: het profiel-ID en het profielbestand in `data/`.

Alleen echte ketel-READ-ACK's tellen (bron B, correcte pariteit, binnen 1,5 s na het
eigen verzoek). `BoilerInfo::profile()` geeft de eerste volledig passende regel; zonder
treffer is het apparaat generiek. ID131 wordt bij het opstarten alleen gelezen als een
passende regel `oem131` vraagt en ID131 voor dat member-ID is waargenomen.

De REST-respons bevat `profile` (het ID) en `profile_file`. De UI laadt dat bestand
één keer en gebruikt het alleen als het `id` erin overeenkomt; anders blijven alle
waarden ruw. De profielbestanden bevatten zelf geen herkenningsregels. Een hosttest
controleert dat elk bestand uit de tabel bestaat, hetzelfde `id` draagt en binnen de
LittleFS-naamlengte past.

Nieuw model: regel toevoegen in de tabel en het profielbestand in `data/` zetten.

### Remeha Tzerra-generaties (member-ID 11)

| Generatie | Model | ID127 | OT-versie | Profiel |
|---|---|---|---|---|
| 1 | Tzerra M | niet bekend | niet bekend | geen |
| 2 | Tzerra M Plus | 13/14, 13/16 | 3.00 | `tzerra-m-28c-plus.json`, alleen bij dF/dU 36/1 |
| 3 | Tzerra Ace | vermoedelijk 23/1 | 3.00 | geen |
| 4 | Tzerra Ace-Matic | 27/1 | 4.00 | `tzerra-ace-matic.json` |

De generatievolgorde is door de gebruiker opgegeven. ID127 23/1 staat in de matrix
als "Remeha Tzerra" (onder meer 2022-11-20, ID15 5/96, ID125 3.00, ID93–95 ✖). Of dat
de Tzerra M of de Tzerra Ace is, is niet bevestigd; het oplopende producttype
(13 → 23 → 27) wijst op de Ace, maar is geen bewijs. 23/1 blijft generiek tot een
handleiding en een bevestigde uitlezing beschikbaar zijn.

Conceptprofielen voor de Tzerra M (7600997-06) en de Tzerra Ace (7680248 v.11) staan
in `docs/boiler-info/profiles-draft/`. Ze worden niet geladen tot hun herkenning bevestigd is.

## Betekenissen en bewijsniveau

Het modelprofiel is specifiek voor de **Remeha Tzerra M 28c Plus**. Het vereist
slave member-ID **11**, ID127 producttype **13** (versie 14 of 16) én dF/dU
(ID131) **36/1**, de typecode van de typeplaat. Binnen de Tzerra-serie verschillen
fabriekswaarden en mogelijk parameters per uitvoering; een andere dF/dU (zoals
37/1, gezien bij 13/14 in de matrix) krijgt daarom het generieke beeld tot er een
eigen, bevestigd profiel is. Alleen member-ID 11 is onvoldoende om deze
modelbeschrijvingen op een willekeurige ketel toe te passen.

Het profiel **Remeha Tzerra Ace Matic** (`data/tzerra-ace-matic.json`) geldt voor
member-ID 11 met ID127 **27/1**. Het bevat alleen status (Tab.54) en substatus
(Tab.55) uit handleiding 7875186-01 voor ID115. Volgens OT 4.2 is ID115 een
OEM-specifieke code; HB=status/LB=substatus is overgenomen van Remeha en past bij
de matrixwaarde 798 (3/30), maar is nog niet live bevestigd. De Ace Matic geeft
UNKNOWN-DATAID op ID10–13, dus er zijn geen parameters of foutgeheugen. De
tweedelige CU-GH16-foutcodes (Axx.yy, Hxx.yy) zijn niet aan ID5 gekoppeld; ID5
blijft ruw. Er wordt geen ID131 gelezen: dat is bij de Ace Matic niet waargenomen.

**Bewijswaarde van de OTGW-ketelmatrix.** De logs zijn vaak korte, onvolledige
scans; sommige ID's verschijnen pas als de thermostaat ze opvraagt. Alleen een
ingevulde waarde (ondersteund) en ✖ (UNKNOWN-DATAID, niet ondersteund) zijn
bewijs. Een lege cel betekent 'niet waargenomen' en wordt nooit als
'niet ondersteund' behandeld. De firmware leest OEM-ID's alleen als ze ergens
daadwerkelijk zijn waargenomen.

Het profiel bevat uit handleiding 7618573 v.10: Tab.43 (status/substatus), Tab.45
(blokkeringscodes als substatus bij status 9), Tab.48 (storingscodes) en Tab.34
(parameters met bereik en de fabrieksinstelling uit de kolom 28c Plus). Wijkt een
uitgelezen parameter af van de fabrieksinstelling, dan toont de UI die ernaast.
De letterlijke teksten staan in `data/tzerra-m-28c-plus.json`, met bronvermelding.
Substatusbeschrijvingen worden met de combinatie status/substatus opgezocht:
substatus 0 heeft bijvoorbeeld niet bij iedere status dezelfde betekenis.

- ID131 heeft de bevestigde bytevolgorde HB=dF en LB=dU. Deze waarden krijgen
  geen extra hexadecimale presentatie. dF/dU staat op de typeplaat; 0x2401 =
  dF 36 / dU 1 = Tzerra M 28c Plus (typeplaat, ID127 13/16). Alleen bevestigde
  combinaties krijgen een typenaam (`models` in het profiel); 0x2501 (dF 37 / dU 1,
  gezien bij 13/14 in de matrix) blijft zonder typenaam.
- Met dF/dU legt de ketel zijn typeafhankelijke veiligheidsparameters vast. Een
  qSense kan die via OpenTherm opnieuw instellen. Deze firmware leest ID131
  uitsluitend en biedt die reset bewust niet aan.
- ID132 blijft een ruwe waarde. Het formaat u8/u8 en de koppeling van antwoorden
  aan A/B/C zijn niet voldoende bevestigd. Het bestaan van servicetypen A/B/C
  in de handleiding bewijst die bytekoppeling niet. Het bevestigde wiscommando
  Write-Data(1,255) wordt door deze uitbreiding niet aangeboden of uitgevoerd.
- ID133 heeft de opgegeven u8/u8-presentatie. Er wordt geen detectie-write gestart.
- TSP-index = P-nummer, bevestigd met een echte uitlezing van een Tzerra M 28c Plus.
  De labels komen uit handleiding 7618573 v.10, Tab.34 (P1–P5, P17–P21, P23–P44;
  P22 bestaat in deze serie niet). P27 en P30 zijn signed (241 = -15, 246 = -10);
  keuzeparameters tonen de handleidingtekst. Een ontvangen waarde wordt niet door
  een default vervangen.
- Volgens OT 4.2 §5.3.6 beantwoordt een slave een onbekende TSP-/FHB-index met
  DATA-INVALID; iedere READ-ACK-waarde, ook 255, is geldig. De uitleesronde telt
  DATA-INVALID als beantwoord, maar bewaart en toont er geen waarde voor.
- De Tzerra M 28c Plus wijkt hiervan af: die antwoordt READ-ACK met 255 voor indexen
  zonder parameter. Het profiel legt dat vast (`tsp_unsupported_value: 255`); alleen
  dan wordt 255 verborgen, behalve waar 255 een echte instelling is (signed -1, P39).
  Zonder profiel wordt 255 gewoon getoond.
- FHB-index en FHB-waarde worden bewaard. Een koppeling van FHB-waarden aan
  blokkeringscodes of E-codes wordt niet als bewezen gepresenteerd.
- Zonder toepasbare betekenis blijft de payload beschikbaar als **Hex / Binary**.

De OEM-allowlist en waarnemingen staan in `docs/boiler-info/oem-vendors.json`.
`BoilerProfiles.h` bevat uitsluitend de daar waargenomen **leesbare** OEM-ID’s.
Actief OEM-uitlezen begint pas nadat de slave-member-ID in die lijst is aangetroffen.
Write-only-waarnemingen autoriseren geen leesverzoek; vendor 8 is bijvoorbeeld
wel bekend maar heeft geen toegestane actieve OEM-read. Er wordt niet blind
128–255 gescand. De lijst is een verzameling waarnemingen per vendor, geen garantie
voor ieder model. Ontbrekende OpenTherm-versies in oude logs blijven onbekend.

## REST en MQTT

`GET /api/v2/boiler/info` geeft de ontvangen gegevens, ontvangstmaskers,
ondersteuning, profiel, uitleesstatus en TSP/FHB-indexwaarden terug. De route
veroorzaakt zelf geen OT-verkeer.

`POST /api/v2/boiler/read/{identification|operating|parameters|history|oem}` start
een uitleesactie, gebruikt de bestaande HTTP-authenticatie en antwoordt 202 bij
acceptatie. Een bezette controller, ongeschikte modus of niet passend OEM-profiel
levert 409 op. GET op deze actie levert 405 op.

De volledige identificatieteksten worden ook retained op de bestaande MQTT-namespace
gepubliceerd: `Brand_text`, `BrandVersion_text`, `BrandSerialNumber_text`.
Dat gebeurt na volledige ontvangst, na reconnect en iedere minuut. Per tick van
drie seconden wordt maximaal één tekst aangeboden. Onvolledige teksten worden
niet als volledige MQTT-tekst gepubliceerd. Bij expliciet niet ondersteund worden
oude retained teksten gewist. De bestaande HB/LB-topics blijven behouden.
Home Assistant Discovery is in deze wijziging niet uitgebreid.

## Gewijzigde software

| Bestand | Functie |
|---|---|
| `BoilerInfo.h` | Niet-blokkerende leescontroller, echte antwoorden, ondersteuning, TSP/FHB-opslag |
| `BoilerProfiles.h` | Toegestane OEM-reads per waargenomen slave-member-ID |
| `OTBrandText.h` | Drie begrensde tekstbuffers met request/response-koppeling en bitmap |
| `OTGW-Core.ino` | Ontvangst vóór bestaande framevertraging, REST-data, PM-wachtrij, MQTT-teksten |
| `OTGW-Core.h`, `OTGW-firmware.h` | Functiedeclaraties |
| `OTGW-firmware.ino` | Controller in de bestaande loop; periodieke MQTT-publicatie |
| `MQTTstuff.ino` | Teksten opnieuw aanbieden na reconnect |
| `restAPI.ino` | Boiler info GET en geauthenticeerde uitleesacties |
| `data/index.html`, `index.js`, `index_common.css` | Knop, pagina, responsieve segmentweergave en polling |
| `data/tzerra-m-28c-plus.json` | Modelgebonden bronbeschrijvingen |
| `version.h` | Samenhangende prereleaseverhoging naar beta.2 |

## Verificatie

Hosttests staan in `test/host/test_otBrandText.cpp`, `test_boilerInfo.cpp` en
`test_otBrandTextUi.js`. Zij controleren onder andere correlatie, parity,
vervangende R-verzoeken, ongewenste A-antwoorden, 50 tekens, ontbrekende posities,
indexreeksen, time-outs, UNKNOWN-DATAID, vendorselectie, contextafhankelijke
beschrijvingen, segmentnotatie, nulwaarden en veilige tekstweergave.

De bestaande `test_otBusLiveness.cpp` wordt als regressiecontrole uitgevoerd.
De browsercontrole gebruikt de echte pagina en JavaScript met **synthetische
REST-testgegevens**, op breed scherm en 360/320 pixels, in beide thema’s. Deze
gegevens zijn geen nieuwe metingen aan de ketel.

Een validatiebuild bouwt steeds **firmware en LittleFS samen** met de bestaande
projectconfiguratie: ESP8266 Arduino core 2.7.4, D1 mini, 160 MHz en 4M2M.
De tijdelijke buildmap heeft geen spaties om de bestaande include-padquoting in
build.py te vermijden. De builds wijzigen de geïnstalleerde firmware niet.

Nog uit te voeren op de echte OTGW met PIC 6.8 en jouw ketel: de volledige
opstartreeks, tekstlengtes en timing, TSP/FHB-traversal, OEM-antwoorden en ontvangst
naast het normale thermostaatverkeer. Ook de onzekere service- en FHB-koppelingen
moeten daar worden getoetst voordat ze een betekenis in het modelprofiel krijgen.
