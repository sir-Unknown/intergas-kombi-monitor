# IDS-protocol Kombi Kompakt HRE (branderautomaat IC2/IC3)

Primaire bron: de seriële module `cable_interface.pyo` en de modules `statistics.pyo` en `IDS.py`
(hoofdscript) uit de officiële **Intergas Diagnostic Software v1.83** (zie
[bronnen.md](./bronnen.md#b1-intergas-diagnostic-software-ids-v183)). `cable_interface.pyo` en
`rf_interface_ic3.pyo` zijn byte-identiek aan die in IDS 1.8 (intergasstore.co.uk); de module is
gecompileerd op 2015-04-10.

Alle byte-offsets en bitposities hieronder zijn **op bytecode-niveau** gecontroleerd (de
decompiler `uncompyle6` gaf constante lijsten verkeerd weer; offsets in subscripts bleken wel
correct). Waar een gegeven niet uit IDS komt, staat de bron erbij.

Notatie: bytes worden 0-geïndexeerd genoemd. "LE16" = 16-bit little-endian (lage byte eerst).
"s16/100" = signed 16-bit gedeeld door 100.

## 1. Fysieke laag

| Eigenschap | Waarde | Bron |
|---|---|---|
| Baudrate / framing | 9600, 8N1 | IDS `serial.Serial(comport, 9600)` (defaults 8N1) |
| Checksum | **Geen**, in geen enkel commando of antwoord | IDS `cable_interface` (geen CRC-functie aanwezig) |
| Antwoordlengte | 32 bytes (uitzondering: `B2` = 16 bytes) | IDS `Get_serialdata(..., returnlenght=32)` |
| Time-out | IDS pollt 3× per 0,1 s; na 0,3 s zonder 32 bytes: "no communication with burnercontrol" | IDS `Get_serialdata` |
| Buffer | Vóór elk commando leest IDS alle wachtende bytes weg | IDS `Get_serialdata` |
| 33 bytes ontvangen | IDS **gooit de eerste byte weg** en gebruikt de volgende 32 | IDS `Get_serialdata` (`if chars == returnlenght + 1: trash = s1.read(1)`) |
| Schrijfcommando's | Geen antwoord verwacht; IDS wacht 0,1 s | IDS `Get_serialdata(..., expect_return=0)` |

Ter vergelijking: de **X-lijn** (Xtreme, Xclusive; branderautomaat "ICX") gebruikt een XOR-checksum
over commando en antwoord (IDS X Range 1.05, `cable_interface_icx`). Dat is wat de Xtreme-firmware
in `backup/xtreme` implementeert.

## 2. Herkenning van de branderautomaat

IDS bepaalt het type uit het `REV`-antwoord (`statistics.pyo`):

- **IC3** als de hardware-release met `ic3` begint (voorbeelden: `ic3mk1   *P130213*`,
  `ic3mk2   *P131024*`); anders IC2 (voorbeeld `ic2kknl01*P100331*`).
- Softwareversie = tekst vanaf `V`: **4 tekens** (`V2.5`), of 5 tekens (`V1.84`, `V3.70`) als het
  getal boven 3 ligt of het antwoord `ic3` bevat. Een IC2 met `V2.53` telt in IDS dus als 2.5.
  IC2-versies zijn meestal 2.x/3.x en IC3-versies 1.x, maar ook een IC3 met `V2.30` komt voor
  (dump in IDS 1.84, [bronnen.md B1b](./bronnen.md#b1b-intergas-diagnostic-software-ids-v184)); het type volgt daarom altijd uit de hardware-release, niet uit het versienummer.

| Functie | Voorwaarde in IDS |
|---|---|
| Status `S?` + extra status `S2` | altijd (`S2` alleen gebruikt bij geldig `S?`) |
| Parameters `V?` | IC3, of IC2 met software ≥ 2.2 |
| Uitgebreide parameters | IC3 met software ≥ 1.31: `L`+0x1F+`?` (positie 31) en `V1\r` (32–63) |
| Bedrijfsuren `HN` | IC2: variant A; IC3 < 1.83: variant B; IC3 ≥ 1.83: variant C (zie §6) |
| Infoblok `B?` | IC3, of IC2 software > 2.5 |
| Infoblok 2 `B2` | IC3 met software ≥ 1.9 |
| `CRC`-blok | IC3: formules IC3; anders formules IC2 |

## 3. Commandoset

### 3.1 Lezen (veilig)

| Commando | Bytes (hex) | Antwoord | Inhoud |
|---|---|---|---|
| `S?\r` | 53 3F 0D | 32 | Status (§4) |
| `S2\r` | 53 32 0D | 32 | Extra status (§5) |
| `REV` | 52 45 56 | 32 | Versie: hardware-release tot `V`, software `Vx.xx`, ROM-checksums bytes 24–27 en 28–31 |
| `CRC` | 43 52 43 | 32 | Interne belasting/netfrequentie/referentiespanning (§9) |
| `HN\r` | 48 4E 0D | 32 | Bedrijfsuren en tellers (§6) |
| `EN\r` | 45 4E 0D | 32 | Storingshistorie (§7) |
| `V?\r` | 56 3F 0D | 32 | Parameters 0–31 (§8.1) |
| `V1\r` | 56 31 0D | 32 | Uitgebreide parameters 32–63 (IC3) (§8.2) |
| `L` + chr(n) + `?` | 4C nn 3F | **1** | Eén uitgebreide parameter (IC3: alleen n = 31 gebruikt; IC2: diverse posities) |
| `B?\r` | 42 3F 0D | 32 | Infoblok: cloud-/RF-adressen, RAMSES-id's, productiecode (bytes 25–31) |
| `B2\r` | 42 32 0D | **16** | Infoblok 2: dongle-/LAN-id's |

### 3.2 Schrijven / acties — **nooit vanuit automatisering sturen**

| Commando | Bytes | Effect volgens IDS | Opmerking |
|---|---|---|---|
| `SL\r` | 53 4C 0D | **Testprogramma: brander aan op minimaal WW-vermogen** | Testmodus 7 in IDS |
| `Sh\r` | 53 68 0D | **Testprogramma: brander aan op ingesteld max. CV-vermogen** | Testmodus 8 |
| `SH\r` | 53 48 0D | **Testprogramma: brander aan op maximaal vermogen** | Testmodus 9 |
| `SQ\r` | 53 51 0D | Testprogramma beëindigen | Testmodus 10 |
| `P` + chr(pos) + chr(waarde) | 50 pp ww | Parameter schrijven | Geen CR, geen checksum, **geen antwoord** |
| `I` + chr(pos) + chr(waarde) | 49 pp ww | Infoblok (RF-koppeling) wijzigen | |
| `W` + chr(n) + `\r` | 57 nn 0D | Storing uit historie wissen | |
| `N` + chr(n) + `\r` | 4E nn 0D | Statistiekteller wissen | |

Gevolg voor firmware: van de `S`-commando's uitsluitend `S?` en `S2` gebruiken. De
testprogramma's komen overeen met de handleiding §2.6 ("L", "h", "H").

## 4. Status `S?` (32 bytes)

| Bytes | IDS-veld | Decodering | Betekenis | Bron betekenis |
|---|---|---|---|---|
| 0–1 | `t1` | s16/100 | **S1 aanvoersensor** (ruwe sensorwaarde) | IDS expertmodus-label "S1"; marcgk (sensor losgetrokken → −31,81) |
| 2–3 | `temp_flow` (= `t6`) | s16/100 | **Aanvoertemperatuur zoals het display toont** | IDS-label "Temp. flow"/"T. aanvoer"; marcgk: berekend ≈ (S1 + 2·S2)/3 |
| 4–5 | `t2` | s16/100 | **S2 retoursensor** | IDS-label "S2"; marcgk (losgetrokken → −31,81) |
| 6–7 | `t3` | s16/100 | **S3 warmwatersensor** | IDS-label "S3" en in normale modus "T. warmw." |
| 8–9 | `t4` | s16/100 | **S4 boiler-/koudwatersensor** (optioneel, X4 9/10) | IDS-label "S4" / "T. boiler" |
| 10–11 | `t5` | s16/100 | **S5 buitenvoeler** (optioneel, X4 8/9) | IDS-label "S5" / "T. buiten"; IC3-dump: −8,71 = S2-veld outside_temp −8,7 |
| 12–13 | `ch_pressure` | s16/100 bar; IDS zet waarden > 3,7 op 0 | CV-waterdruk (sensor S7) | IDS |
| 14–15 | `temp_set` | s16/100 | Gevraagde aanvoertemperatuur ("T. max."); 0 als er geen warmtevraag is | IDS; snotneus (CO) |
| 16–17 | `fanspeed_set` | s16 (= s16/100 × 100) | Gewenst ventilatortoerental [rpm] | IDS |
| 18–19 | `fanspeed` | s16 | Werkelijk ventilatortoerental [rpm] | IDS |
| 20–21 | `fan_pwm` | s16/10 | Ventilator-PWM | IDS |
| 22–23 | `io_curr` | s16/100 | Ionisatiestroom [µA] | IDS |
| 24 | `displ_code` | u8 | Bedrijfscode (§10) | IDS |
| 25 | bits | zie onder | RF-thermostaat | IDS |
| 26 | bits | zie onder | Ingangen/uitgangen | IDS |
| 27 | storingsvlag | `== 128` → storing actief | | IDS |
| 28 | bits | zie onder | Brander/druk | IDS |
| 29 | `fault_code` | u8 | Actieve storingscode als byte 27 = 128; IDS toont dan `displ_code = 256 + code` | IDS |
| 30–31 | — | — | Niet gebruikt door IDS | — |

Controle op echte data (IC3-dump `memory_1404s01060`, software V1.84, zie bronnen):
S1 = 53,71, S2 = 29,00 → (53,71 + 2·29,00)/3 = 37,24; gemeten `temp_flow` = 37,15.

### 4.1 Bitvelden

| Byte | Bit | IDS-veld | NL-label in IDS | Bron |
|---|---|---|---|---|
| 25 | 0 | `rf_rth_bound` | RF-thermostaat gekoppeld | IDS |
| 25 | 1 | `rf_rth_communication` | RF-communicatie | IDS |
| 25 | 2 | `rf_rth_battery_info` | | IDS |
| 25 | 3 | `rf_rth_battery_ok` | | IDS |
| 26 | 0 | `gp_switch` | "Ext. comfort" | IDS |
| 26 | 1 | `tap_switch` | "Tap schakelaar" | IDS |
| 26 | 2 | `roomtherm` | "Aan/uit therm." | IDS |
| 26 | 3 | `pump` | "Pomp" | IDS |
| 26 | 4 | `dwk` | "CV klep" (driewegklep/afsluiter) | IDS |
| 26 | 5 | `alarm_status` | — | **Niet in IDS**; Leo van der Kallen (CO), rixvet |
| 26 | 6 | `ch_cascade_relay` | — | **Niet in IDS**; idem |
| 26 | 7 | `opentherm` | "Open Therm" | IDS |
| 28 | 0 | `gasvalve` | "Gasklep" | IDS |
| 28 | 1 | `spark` | "Vonk" | IDS |
| 28 | 2 | `io_signal` | "Ion. signaal" | IDS |
| 28 | 3 | `ch_ot_disabled` | — | **Niet in IDS**; Leo, rixvet |
| 28 | 4 | `low_water_pressure` | — | **Niet in IDS**; idem |
| 28 | 5 | `pressure_sensor` | "Druk sensor" (sensor aanwezig) | IDS |
| 28 | 6 | `burner_block` | — | **Niet in IDS**; idem |
| 28 | 7 | `grad_flag` | — | **Niet in IDS**; idem |

## 5. Extra status `S2` (32 bytes)

| Bytes | IDS-veld | Decodering | NL-label / betekenis |
|---|---|---|---|
| 0–1 | `bc_tapflow` / `flowmeter_tap` | s16/100 | "Tapdebiet" (eenheid niet in IDS; vermoedelijk l/min) |
| 2 | `pump_pwm` | (200 − u8) / 2 | Pomp-PWM [%] |
| 3 | `OT_master_member_id` | u8 | OpenTherm-master-id |
| 4–5 | `room_override_zone1` | s16/100 | |
| 6–7 | `room_set_zone1` | s16/100 | Kamersetpoint (van OT/RF-thermostaat) |
| 8–9 | `room_temp_zone1` | s16/100 | Kamertemperatuur (IC3-dump: 24,17) |
| 10–11 | `room_override_zone2` | s16/100 | |
| 12–13 | `room_set_zone2` | s16/100 | 327,67 (0x7FFF) = niet beschikbaar |
| 14–15 | `room_temp_zone2` | s16/100 | idem |
| 16–17 | `override_outside_temp` | s16/100 | Buitentemperatuur (label `outside_temp`) |
| 18 | `OT_therm_prod_version` | u8 | |
| 19 | `OT_therm_prod_type` | u8 | |

Deze indeling is gelijk aan die in `process_status_extra` van de Xtreme-code (backup), op `pump_pwm`
(byte 2) en de OT-id's na.

## 6. Bedrijfsuren en tellers `HN` (32 bytes)

Bytes 0–15: acht LE16-tellers (alle varianten):

| Bytes | Veld |
|---|---|
| 0–1 | Line power connected (uren) |
| 2–3 | Line power disconnected (aantal) |
| 4–5 | CV-functie (uren) |
| 6–7 | Tapwaterfunctie (uren) |
| 8–9 | Branderstarts (aantal) |
| 10–11 | Ontsteking mislukt (aantal) |
| 12–13 | Vlamverlies (aantal) |
| 14–15 | Resets (aantal) |

| Variant | Wanneer | Extra velden |
|---|---|---|
| A | IC2 | Geen. (snotneus, HRE 28/24 met IC2: laatste 10 bytes = 0) |
| B | IC3, software < 1.83 | Gasmeter CV = LE32 bytes 16–19 / 10000; gasmeter WW = LE32 bytes 20–23 / 10000; uren += byte 30 × 65536; branderstarts += byte 31 × 65536 |
| C | IC3, software ≥ 1.83 | Als B, plus watermeter = bytes 24, 25, 28 (LE24) / 10000; tapwater-branderstarts = bytes 26, 27, 29 (LE24) |

Variant C is identiek aan wat de Xtreme-code (backup) met `H0` doet (`process_runtime_stats`).
Eenheid gasmeter/watermeter staat niet in IDS (vermoedelijk m³).

## 7. Storingshistorie `EN` (32 bytes)

32 posities, één byte per storing. IDS toont per positie **byte + 1**, en 0 als de byte 0xFF is
(lege plek). Voorbeeld IC3-dump: posities met getoonde waarden 2, 12 en 5.

Of de opgeslagen waarde gelijk is aan de displaycode (en IDS er onterecht 1 bij optelt) of
code − 1 is, is niet vastgesteld; zie [onzekerheden.md](./onzekerheden.md#o5-storingshistorie-en-1).

## 8. Parameters

### 8.1 `V?` — posities 0–31 (signed bytes)

Namen uit de IDS-labeltabel (labels 75–103, 119 en de vaste tekst "Parameter F."). Kolommen
"dump" komen uit twee IC3-geheugendumps in de IDS 1.8-download: **D1** = `1309d14637`
(ic3mk1, V1.81) en **D2** = `1404s01060` (ic3mk2, V1.84). "Handl." = fabriekswaarde
HRE 28/24 A volgens het installatievoorschrift §7.2.

| Pos | IDS-naam | Handleiding | D1 | D2 | Handl. 28/24 A |
|---|---|---|---|---|---|
| 0 | HEATER_ON | toestel aan/uit | 1 | 1 | — |
| 1 | COMFORT_MODE | tapcomfort uit/eco/aan | 2 | 0 | — |
| 2 | CH_SET_MAX | max. CV-aanvoertemperatuur (30–90) | 80 | 80 | 80 |
| 3 | DHW_SET | tapwatertemperatuur (40–65) | 60 | 60 | 60 |
| 4 | ECO_DAYS | o. aantal ecodagen | 3 | 3 | 3 |
| 5 | COMFORT_SET | vermoedelijk n. warmhoudtemperatuur | 57 | 0 | 0 |
| 6 | DHW_AT_NIGHT | — | 1 | 0 | — |
| 7 | CH_AT_NIGHT | — | 2 | 0 | — |
| 8 | Parameter 1 | installatietype | 0 | 0 | 0 |
| 9 | Parameter 2 | CV-pomp continu | 0 | 0 | 0 |
| 10 | Parameter 3 | max. CV-vermogen | 70 | 70 | 70 |
| 11 | Parameter 4 | max. WW-vermogen | 100 | 100 | 99 |
| 12 | Parameter 5 | min. aanvoertemp. stooklijn | 25 | 25 | 25 |
| 13 | Parameter 6 | min. buitentemp. stooklijn | −7 | −7 | −7 |
| 14 | Parameter 7 | max. buitentemp. stooklijn | 25 | 25 | 25 |
| 15 | Parameter 8 | pompnadraaitijd CV | 1 | 1 | 1 |
| 16 | Parameter 9 | pompnadraaitijd boiler | 1 | 1 | 1 |
| 17 | Parameter A | stand driewegklep/afsluiter | 0 | 0 | 0 |
| 18 | Parameter b | booster | 1 | 0 | 1 |
| 19 | Parameter C | stappenmodulatie | 1 | 1 | 1 |
| 20 | Parameter c | min. toerental CV | 30 | 30 | 30 |
| 21 | Parameter d | min. toerental WW | 25 | 25 | 25 |
| 22 | Parameter E | min. aanvoertemp. OT/RF | 40 | 40 | 30 |
| 23 | Parameter E. | reactie OT/RF-thermostaat | 1 | 1 | 1 |
| 24 | Parameter F | starttoerental CV | 70 | 70 | 60 |
| 25 | Parameter h | max. ventilatortoerental (×100 rpm) | 47 | 45 | 47 |
| 26 | Parameter n | regeltemp. boilerbedrijf | 85 | 80 | 85 |
| 27 | Parameter o | wachttijd CV na WW | 0 | 0 | 0 |
| 28 | Parameter P | antipendeltijd | 5 | 5 | 5 |
| 29 | Parameter r | (UK-lijst: stooklijncoëfficiënt) | 15 | 0 | — |
| 30 | Parameter F. | starttoerental WW | 70 | 70 | 60 |
| 31 | — | geen label in IDS; zie `DHW_GRADIENT_CONTROL` in §8.2 | | | |

De dumps komen van onbekende toesteltypen; afwijkingen (E, F, n) passen bij andere modellen of
oudere fabriekswaarden (de UK-parameterlijst noemt E = 40).

### 8.2 Uitgebreide parameters (IC3) — positie 31 via `L`, 32–63 via `V1\r`

Uit de `ext_par_list` in `statistics.pyo` van IDS v1.83 (bytecode). Kolommen: positie, IDS-standaard,
min, max, naam, zichtbaar zonder "more advanced mode". D1 = waarde in dump D1 (positie 31–63).

| Pos | IDS-std | Min | Max | Naam | Standaard zichtbaar | D1 | Handleidingparameter |
|---|---|---|---|---|---|---|---|
| 31 | 0 | 0 | 1 | DHW_GRADIENT_CONTROL | nee | 0 | — |
| 32 | 85 | 70 | 90 | DHW_HOTWATER_LIMIT | nee | 85 | — |
| 33 | 5 | 5 | 50 | FANSPEED_IGNITION | nee | 25 | — |
| 34 | 100 | 50 | 250 | FAN_SLOPE_INCREASE | nee | 150 | — |
| 35 | 70 | 50 | 100 | FANSPEED_POSTPURGE | nee | 50 | — |
| 36 | 0 | 0 | 1 | HARDWARE_TYPE | nee | 1 | — |
| 37 | 0 | 0 | 1 | BETA_CH_FLOW | ja | 0 | — |
| 38 | 0 | 0 | 15 | Parameter O. | ja | 0 | O. wachttijd CV-vraag |
| 39 | 1 | 0 | 2 | CASCADE_REACTION | ja | 1 | — |
| 40 | 0 | 0 | 1 | FLAME_MODULATION | nee | 0 | — |
| 41 | 30 | 20 | 100 | FANSPEED_HEATUP | nee | 35 | — |
| 42 | 1 | 0 | 1 | FLAME_LEVEL | nee | 1 | — |
| 43 | 1 | 0 | 1 | GRADIENT_LOW_PRESSURE | nee | 1 | — |
| 44 | 65 | 50 | 100 | POSTPURGE_TIME | nee | 50 | — |
| 45 | 35 | 15 | 100 | PREPURGE_TIME | nee | 15 | — |
| 46 | 50 | 10 | 70 | ABS_FANSPEED_MAXIMUM | nee | 52 | — |
| 47 | 0 | 0 | 1 | DISPLAY_TYPE | nee | 0 | — |
| 48 | 80 | 30 | 90 | Parameter 5. | ja | 90 | 5. max. instelwaarde aanvoer (90) |
| 49 | 0 | 0 | 255 | GASMETER_CONST_LSB | nee | 79 | — |
| 50 | 0 | 0 | 255 | GASMETER_CONST_MSB | nee | 20 | — |
| 51 | 0 | 0 | 255 | Parameter c. | ja | 40 | c. min. pompcapaciteit (50; UK-lijst 40) |
| 52 | 0 | 0 | 255 | Parameter 3. | ja | 80 | 3. max. pompcapaciteit (80) |
| 53 | 0 | 0 | 255 | Parameter P. | ja | 36 | P. referentiewaarde tapwater (24/30/36) |
| 54 | 0 | 0 | 255 | BOILER_TYPE | nee | 255 | — |
| 55 | 0 | 0 | 255 | BOILER_SETPOINT | nee | 255 | — |
| 56 | 0 | 0 | 255 | BOOSTER_SUPPLY_OFFSET | nee | 255 | — |
| 57 | 0 | 0 | 255 | FLAME_MODULATION_H | nee | 255 | — |
| 58 | 0 | 0 | 255 | Parameter q | ja | 255 | q zomerstand |
| 59 | 0 | 0 | 255 | HIGH_POWER | nee | 255 | — |
| 60 | 0 | 0 | 2 | Parameter L | ja | 255 | L legionellapreventie |
| 61 | 0 | 0 | 255 | DHW_WAIT_MAX_TIME | nee | 255 | — (nieuw in IDS v1.83) |
| 62 | 0 | 0 | 255 | DHW_WAIT_FACTOR | nee | 255 | — (nieuw in IDS v1.83) |
| 63 | — | — | — | — | — | 255 | — |

255 = niet in gebruik in die firmwareversie. Parameter **J** (CLV-overdruk) uit de handleiding
staat in geen van beide lijsten; zie [onzekerheden.md](./onzekerheden.md#o7-parameter-j-en-nieuwere-parameters).

### 8.3 Schrijven

`P` + chr(positie) + chr(waarde), waarbij positie de index uit §8.1/§8.2 is (0–63). Negatieve
waarden als two's complement (bijv. −7 = 0xF9). Er komt geen antwoord; de enige verificatie is
opnieuw `V?`/`V1` lezen.

Praktijkbevestiging: BAS_esp (Circuitsonline, juni 2025) legde het IDS-verkeer vast en schreef
met `50 03 3C` de tapwatertemperatuur (positie 3) naar 60 °C op zijn HReco; _fgj_ meldt in juli
2025 dat schrijven bij hem ook werkt.

## 9. `CRC`-blok

| Bytes | IC2-formule | IC3-formule |
|---|---|---|
| 0–1 | interrupt_time = LE16 / 5 | LE16 / 8 |
| 2–3 | interrupt_load = LE16 / 6,25 | LE16 / interrupt_time-ruw × 100 |
| 4–5 | main_load = LE16 / 8 | LE16 / 8 |
| 6–7 | net_frequency = 2000 / LE16 | idem |
| 8–9 | voltage_reference = LE16 × **5** / 1024 | LE16 × **3,3** / 1024 |
| 24–27, 28–31 | ROM-checksums (hex) | idem |

De referentiespanning wijst erop dat de IC2-processor op 5 V werkt en de IC3-processor op 3,3 V
(**afgeleid**; zie [hardware.md](./hardware.md#signaalniveaus)).

## 10. Bedrijfscodes (byte 24 van `S?`)

Uit de bytecode van `DoDrawing` in `IDS.py` (v1.8 en v1.83 identiek): de lijst
`[85, 170, 204, 51, 240, 15, 153, 102, 0, 255, 24, 231, 126, 37, 205]` wordt index-gewijs
gekoppeld aan de statuslabels 45–57 plus twee afgeleide labels.

| Code | IDS-label (EN / NL) | Betekenis |
|---|---|---|
| 0 | opentherm / OpenTherm | Warmtevraag van OpenTherm-thermostaat |
| 15 | boiler_ext / boiler ext. | Externe boiler |
| 24 | frost / vorst | Vorstbeveiliging |
| 37 | ch rf / cv rf | CV-vraag van RF-thermostaat |
| 51 | dhw_int / tapwater int. | Interne tapwatervraag (warmhouden / tapcomfort) |
| 85 | Sensortest / sensortest | Zelftest |
| 102 | ch / cv | CV-bedrijf |
| 126 | standby / rust | Rust |
| 153 | postrun_boiler / nadraaien boiler | Nadraaien na boilerbedrijf |
| 170 | service / service | Servicestand / testprogramma |
| 204 | dhw / tapwater | Tapwaterbedrijf |
| 205 | dhw hreco / tapwater hreco | Tapwater (HReco) |
| 231 | postrun_ch / nadraaien cv | Nadraaien na CV |
| 240 | boiler_int / boiler int. | Interne boiler |
| 255 | heatup / buffer | Opwarmen |
| > 255 | Fault / Storing | Storing; code = waarde − 256 (alleen in IDS-weergave) |

Afwijkende community-interpretaties: rixvet en Meewis (51 = warm water, 204 = nadraaien);
Edwin Bontenbal (58 = sensortest). IDS is leidend.

## 11. Storingscodes

Teksten volgens het HRE-installatievoorschrift §8.2 en de UK-lijst "HRE Parameters v6" (iC3):

| Code | Omschrijving |
|---|---|
| 0 | Sensorfout na zelftest |
| 0.0 | Lage waterdruk (alleen UK-lijst; weergave op display) |
| 1 | Temperatuur te hoog |
| 2 | Verwisseling S1 en S2 |
| 4 | Geen vlamsignaal |
| 5 | Slecht vlamsignaal |
| 6 | Vlamdetectiefout |
| 8 | Ventilatortoerental niet juist |
| 10 | S1 open circuit |
| 11 | S1 kortsluiting |
| 12 | S1 temperatuursprong −10 °C binnen 1 s |
| 13 | S1 temperatuursprong +10 °C binnen 1 s |
| 14 | S1 geen temperatuurverandering na 2 min bedrijf |
| 20–24 | Idem voor S2 |
| 25 | Rookgassensor > 2 min gesloten (n.v.t. UK) |
| 26 | Rookgassensor > 24 h niet gesloten (n.v.t. UK) |
| 27 | Kortsluiting buitenvoeler |
| 29, 30 | Gasklep-relaisfout (handleiding: vlamdetectiefout) |

## 12. Wat het protocol níet levert

- Geen checksum → integriteit alleen via lengte (32 bytes) en plausibiliteit.
- Geen bevestiging op schrijven.
- Geen expliciete "storing actief" in de bitvelden behalve byte 27 = 128.
