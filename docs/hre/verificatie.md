# Verificatie van de HRE-firmware tegen IDS v1.83

Doel: aantonen dat [esphome/hre_ids.h](../../esphome/hre_ids.h) en
[esphome/intergas-hre-monitor.yaml](../../esphome/intergas-hre-monitor.yaml) hetzelfde doen als de
officiële Intergas Diagnostic Software v1.83 (`ids-v1.83.zip`, zie
[bronnen.md](./bronnen.md#primaire-bronnen); de SHA-256 is bij deze controle opnieuw vergeleken),
en dat er niets ontbreekt van wat IDS over de kabel uitleest.

Wat dit **niet** aantoont: dat een echte ketel zich gedraagt zoals IDS verwacht. Dat blijft de
controle V1–V9 uit [implementatieplan.md](./implementatieplan.md#verificatie-op-de-ketel).

## 1. Differentiële test van de decoders

Geen vergelijking met gedecompileerde tekst, maar met de **uitgevoerde originele bytecode**:

1. `cable_interface.pyo` uit `IDS.exe` (Python 2.7) wordt ongewijzigd geladen in Python 2.7.15.
2. De module `serial` is vervangen door een nagebootste poort die per commando vaste bytes
   teruggeeft en alles logt wat IDS schrijft.
3. Dezelfde bytes gaan door `hre_ids.h`, gecompileerd met `g++ -std=c++17 -Wall -Wextra -Werror`.
4. Elk veld van beide uitkomsten wordt vergeleken.

Testvectoren: 24.824 antwoorden. Willekeurige bytes, de echte voorbeeldantwoorden uit de bronnen
(onder meer die van Meewis en `REV`-antwoorden van IC2 en IC3) en grenswaarden (0x0000, 0x7FFF, 0x8000,
0xFFFF, -51,00 °C, 3,7 bar, deling door nul).

| Commando | IDS-functie | Velden | Vergelijkingen | Afwijkend |
|---|---|---|---|---|
| `S?` | `get_bc_status` | 26 | 109.538 | 0 |
| `S2` | `get_bc_status` | 12 | 50.556 | 0 |
| `REV` | `get_bc_version` + versiedrempels uit `statistics.pyo` | 5 | 400 | 0 |
| `CRC` | `get_bc_crc`, `get_bc_crc_ic3` | 12 | 34.750 | 0 |
| `HN` | `get_bc_hours`, `get_bc_hours_ic3`, `get_bc_hours_ic3_v2` | 6 groepen | 18.000 | 0 |
| `EN` | `get_bc_faults` | 1 (32 tellers) | 1.004 | 0 |
| `V?` | `get_bc_params` | 1 (31 posities) | 1.004 | 0 |
| `L` + 0x1F + `?` | `get_bc_ext_params` | 1 | 256 | 0 |
| `V1` | `get_bc_ext_params_ic3` | 1 (31 posities) | 1.004 | 0 |
| `B?` | `get_bc_infoblock` (productiecode) | 1 | 2.263 | 0 |
| **Totaal** | | | **218.775** | **0** |

`B2` (`get_bc_infoblock2`) is niet differentieel getest: de firmware publiceert die 16 bytes ruw.

### Gevonden en hersteld

| Fout | Gevolg | Herstel |
|---|---|---|
| Softwareversie: de firmware las alle cijfers ("V2.53"), IDS leest bij een IC2 onder V3 maar 4 tekens ("V2.5") | Bij IC2 V2.51–V2.59 zou de firmware `B?` sturen, IDS doet dat niet (drempel `> 2.5`) | `decode_version` volgt nu exact IDS: 4 tekens, 5 als het getal boven 3 ligt of het antwoord "ic3" bevat |

### Bewuste verschillen met IDS

| Situatie | IDS | Firmware | Reden |
|---|---|---|---|
| Temperatuur onder -30 °C (geen sensor) | toont de ruwe waarde (bijv. -51,00) | onbekend (NaN) | geen onzinwaarde in Home Assistant; zie [O18](./onzekerheden.md#o18-grens-voor-geen-sensor) |
| Druk zonder druksensorbit (byte 28 bit 5) | toont de waarde | onbekend | zonder sensor is de waarde betekenisloos |
| Druk boven 3,7 bar | toont 0 | de gemeten waarde | een te hoge druk verbergen is onveilig |
| `CRC` met deler 0 | Python-fout | onbekend | geen crash |

## 2. Simulatie van de poll-lus

De lambda uit `intergas-hre-monitor.yaml` wordt letterlijk uit de YAML gehaald en op de pc
gecompileerd met minimale vervangers voor de ESPHome-klassen, tegen een nagebootste ketel die na
een instelbare vertraging antwoordt. Per scenario 2 uur gesimuleerde tijd (foutscenario's 2 minuten).

| Scenario | Controle | Resultaat |
|---|---|---|
| IC3 V1.84, V1.90, V1.81, V1.20 | commando's per versie zoals IDS: `B2` vanaf 1.9, `V1`/`L` vanaf 1.31, watermeter vanaf 1.83 | ok |
| IC2 V3.70, V2.53, V2.10 | `B?` alleen boven 2.5, `V?` vanaf 2.2, geen `V1`/`B2`, referentiespanning 5 V-schaal | ok |
| Ketel antwoordt niet | alleen `REV` (elke 10 s) en `S?`/`S2`; "Verbinding ketel" uit; geen waarden | ok |
| 33 bytes (extra eerste byte) | eerste byte vervalt, waarden correct | ok |
| Antwoord na 250 ms | waarden correct | ok |
| Antwoord na 350 ms, 31 of 34 bytes | antwoord verworpen, waarschuwing, verbinding uit; andere commando's werken door | ok |
| Losse bytes op de lijn tussen twee commando's | buffer leeg vóór elk commando, geen fouten | ok |
| Ketel 1 minuut weg en terug | verbinding uit en weer aan | ok |

In **elk** scenario is gecontroleerd dat uitsluitend de elf leescommando's verstuurd worden
(`REV`, `S?`, `S2`, `HN`, `EN`, `V?`, `L\x1f?`, `V1`, `CRC`, `B?`, `B2`), dat `REV` het eerste
commando is en dat de ontvangstbuffer leeg is op het moment van versturen. In 2 uur: 1440× `S?`,
1440× `S2`, 120× `HN`, 12× `EN`/`V?`/`V1`, 2× `REV`/`CRC`/`B?`.

### Gevonden en hersteld

| Fout | Gevolg | Herstel |
|---|---|---|
| Interval liep vanaf het antwoord in plaats van vanaf het versturen | 1359 in plaats van 1440 statusverzoeken in 2 uur (5,3 s i.p.v. 5 s) | interval loopt vanaf het versturen |
| Variabele voor het gelezen byte niet geïnitialiseerd | compilerwaarschuwing, geen functioneel gevolg | geïnitialiseerd |

## 3. Volledigheid

Commando's in `cable_interface.pyo` (alle `string =`/`sendstring =` toewijzingen):

| Soort | Commando's | Firmware |
|---|---|---|
| Lezen | `S?`, `S2`, `REV`, `CRC`, `HN`, `EN`, `V?`, `L<pos>?`, `V1`, `B?`, `B2` | alle elf |
| Schrijven / testprogramma's | `P`, `I`, `W<n>`, `N<n>`, `SL`, `Sh`, `SH`, `SQ` | komen in de firmware niet voor |

Bij deze controle toegevoegd omdat IDS ze leest en de firmware ze nog niet publiceerde:

- alle 63 parameters (posities 0-62) als afzonderlijke alleen-lezen sensoren, in plaats van vier
  sensoren en de rest in de log;
- storingstellers per code uit `EN`;
- `CRC`: interrupttijd, interrupt- en hoofdbelasting, netfrequentie, referentiespanning, checksums;
- `B?`: productiecode en het RF-infoblok (ruw); `B2`: RF-infoblok 2 (ruw);
- `S2`: zone 2, kameroverride, buitentemperatuur-override, OpenTherm-thermostaatgegevens;
- `S?`: de vier RF-thermostaatbits uit byte 25;
- ROM-checksums uit `REV`.

De 15 bedrijfscodes en hun labels zijn rechtstreeks uit de bytecode van `DoDrawing` gelezen
(`LOAD_CONST`-reeks vóór `BUILD_LIST 15`), niet uit de decompiler-uitvoer; zie
[protocol.md §10](./protocol.md#10-bedrijfscodes-byte-24-van-s).

## 4. Wat open blijft

- Niets hiervan is op een ketel getest.
- De wachttijd van 300 ms is die van IDS. Een antwoord van 32 bytes duurt op 9600 baud 33 ms;
  EdwinBontenbal mat op een IC2 maximaal 100–150 ms per commando ([bronnen.md](./bronnen.md#community-code)).
- De storingsteksten komen uit "HRE Parameters v6" en de handleiding, niet uit IDS (IDS toont
  alleen het nummer).

## Reproductie

De testopstelling staat niet in de repository: ze heeft de IDS-bytecode nodig (auteursrecht
Intergas). Stappen: `ids-v1.83.zip` downloaden en de hash controleren, de `.pyo`-bestanden uit het
zip-archief achteraan `IDS.exe` halen, Python 2.7 installeren (bijv. conda-forge), een module
`serial` aanbieden met `Serial(port, baud, timeout=...)`, `write`, `read`, `inWaiting`,
`flushInput` en `close`, en de uitkomst van elke `get_bc_*`-functie vergelijken met de bijbehorende
decoder uit `hre_ids.h`.
