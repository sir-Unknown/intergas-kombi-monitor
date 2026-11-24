# Bronnen

Betrouwbaarheidsklassen:

- **Primair** — officiële Intergas-software; gedrag rechtstreeks uit de code.
- **Officieel document** — Intergas-handleiding of dealerdocument.
- **Community-code** — werkende code van derden, vaak op één ketel getest.
- **Forum** — waarneming of bewering van een gebruiker.
- **Meting** — eigen meting met een multimeter op één ketel.

## Primaire bronnen

### B1 Intergas Diagnostic Software (IDS) v1.83

- **Klasse:** primair. Officiële download van Intergas (bevestigd door de gebruiker: gedownload
  via de link van Intergas).
- **URL:** <https://www.intergas-verwarming.nl/app/uploads/2021/02/ids-v1.83.zip> (bereikt via een
  Intergas-maillink; redirect naar deze URL).
- **SHA-256 zip:** `e6f3006bbcc6da11c848112b153ae084d1d99c2d95b58e205b52a428a6df02a2`
- **Inhoud:** `IDS Win32 V1.83/IDS.exe` (SHA-256
  `9f76e7101b0e22618d24f5cc5d2033ccbedc13c5ad02366b69ac6bd149112dda`), een variant
  `BDS Win32 V1.83/IDS.exe` (andere hash, modules van gelijke grootte; niet verder vergeleken),
  USB-kabeldriver (FTDI CDM 2.04.06).
- **Techniek:** py2exe-bundel, Python 2.7 bytecode (magic 62211). Modules in de aangehangen zip;
  hoofdscript `IDS.py` in de PYTHONSCRIPT-resource (offset 2449540).
- **Gebruikte modules:**

| Module | SHA-256 | Gebruikt voor |
|---|---|---|
| `cable_interface.pyo` (gecompileerd 2015-04-10) | `6ae8d1ae43c8369f130eef7dc4aaace02d628df195f2a540cf41df003b53253a` | Commando's, byte-indeling, decodering, schrijfformaat, seriële timing |
| `statistics.pyo` | `ae20227dd9ebaa5fe389510b7f779143764513f5fc8e0b7fb5b1ea7d72092315` | IC2/IC3-detectie, versiedrempels, `HN`-variant, uitgebreide parameterlijst (positie, standaard, min, max, naam) |
| `IDS.py` (hoofdscript) | — (deel van `IDS.exe`) | Labeltabel (veldnamen NL/EN), koppeling bedrijfscode → label (`DoDrawing`), normale/expert-weergave temperaturen |

### B1b Intergas Diagnostic Software (IDS) v1.84

- **Klasse:** primair, maar herkomst niet officieel bevestigd: Google Drive-bestand
  (`1tVNWh7s8zMSbDaJpdOmLinMk5EoUVSJd`, de link van de "IDS 1.84 Software (20 MB .zip)" in de
  archiefversie van theintergasshop.co.uk, B7). De zip bevat `Intergas IDS Win32 V1.84/IDS.exe`
  en meldt zichzelf als 1.84.
- **SHA-256 zip:** `1a5de3e58a8e20acd4120009ff8e7baff1cfc3666aca4cd9e71a3da6375713e8`
- **SHA-256 IDS.exe:** `369959be1ff59d88d7b13969ee38371ec715366ec7ffebd125c444324da6984b`
- **Vergelijking met v1.83 (B1):** de seriële module `cable_interface.pyo`, `rf_interface_ic3.pyo`,
  `rf_interface.pyo` en `options2.pyo` zijn **byte-identiek**. Alleen `statistics.pyo` en het
  hoofdscript verschillen, en uitsluitend cosmetisch (decompilatie-diff): weergave van labels
  (spaties, kolombreedte), een tikfout `nmode` in het geheugendumpbestand hersteld, dumps in de
  map `memory_dumps`, en verschoven indexnummers in de vertaaltabel. Geen enkel commando,
  veldindeling of versiedrempel is anders; de analyse in [protocol.md](./protocol.md) geldt dus
  voor 1.83 en 1.84.
- **Voorbeelddump:** `memory_dumps/memory_1606e08200_04-04-2018 12-49.txt`, een echte IC3:
  `ic3mk1   *P160422*`, **software V2.30**, ROM-checksums `d5232adc` / `a15f5ea0`, 75 °C max. CV,
  parameter 3 = 99, watermeter 0. De decoder verwerkt dit antwoord correct (versie `V2.30`,
  IC3, alle IDS-drempels voldaan). IC3 komt dus ook met 2.x-versienummers voor. De differentiële test
  ([verificatie.md](./verificatie.md)) bevatte al IC3-versies van 0.99 tot 3.x.

### B2 IDS 1.8 (dealerdownload)

- **Klasse:** primair (oudere build); gebruikt ter vergelijking en voor de geheugendumps.
- **URL:** <https://img1.wsimg.com/blobby/go/b12ed59b-2083-4ff0-b97a-e6e57cbba677/downloads/IDS%201.8-20170509T111851Z-001.zip?ver=1772614120768>
  via <https://intergasstore.co.uk/downloads>
- **SHA-256 zip:** `0cf3c3a93cb575f3ee960ca7c3af5e32f3e7baf4dfae021f200bdd9729e938da`
- **SHA-256 `IDSprog.exe`:** `ec883ff1addafa4c6943c1f16f20990ea3d6c33b68e2fff8bba7c4bdaee86854`
- **Bevindingen:** `cable_interface.pyo` en `rf_interface_ic3.pyo` byte-identiek aan v1.83.
  `statistics.pyo` verschilt alleen in de uitgebreide parameterlijst (v1.83 voegt posities 61 en 62
  toe). Bedrijfscodelijst in `DoDrawing` identiek.
- **Extra inhoud:** twee IC3-geheugendumps, gebruikt als plausibiliteitscontrole:
  - **D1** `memory_1309d14637_17-06-2015 10-19.txt`: `ic3mk1 *P130213*`, V1.81
  - **D2** `memory_1404s01060_27-05-2015 17-21.txt`: `ic3mk2 *P131024*`, V1.84

  Verder een map `IDS Win32 V1.72 UK` en een testlog `datainput-il.txt` uit 2009.

### B3 IDS X Range 1.05 (ICX)

- **Klasse:** primair voor de X-lijn (Xtreme/Xclusive); **niet** voor de HRE.
- **URL:** <https://img1.wsimg.com/blobby/go/b12ed59b-2083-4ff0-b97a-e6e57cbba677/downloads/IDS_ICXbeta_softwar%201.05_2018.zip?ver=1772614120768>
- **SHA-256:** `18555a2024680d8e3dd0d017be85f38a66d69da6cc06ca3a5ff87700b21eff54`
- **Bevindingen:** `cable_interface_icx` voegt standaard een XOR-checksum toe (`REV` zonder).
  Commando's `S?`, `S2`, `D2`, `REV`, `REW`, `CRC`, `H<n>`, `E<n>`, `B?`, `B1`, `V<n>`, `Q<n>`
  (blok schrijven), `P`, `G0`/`G1`, `Z0`/`Z1`, `RDP`, `I<n>`, `W<n>`, `N<n>`, plus thermostaat- en
  buitentemperatuur-override. Dit is het protocol van de Xtreme-firmware in `backup/xtreme`.

## Officiële documenten

### B4 Installatievoorschrift Kombi Kompakt HRE 24/18 A – 36/48 A

- **URL:** <https://www.intergas-verwarming.nl/app/uploads/2018/01/88557809-Installatievoorschrift-Kombi-Kompakt-HRE.pdf>
- **SHA-256 (gedownload november 2026):** `65b66db2102911e96d40f1f21b0dc6939ace933e9d72e19f3d42ac7ac66d8bf9`; © 2025.
- **Gebruikt:**
  - §2.5 IDS
  - §2.6 testprogramma's (L/h/H)
  - §3 componenten (S1, S2, S3, S5 stromingssensor, S7 druksensor)
  - §5.3 openen branderautomaat, X4-aansluitingen
  - §7.1 bereiken CV 30–90 °C en tapwater 40–65 °C
  - §7.2 parametertabel met fabriekswaarden
  - §8.1 laatste storing tonen
  - §8.2 storingscodes
  - §10.2 X2 230 V, X4, **X5 "Computer interface"**
  - p. 3 typeplaatje

### B5 HRE Parameters version 6 (JDL, 14-02-2020), "Intergas boilers iC3 PCB"

- **Klasse:** dealerdocument (UK).
- **URL:** <https://img1.wsimg.com/blobby/go/b12ed59b-2083-4ff0-b97a-e6e57cbba677/downloads/HRE%20Parameters%20version%206%20jdl%2014.02.2020.pdf?ver=1772614120669>
- **SHA-256:** `dd1d8c37b58a0f15af84786102d846a8db81eb8de85194cb6a48c78d2bda3bc1`
- **Gebruikt:**
  - Parameterlijst iC3 met bereiken: 3., 5., c., L, n., O., o., o., P., q, r
  - Storingscodes 0, 0.0, 1, 2, 4, 5, 6, 8, 10–14, 20–27, 29, 30
- **Let op:** UK-standaardwaarden wijken af van de NL-handleiding (bijv. E = 40, 5. = 80).

### B6 Intergas IDS-productpagina

- **URL's:** <https://www.intergas-verwarming.nl/zakelijk/producten/ids/> (NL, laatst gewijzigd
  19-2-2018) en <https://www.intergas-verwarming.nl/en/trade/products/ids/> (EN, gewijzigd
  9-3-2026). Beide gelezen november 2026.
- **Gebruikt:** IDS is bedoeld om een IC2 en/of IC3 branderautomaat van een Kombi Kompakt HRE of
  HReco uit te lezen. Onder "Geschiktheid" staat dat de set is samengesteld voor ketels met een
  **IC3**: Kombi Kompakt HR(E) vanaf 1 januari 2013 en alle HReco. "Voor het uitlezen van de
  overige Kombi Kompakt HR(E) ketels" verwijst Intergas naar de **USB-uitleeskabel
  branderautomaat**, een ander hulpmiddel.
- **Lezing:** officieel is IDS dus voor IC3 (vanaf 2013). Dat een IC2 (`ic2kknl01*P100331*`, B13)
  toch antwoordt, volgt uit de forumbronnen, niet uit Intergas. Ketels van vóór 2013 vallen buiten
  de officiële ondersteuning, wat past bij het 52-bytes-antwoord van een HR uit 2006 (B17,
  [O19](./onzekerheden.md#o19-oudere-hr-ketels-met-een-ander-antwoordformaat)).

### B6b Intergas "End of support"

- **URL:** <https://www.intergas-verwarming.nl/consument/end-of-support/> (laatst bijgewerkt
  27-8-2026; gelezen november 2026).
- **Gebruikt:**
  - **HRE / HREco (IC3): geproduceerd 2008 – heden**, status ondersteund. Intergas noemt voor
    cv-ketels maar twee branderautomaat-families: IC3 (HRE/HREco) en ICX (Xtreme/Xclusive, 2017 –
    heden). Een nieuwere generatie wordt niet genoemd.
  - Updates van de ketel verlopen "via Intergas service monteur", dus er bestaan veldupdates van de
    branderautomaat-software en de softwareversie in een ketel hoeft niet bij de productiedatum te
    passen (vgl. de IC3 met `V2.30` in B1b).
  - De HRE heeft zelf geen internetverbinding; alleen indirect via de RF-gateway (zie `B?`/`B2`,
    het RF-infoblok in [protocol.md](./protocol.md)). Gateway V1 (2013–2018) en V2 (2017–2025) zijn
    verlopen; Gateway V3 en een LTE-gateway zijn van 2026.
- **Afwijking:** de tabel noemt de HRE als IC3 vanaf 2008, terwijl [B6](#b6-intergas-ids-productpagina)
  IC3 vanaf 2013 noemt en een HRE 28/24 uit 2010 een IC2 had (B13). De tabel is waarschijnlijk
  vereenvoudigd; het type volgt uit het `REV`-antwoord.

### B7 theintergasshop.co.uk — Intergas boiler manuals

- **URL:** <https://theintergasshop.co.uk/content/181-intergas-boiler-manuals> — **404** bij
  controle (november 2026).
- **Archief:** <http://web.archive.org/web/20260520075658/https://theintergasshop.co.uk/content/181-intergas-boiler-manuals>
  linkt naar "IDS 1.84 Software" en "IDS X Range Software" op Google Drive. Die downloads zijn niet
  gebruikt; dezelfde soort bestanden komen uit B1–B3.

## Community-code

| # | Bron | Ketel | Gebruikt | Overeenstemming met IDS |
|---|---|---|---|---|
| B8 | [keesma/Intergas-Central-Heating-Monitor-Homie](https://github.com/keesma/Intergas-Central-Heating-Monitor-Homie) (`src/homie_ch_intergas_reader.ino`) | Prestige CW6 | `S?\r` zonder checksum, 32 bytes, flow = bytes 2–3, t2 = 4–5 | Volledig |
| B9 | [rixvet/IntergasBoilerReader](https://github.com/rixvet/IntergasBoilerReader) (was rickvanderzwet) | Prestige CW6 | X5-pinoutplaatje; 9600 8N1; bitnamen bytes 26/28 inclusief niet-IDS-bits | Temperatuurlabels t1–t6 opeenvolgend op bytes 0–11 **wijken af** van IDS; bedrijfscodes 51/204 omgedraaid |
| B10 | [Meewis/ESP8266_MQTT_INTERGAS](https://github.com/Meewis/ESP8266_MQTT_INTERGAS) (`.ino`, `Command_Sxx`, `command_REV.txt`, `intergas-esp8266.pdf`) | Intergas, type niet vermeld (IC3: REV-bytes = `ic3mk1 *P190517*V2.53`) | Voorbeeldantwoorden zonder checksum; ruwe doorgifte; adapterschema (zie [hardware.md](./hardware.md#adapters-in-community-projecten)) | Ja. De ASCII-vertaling "ic2mk0" in zijn forumpost klopt niet met zijn eigen bytes |
| B11 | [So871/intergasHrE-esphome-reader](https://github.com/So871/intergasHrE-esphome-reader) | HRE 36/48 A / HReco 40 (IC3) | ESPHome-variant van deze repo voor HRE: `S?`, `S2`, `V?`, `HN`, `REV` zonder checksum | Grotendeels; publiceert enkele temperaturen onder Xtreme-namen |
| B12 | [EdwinBontenbal/InterGasGateway](https://github.com/EdwinBontenbal/InterGasGateway) | HRE (IC2) | Kabelschema X5 (pin 1 TX, 2 RX, 3 GND, 4 +5 V), adapter 3V3 + 4N25 + BC337 (zie [hardware.md](./hardware.md#adapters-in-community-projecten)), IC2-commandotabel, bedrijfscodes | Ja; sensortest 58 i.p.v. IDS 85; leest `io_curr` uit bytes 21/22 i.p.v. 23/22 |
| B15 | [RichieB2B/intergas-exporter](https://github.com/RichieB2B/intergas-exporter) (`intergas-exporter.py`, Prometheus) | Intergas met IC3-waarden (watermeter, gasmeters, `V?`-parameters) | Alleen leescommando's `S?\r`, `S2\r`, `CRC`, `HN\r`, `V?\r`, 9600 baud; antwoord alleen geaccepteerd bij precies 32 bytes; parameternamen `param_3`, `param_c` enz. (volgens de handleiding, niet de IDS-namen) | Ja; geen checksum, zelfde commando's en lengte |
| B16 | [keesma/Central-Heating-Intergas-Reader](https://github.com/keesma/Central-Heating-Intergas-Reader) | Prestige CW6 | Voorloper van B8 (ESP8266, MQTT, openHAB); 4N25, 220 Ω in / 1 kΩ uit, 9600 baud, vier polen Vcc/GND/Tx/Rx | Zelfde ketel en aanpak als B8; de README zegt dat B8 hem vervangt. Verder niet gebruikt |
| B18 | [Pluimvee/esphome-intergas](https://github.com/Pluimvee/esphome-intergas) (ESPHome-component) en de voorganger [Pluimvee/Intergas2MQTT](https://github.com/Pluimvee/Intergas2MQTT) (`Protocol.txt`, `WemosSerial.h`, `X5 connector.png`) | HRE 24/18 | `S?\r`, `S2\r`, `HN\r` zonder checksum, frame alleen bij precies 32 bytes; log met echte waarden (S1, S2, `temp_flow`, druk, toerental, tapdebiet); niet-aangesloten S4/S5 = −50,81; Wemos D1 mini | Bytes, schaal en `S2` wel. **Bitvolgorde bytes 26/28 omgekeerd** en byte-28-bits uit byte 26 gelezen ([protocol.md §4.1](./protocol.md#41-bitvelden)); code 0 = "heating/standby" (IDS: OpenTherm); `io_curr` × 1,361 als "power"; gasmeter ÷ 11619,27 i.p.v. ÷ 10000 |
| B19 | [DereshBounce/IntergasHREBoilerReader](https://github.com/DereshBounce/IntergasHREBoilerReader) (`intergas_hre.py`, fork van B9) | Kombi Kompakt HRE 36/30 | Python, 9600 baud; elke 2 s `S?\r` en `S2\r`, elke 60 s `HN\r`, elke 15 min `V?\r` en `V1\r`; antwoord alleen bij precies 32 bytes; weigert `P`-commando's (alleen lezen); FTDI 5 V zonder scheiding, Vcc niet aansluiten "or the boiler will trip" | `S?`-bytes, schaal, bitvolgorde 26/28 en `HN` variant C (inclusief bytes 30/31) wel. Bytes 0–1 heten "heat_exchanger_temp (not sure)" (IDS: S1), 8–9 "t5" (IDS: S4); `fan_pwm` als "watts?"; code 0 = "Central Heating active" (IDS: OpenTherm), 51 = "Hot water ramp down" (IDS: tapwater int.), 102 = anti-blokkeerrun (IDS: CV) |
| B20 | [jjwbruijn/intergasHrE-esphome-reader-ng](https://github.com/jjwbruijn/intergasHrE-esphome-reader-ng) (ESPHome-component, fork van B11) | HRE (niet gespecificeerd) | Alleen een aanpassing voor ESPHome 2026.9 (`disabled_by_default` via de entiteitsconfiguratie); protocolcode ongewijzigd | Als B11. `STATE_BLOCK_COUNT = 2`: alleen `HN` en `S?` worden afgewisseld, `S2` en `V?` nooit gevraagd (de eigen COMPATIBILITY.md noemt dit). Bytes 2–3 gepubliceerd als `temperature_t2` (IDS: `temp_flow`; S2 = 4–5); code 0 = "Central Heating"; uren zonder byte 30. Verder niet gebruikt |
| B21 | [eddgrant/lan2rf-gateway-stats](https://github.com/eddgrant/lan2rf-gateway-stats) (`StatusData.kt`, Kotlin/InfluxDB) | Via LAN2RF-gateway, niet X5 | Leest `/data.json` van de gateway (HTTP, geen X5); veldnamen en codes afgeleid van Domoticz `InComfort.cpp` | Bedrijfscodetabel gelijk aan IDS ([protocol.md §10](./protocol.md#10-bedrijfscodes-byte-24-van-s)) behalve 205 (HReco). Bevestigt §10 onafhankelijk; verder niet bruikbaar voor X5 |

## Fora

### B13 Circuitsonline — "PC aansluiting (RS 232?) op Intergas CV ketel HR 28/24"

<https://www.circuitsonline.net/forum/view/80667> — alle 12 pagina's gelezen (november 2026).

| Datum | Auteur | Inhoud (gebruikt in) |
|---|---|---|
| 9-1-2010 | NikNeem | 4-polige connector zoals ATX-voeding (hardware) |
| 6-2-2010 | melkman | 5 V-voeding op twee pinnen, data ook 5 V (hardware, oude HR) |
| 29-10-2010 | Martin-B | PIC18F4520, MAX232 (hardware, oude HR) |
| 11-3-2016 | Leo van der Kallen | `53 3F 0D` → 32 bytes, voorbeeldantwoord |
| 8-4-2016 | Leo van der Kallen | VB6-decodering met PE9SMS; bits 26/28 inclusief niet-IDS-bits (O4) |
| 1-10-2019 | marcgk | Kombi Kompakt HR 36/30 (2006): bytes 0–1 = S1, 4–5 = S2 (losgetrokken → −31,81); bytes 2–3 berekend ≈ (S1 + 2·S2)/3 en getoond door display/IDS |
| 23-10-2021 | Meewis | BC337 als emittervolger voor ESP8266-boot |
| 11-12-2021 | Meewis | **TX en RX in de Intergas-handleiding omgedraaid** t.o.v. connector |
| 23-12-2021 | snotneus | IC2 `REV`: `ic2kknl01*P100331*V3.70` |
| 2-1-2022 | Meewis | `REV`-antwoord (bytes = `ic3mk1`) |
| 4/7-1-2022 | snotneus | Werkt op HRE met FTDI TTL-232R-**5V**; `temp_set` 0 buiten bedrijf |
| 11-1-2022 | So871 | "na-draaien cv en ww stonden verkeerdom" in de oorspronkelijke code van Meewis |
| 12/13-1-2022 | EmielA | Oude Kompakt HR (Furimat): geen reactie op `S?`; 2400 7E2 |
| 27-1-2022 | Johnny321_nl | Watermeter = niet-aaneengesloten bytes incl. byte 28 |
| 28-1-2022 | snotneus | Eigen ketel: HRE 28/24 met IC2; `HN` laatste 10 bytes 0 (geen watermeter); So871 heeft IC3 |
| 23-2-2022 | rbohm | Xtreme FW 1.80 vereist checksum; IDS 1.83/1.84 komt op Xtreme niet verder dan `REV` |
| 4-8-2022 | rbohm | "HRE-serie praat een ander protocol … veel minder CRC checks, data op andere plekken"; ESP8266 rechtstreeks → blokkeert bij boot |
| 5-1-2023 | edwin4b | ATOM-RS232-KIT met geïnverteerde serial, voeding uit ketel, code van rbohm (ketel: vermoedelijk Xtreme) |
| 26-8-2023 | acfnews | HRE: Xtreme-software "niet compatible"; **ESP + opto's uit 5 V van de ketel → storing 2** |
| 10-6-2025 | BAS_esp | MAX3232 werkt niet, PL2303 wel; ESP rechtstreeks op X5 (3,3 V) |
| 13-6-2025 | BAS_esp | Schrijven vastgelegd uit IDS: `50 03 3C` = tapwatertemperatuur 60 °C |
| 24-7-2025 | _fgj_ | Schrijven werkt |
| 15-8-2025 | EB_74 | InterGasGateway-print (B12) |
| 7-12-2025 | BAS_esp | ESP gevoed uit "3,3 V van de X5-stekker" |

### B14 Tweakers — "UDP Data ontcijferen Intergas LAN2RF Gateway"

<https://gathering.tweakers.net/forum/list_messages/2056344>

| Datum | Auteur | Inhoud |
|---|---|---|
| 27-2-2021 | Blihi | Datablokken uit gedecompileerde IDS (status, extra status, versie, CRC, uren, fouten, parameters, infoblokken) |
| 23-11-2021 | RichieB | HRE-commando's `S?\r`, `S2\r`, `REV`, `CRC`, `HN\r`, `EN\r`, `V?\r`, `LX?`, `B?\r`, `B2\r`; X-lijn gebruikt ICX-software |
| 23-11-2021 | Blihi | Xtreme-sniff: `S?`+`a`, `S2`+`l`, `D2`+`{` (checksum), `REV`, `REW@`, `H0` … |
| 23-11-2021 | RichieB (beste antwoord) | `D2` alleen ICX; HRE gebruikt `HN` i.p.v. `H0`; bij HRE geeft `V?` alle instellingen uit de handleiding |

### B17 Tweakers — "Intergas CV data uitlezen"

<https://gathering.tweakers.net/forum/list_messages/2170490> — één bericht, geen reacties (gelezen november 2026).

| Datum | Auteur | Inhoud |
|---|---|---|
| 24-1-2023 | thys | Kombi Kompakt **HR 28/24 uit 2006**. Antwoord op `S?\r` (9600 8N1) is **52 bytes** in plaats van 32; begint met `3f 00`, 24 voorbeeldregels met 10 s interval. IDS kan de data niet ontcijferen. Verwacht set-temperatuur 65 en een modus die wisselt tussen 1 en 5. Geen oplossing in het topic. Noemt B13, B14, B15, B9 en B16. |

Gevolg: een ketel die op `S?` 52 bytes teruggeeft is geen HRE met IC2/IC3 en valt buiten het protocol van deze analyse (zie [O19](./onzekerheden.md#o19-oudere-hr-ketels-met-een-ander-antwoordformaat)). De firmware verwerpt zo'n antwoord met een lengtewaarschuwing.

## Metingen

### M1 X5 op een Kombi Kompakt HRE 28/24 A (bouwjaar 2026)

- **Klasse:** meting (november 2026), één ketel, multimeter DC V, ketel in wachtstand zonder
  warmtevraag.
- **Resultaat:** met de vergrendeling links zit GND linksonder; V+ (linksboven), TX (rechtsboven)
  en RX (rechtsonder) meten elk 3,3 V t.o.v. GND. TX en RX zijn op positie toegewezen, nog niet
  met communicatie bevestigd.
- **Gebruikt in:** [hardware.md](./hardware.md#pinout) (pinout, signaalniveaus, meten op de ketel),
  [images/pinout-X5-hre.svg](../../images/pinout-X5-hre.svg),
  [onzekerheden.md](./onzekerheden.md) O2 en O3.

## Datasheets

| Bron | Gebruikt |
|---|---|
| [Vishay 4N25](https://www.vishay.com/docs/83725/4n25.pdf) | CTR min. 20 % / typ. 50 % bij 10 mA; I<sub>F</sub> max. 60 mA |
| [Espressif ESP32 datasheet v5.3](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf), tabel 5-3 | V<sub>IH</sub> min. 0,75 × VDD |
| [TI LP2950-N](https://www.ti.com/product/LP2950-N) | 100 mA, ruststroom 75 µA, min. 1 µF uitgangscondensator |

## Repository

[README.md](../../README.md), [GUIDE.md](../../GUIDE.md), [images/pinout-X14.png](../../images/pinout-X14.png),
[images/schematic.png](../../images/schematic.png) en de Xtreme-code in
[backup/xtreme/esphome/](../../backup/xtreme/esphome/).

## Reproductie

De gedecompileerde IDS-code staat niet in deze repository (auteursrecht Intergas). Zo is de analyse
te herhalen:

1. Download B1 en controleer de SHA-256.
2. `IDS.exe` is een py2exe-bundel. De zip met modules is aangehangen aan het bestand; Python's
   `zipfile` opent die direct (`zipfile.ZipFile(io.BytesIO(exe_bytes))`). Pak `cable_interface.pyo`
   en `statistics.pyo` uit.
3. Decompileer voor een eerste lezing met
   `uvx --python 3.11 --from uncompyle6 uncompyle6 cable_interface.pyo`.
4. **Controleer constanten altijd op bytecode-niveau.** `uncompyle6` geeft constante lijsten soms
   verkeerd weer. Laad met `xdis.load.load_module` en lees `co_code` (Python 2.7: opcode ≥ 90 heeft
   een 2-byte argument; `LOAD_CONST` = 100, `BUILD_LIST` = 103) samen met `co_consts`. Zo zijn
   gecontroleerd:
   - `get_bc_status`, `get_bc_status_ic3_extra`, `get_bc_hours_ic3_v2`, `get_bc_faults` en
     `edit_bc_params`
   - `ext_par_list` in `statistics.__init__`
   - de 15-elementenlijst in `DoDrawing`
5. Het hoofdscript staat in de PYTHONSCRIPT-resource. Zoek de header met magic `0x78563412`,
   gevolgd door 3 × uint32 en een lege zipnaam. Laad de daaropvolgende marshal-data met
   `xdis.unmarshal.load_code(..., 62211)`.
6. De labeltabel (veldnamen, statusnamen) staat als gemarshalde strings in hetzelfde script. Elke
   label-ID ("23", "24", …) wordt gevolgd door vertalingen (EN, NL, DE, …).
