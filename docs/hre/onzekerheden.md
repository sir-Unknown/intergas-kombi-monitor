# Onzekerheden

Per punt: wat niet vaststaat, waarom, de impact, en hoe het te verifiëren is. "Hoog/middel/laag"
is de impact op een veilige, correcte implementatie.

Wat **wel** vaststaat (uit de officiële IDS v1.83, op bytecode gecontroleerd): commando's, het
ontbreken van een checksum, byte-offsets en bitposities van `S?`/`S2`/`HN`/`EN`, de
parameterposities en -namen van `V?`/`V1`, het schrijfformaat `P`+positie+waarde, de koppeling
bedrijfscode → label.

## O1 Type branderautomaat en firmwareversie van de eigen ketel

- **Wat:** IC2 of IC3, en welke softwareversie.
- **Waarom:** het installatievoorschrift noemt het niet. Een HRE A is vrijwel zeker IC3 (IDS-
  productpagina: IC3 vanaf 1-1-2013), maar niet bevestigd. De seriële module in IDS v1.83 is in
  2015 gecompileerd; branderautomaten van na 2015 kunnen velden of parameters hebben die IDS niet
  kent.
- **Impact:** hoog — `HN`-variant, beschikbaarheid van `V1`, en de vraag of de IDS-indeling nog
  klopt.
- **Verifiëren:** `REV` uitlezen (`ic3…`/`ic2…` en `Vx.xx`); alle velden uit de testronde naast
  het display leggen ([implementatieplan.md](./implementatieplan.md#verificatie-op-de-ketel)).

## O2 Signaalniveau op X5

- **Wat:** 3,3 V of 5 V logica.
- **Waarom:** alleen indirect bekend (IDS-referentiespanning IC2 5 V / IC3 3,3 V; praktijkverslagen
  van een 5 V-FTDI op IC2 en een 3,3 V-ESP op een HReco). Een eventuele buffer op X5 is onbekend.
- **Impact:** hoog — verkeerd niveau geeft geen communicatie of, bij 5 V op een 3,3 V-ingang,
  mogelijk schade.
- **Verifiëren:** meten volgens [hardware.md](./hardware.md#meten-op-de-ketel-vóór-aansluiten).
- **Status (november 2026):** gemeten op één HRE 28/24 A uit 2026: TX, RX én V+ in rust 3,3 V.
  Het niveau is daarmee bekend voor die ketel; dat de ketel 3,3 V op RX als hoog leest, is nog
  niet met communicatie bevestigd. Andere (oudere) ketels kunnen 5 V hebben.

## O3 Pinout-oriëntatie

- **Wat:** welke fysieke pin TX/RX/V+/GND is op de eigen ketel.
- **Waarom:** drie bronnen zijn consistent, maar een plaatje kan gespiegeld of vanaf de andere zijde
  getekend zijn; Meewis meldt dat de Intergas-handleiding TX en RX andersom labelt.
- **Impact:** hoog (V+/GND verwisselen).
- **Verifiëren:** meten; de regel "V+ en GND in de kolom aan de vergrendelzijde, TX in de rij van
  V+" blijft geldig bij spiegeling.
- **Status (november 2026):** op de gemeten HRE 28/24 A zit GND linksonder met de vergrendeling
  links, zoals de regel voorspelt ([pinout](./hardware.md#pinout)). Omdat V+, TX en RX alle drie
  3,3 V meten, zijn TX en RX alleen op positie bepaald; de probe moet ze bevestigen.

## O4 Statusbits die IDS niet decodeert

- **Wat:** byte 26 bit 5 (`alarm_status`), bit 6 (`ch_cascade_relay`); byte 28 bit 3
  (`ch_ot_disabled`), bit 4 (`low_water_pressure`), bit 6 (`burner_block`), bit 7 (`grad_flag`).
- **Waarom:** deze namen komen uit de VB-code van Leo van der Kallen (Circuitsonline, 2016, met
  hulp van PE9SMS uit een oudere IDS) en rixvet. IDS v1.83 decodeert ze voor de HRE niet.
- **Impact:** middel — verkeerde binaire sensoren in Home Assistant.
- **Verifiëren:** bits loggen tijdens bekende situaties (lage druk, antipendeltijd, OpenTherm uit).

## O5 Storingshistorie en +1

- **Wat:** of een `EN`-byte de storingscode is of code − 1.
- **Waarom:** IDS toont `byte + 1` (0xFF → 0). De statusbyte 29 gebruikt IDS zonder +1. Onbekend of
  IDS hier corrigeert of een fout bevat.
- **Impact:** middel — storingshistorie zou één code verschoven kunnen zijn.
- **Verifiëren:** laatste storing tonen volgens handleiding §8.1 (toestel uit, reset-toets) en
  vergelijken met de eerste gevulde `EN`-positie.

## O6 Eenheden en schaal

- **Wat:** eenheid van `S2` tapdebiet (vermoedelijk l/min), gasmeters en watermeter (vermoedelijk
  m³), `fan_pwm`.
- **Waarom:** IDS toont alleen getallen met labels zonder eenheid.
- **Impact:** laag.
- **Verifiëren:** tappen met bekend debiet; gasmeterstand over een periode vergelijken met de
  gasmeter van de woning.
- **Waarneming:** Pluimvee ([B18](./bronnen.md#community-code)) deelt de `HN`-gasmeters door
  11619,27 in plaats van 10000, na ijking tegen de woningmeter. Dat is 0,86 × de IDS-waarde. Of
  daar ander gasverbruik (kookplaat) in meetelt, staat er niet bij. Zijn tapdebiet tijdens het
  tappen (1,7–5,7) past bij l/min.

## O7 Parameter J en nieuwere parameters

- **Wat:** waar parameter **J** (CLV-overdruk) uit de handleiding staat; betekenis van de waarden
  van `COMFORT_MODE` (positie 1), `COMFORT_SET` (positie 5, vermoedelijk n.), `DHW_AT_NIGHT` en
  `CH_AT_NIGHT` (posities 6–7); positie 31 van `V?` versus `DHW_GRADIENT_CONTROL` via `L`.
- **Waarom:** J staat in geen van de IDS-lijsten; de overige namen staan in IDS zonder
  waardetabel.
- **Impact:** middel voor schrijven, laag voor lezen.
- **Verifiëren:** waarden in het servicemenu (code 15) aflezen en **zonder opslaan** verlaten
  (handleiding §7.2); tapcomfortknop omzetten en positie 1 volgen.

## O8 Schrijven

- **Wat:** of de branderautomaat waarden buiten het bereik weigert; of een schrijfactie direct
  actief is; slijtage van het parametergeheugen; gedrag bij posities > 31.
- **Waarom:** `P` krijgt geen antwoord. Praktijkbevestiging is beperkt tot één gebruiker
  (BAS_esp, HReco, positie 3 tapwatertemperatuur) en een korte melding van _fgj_.
- **Impact:** hoog.
- **Verifiëren:** alleen na volledige leesverificatie; eerst een onschuldige gebruikersparameter
  (tapwatertemperatuur) met één stap wijzigen, terugzetten, en beide keren `V?` teruglezen.
  Bereiken in de firmware afdwingen uit handleiding §7.2.

## O9 Antwoorden van 33 bytes

- **Wat:** waarom IDS bij 33 bytes de eerste byte weggooit; waarom de voorbeeldantwoorden van
  Meewis op 0 eindigen.
- **Waarom:** IDS geeft geen toelichting; bij Meewis kan het een artefact van zijn conversie zijn.
- **Impact:** laag — implementeer hetzelfde als IDS en log het voorval.
- **Verifiëren:** ruwe UART-log van de testconfiguratie.

## O10 Bedrijfscodes buiten de IDS-lijst

- **Wat:** codes die de ketel kan sturen maar IDS niet benoemt (bijv. voor "LC" load control,
  zomerstand Su/So/Et, opwarmen wisselaar "vorst" vs. tapcomfort).
- **Waarom:** IDS v1.83 kent 15 codes; de handleiding beschrijft meer displaytoestanden.
- **Impact:** laag — onbekende code als getal publiceren.
- **Verifiëren:** byte 24 loggen tijdens die toestanden.

## O11 Storingscodes in byte 29

- **Wat:** of byte 29 dezelfde nummering gebruikt als het display (inclusief 0.0 lage druk).
- **Waarom:** niet direct uit IDS af te leiden; IDS toont alleen "Storing <n>".
- **Impact:** middel.
- **Verifiëren:** bij een eerstvolgende storing display en byte 29 vergelijken.

## O12 Stroombudget X5

- **Wat:** hoeveel stroom X5 mag leveren.
- **Waarom:** nergens gedocumenteerd. Eén melding van storing 2 bij voeden van ESP + opto's
  (< 100 mA). Daartegenover ontwierp Edwin Bontenbal ([B12](./bronnen.md#community-code)) zijn
  gateway om met jumpers J4/J5 een Wemos D1 mini uit X5 te voeden, op een IC2. Een meting of
  langetermijnervaring daarvan staat niet in zijn repo.
- **Impact:** middel — adapter trekt ≈ 10 mA; ESP blijft op eigen voeding.

## O13 Betrouwbaarheid van de decompilatie

- **Wat:** of de gereconstrueerde IDS-logica correct is.
- **Waarom:** `uncompyle6` gaf constante lijsten verkeerd weer (bijv. 97…106 i.p.v.
  85, 170, 204, …).
- **Maatregel:** alle gebruikte offsets, bitposities, de parameterlijst en de bedrijfscodelijst
  zijn opnieuw uit de ruwe bytecode (`LOAD_CONST`-volgorde, `co_consts`) gehaald. De labeltabel
  (veldnamen) is met een eenvoudige marshal-parser uit `IDS.exe` gelezen; de volgorde van labels is
  plausibel en consistent met de code, maar niet met een volledige unmarshal gecontroleerd.
- **Impact:** laag.

## O14 Herkomst van de geheugendumps

- **Wat:** welk toesteltype bij dumps D1 (`1309d14637`, ic3mk1 V1.81) en D2 (`1404s01060`,
  ic3mk2 V1.84) hoort.
- **Waarom:** de dumps zitten in de IDS 1.8-download van intergasstore.co.uk zonder toelichting.
  D1 heeft h = 47 (handleiding: 28/24 A); D2 h = 45 (niet in de huidige handleiding).
- **Impact:** laag — alleen gebruikt als plausibiliteitscontrole.

## O15 Berekening aanvoertemperatuur

- **Wat:** of bytes 2–3 altijd (S1 + 2·S2)/3 zijn.
- **Waarom:** hypothese van marcgk (Kombi Kompakt HR 36/30, 2006); klopt op dump D2 binnen 0,1 °C
  (37,24 berekend vs. 37,15), niet verder onderbouwd.
- **Impact:** laag — publiceer S1, S2 en de displaywaarde apart.

## O16 Gedrag van de branderautomaat op de Xtreme-checksumbyte

- **Wat:** wat een HRE doet met de extra XOR-byte die de Xtreme-firmware (backup) achter elk
  commando stuurt.
- **Impact:** alleen relevant als iemand de Xtreme-firmware op een HRE flasht; dat wordt
  afgeraden.

## O17 Bronnen die niet bereikbaar of niet bekeken zijn

- `theintergasshop.co.uk/content/181-intergas-boiler-manuals`: 404; alleen archiefversie
  (20-05-2026) bekeken; Google Drive-downloads daarvan niet gebruikt (zelfde soort bestanden via
  intergasstore.co.uk en intergas-verwarming.nl).
- ~~RichieB2B/intergas-exporter~~: inmiddels gelezen, zie [B15](./bronnen.md#community-code); bevestigt commando's, 9600 baud en antwoordlengte 32 zonder checksum.
- Domoticaforum-link (Circuitsonline p. 4, "er zijn er ook met RS232 signalen"): niet bekeken.

## O18 Grens voor "geen sensor"

- **Wat:** bij welke waarde een temperatuurveld een ontbrekende of open sensor betekent.
- **Waarom:** IDS zelf filtert niet. Waargenomen waarden voor afwezige of losgetrokken sensoren:
  −31,13 (Leo), −31,81 en −32,00 (marcgk), −34,81 (IDS-testlog), −50,81 (Pluimvee, HRE 24/18),
  −51,00 (Meewis).
  [esphome/hre_ids.h](../../esphome/hre_ids.h) publiceert daarom alles onder −30 °C en 0x7FFF als
  "onbekend".
- **Impact:** laag; een echte buitentemperatuur onder −30 °C zou als onbekend verschijnen.
- **Verifiëren:** waarde van niet-aangesloten S4/buitenvoeler in de log van de eigen ketel.

## O19 Oudere HR-ketels met een ander antwoordformaat

- **Wat:** een Kombi Kompakt HR 28/24 uit 2006 antwoordt op `S?\r` met 52 bytes ([B17](./bronnen.md#b17-tweakers--intergas-cv-data-uitlezen)). Een HR 36/30 uit 2006 (B13, marcgk) gaf wel 32 bytes met de bekende indeling voor S1/S2, en een Furimat-HR reageerde helemaal niet (2400 baud, 7E2). Welke ketels 32, 52 of geen antwoord geven is dus niet vastgesteld.
- **Impact:** laag voor de HRE zelf (IC2/IC3). Wel relevant voor wie deze firmware op een oudere HR probeert: het antwoord wordt verworpen en "Verbinding ketel" blijft uit.
- **Officieel:** Intergas ondersteunt IDS voor IC3-ketels vanaf 1-1-2013; oudere HR(E) lezen zij met een andere USB-uitleeskabel ([B6](./bronnen.md#b6-intergas-ids-productpagina)).
- **Verifiëren:** bij een ketel die geen geldig antwoord geeft, de probe-configuratie ([intergas-hre-probe.yaml](./intergas-hre-probe.yaml)) draaien en de ruwe lengte en bytes van `S?` bewaren; die vergelijken met de 52-byte-voorbeelden in B17.

## O20 Ketel uit 2026: nieuwer dan alle bronnen

- **Wat:** de HRE staat in 2026 nog in de prijslijst van Intergas
  ([Prijslijst 2026](https://www.intergas-verwarming.nl/app/uploads/2026/02/Prijslijst-Intergas-Verwarming-01-01-2026-tot-en-met-31-12-2026.pdf):
  Kombi Kompakt HRE 24/18, 28/24, 36/30 en 36/48), dus het product bestaat nog. Maar alle ketels in
  de bronnen zijn ouder: de `REV`-antwoorden en dumps dragen codes die op een datum lijken (niet bevestigd) uit 2013 (`*P131024*`),
  2016 (`*P160422*`) en 2019 (`*P190517*`). Er is geen enkel antwoord van een ketel uit 2026 bekend.
- **Bekend:** IDS 1.83 (2021) en 1.84 (2018) kennen alleen IC2 en IC3, en de IDS-pagina van Intergas
  (EN-versie gewijzigd maart 2026, NL-versie sinds 2018 ongewijzigd; zie
  [B6](./bronnen.md#b6-intergas-ids-productpagina)) noemt ook alleen IC2/IC3. Een gewijzigde
  pagina zegt niets over een nieuwe generatie. De [end-of-supportpagina van Intergas](./bronnen.md#b6b-intergas-end-of-support) (27-8-2026) noemt voor de
  HRE nog steeds alleen IC3 (productie 2008 – heden) en kent voor cv-ketels geen andere familie dan
  IC3 en ICX (Xtreme/Xclusive): een sterke aanwijzing dat een HRE uit 2026 nog een IC3 is. Updates
  gaan wel via de servicemonteur, dus een nieuwere IC3-software is mogelijk. Er is geen bevestiging dat een nieuwe productieserie nog dezelfde antwoorden
  geeft. [INFERENTIE: een IC3 met nieuwere software werkt, omdat alle versiedrempels open eindigen.]
- **Mogelijke afwijkingen:** nieuwe softwareversie met andere byte-indeling; nieuwe parameters of
  storingscodes (onbekende codes worden als "onbekend" getoond); een andere X5-aansluiting of
  signaalspanning (de metingen in [hardware.md](./hardware.md) gelden juist daarom vóór het
  aansluiten); een nieuwe hardware-release die niet met `ic3` begint (dan behandelt de firmware hem
  als IC2: minder commando's, geen foutmelding).
- **Impact:** hoog voor de eerste verbinding, daarna laag. De firmware is alleen-lezen en
  verwerpt elk antwoord dat niet 32 (of 33) bytes lang is.
- **Verifiëren:** de eerste testronde met [intergas-hre-probe.yaml](./intergas-hre-probe.yaml):
  bewaar het `REV`-antwoord (hardware-release met datumcode en softwareversie) en de lengte van
  elk antwoord. De firmware logt een verworpen antwoord nu ook als ruwe bytes (tot 64), zodat een
  afwijkend formaat te analyseren is.

