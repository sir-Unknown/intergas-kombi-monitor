# Implementatieplan HRE-ondersteuning

Uitgangspunten:

- **HRE-specifiek en eenvoudig:** geen gedeelde code met de Xtreme. De Xtreme-firmware staat
  ongewijzigd in [backup/xtreme](../../backup/xtreme/esphome/).
- Eerst alleen lezen; schrijven pas na verificatie op de eigen ketel.
- Alles wat in [onzekerheden.md](./onzekerheden.md) open staat, wordt vóór het betreffende
  werkpakket geverifieerd.

Status (november 2026): fase 2 en 3 zijn gebouwd en compileren. De decoders zijn veld voor veld
vergeleken met de originele IDS v1.83-bytecode en de poll-lus is gesimuleerd tegen een nagebootste
ketel, zie [verificatie.md](./verificatie.md); **nog niet op een ketel geverifieerd**. De ketel van de gebruiker is van 2026 en dus nieuwer
dan elke bron ([O20](./onzekerheden.md#o20-ketel-uit-2026-nieuwer-dan-alle-bronnen)); de eerste
testronde bepaalt of hij hetzelfde protocol spreekt.

## Fase 0 — Hardware en eerste verbinding (gebruiker)

| Stap | Klaar wanneer |
|---|---|
| Adapter bouwen ([hardware.md](./hardware.md#voorbereiding-bij-het-solderen)) | Bankcontroles gehaald (GPIO16 ≥ 2,5 V, geen doorgang tussen beide zijden) |
| X5 meten ([hardware.md](./hardware.md#meten-op-de-ketel-vóór-aansluiten)) | GND, V+, TX, RX en TX-ruststand bekend; draadbrug op 3,3 V of 5 V gezet |
| [intergas-hre-probe.yaml](./intergas-hre-probe.yaml) | Per commando een antwoord van 32 bytes (16 bij `B2` als die later wordt toegevoegd) in de log |

## Verificatie op de ketel

Leg één volledige ronde van de testconfiguratie naast het display. Elk punt sluit een onzekerheid.

| # | Handeling | Te vergelijken | Sluit |
|---|---|---|---|
| V1 | `REV` lezen; ruwe bytes en lengte van elk antwoord bewaren (ketel van 2026, zie O20) | `ic3…`/`ic2…`, code van de vorm `*P131024*` (waarschijnlijk een datum, niet bevestigd), `Vx.xx`; alle antwoorden 32 bytes | O1, O20 |
| V2 | CV-bedrijf, servicetoets indrukken (werkelijke aanvoertemperatuur) | `S?` bytes 2–3 ≈ display; bytes 0–1 = S1 | O15 |
| V3 | Warm water tappen | byte 24 = 204; bytes 6–7 stijgen; `S2` bytes 0–1 tapdebiet; tijdens tappen +-toets (tapwaterhoeveelheid) | O6, O10 |
| V4 | Na tappen | byte 24 → nadraaien of rust; tapcomfort aan: periodiek 51 | O10 |
| V5 | Toestel uit: display toont druk | `S?` bytes 12–13 ≈ display; byte 28 bit 5 = 1 | — |
| V6 | Servicemenu (code 15) doorlopen, **verlaten zonder opslaan** | elke waarde = `V?`/`V1` op de positie uit [protocol.md §8](./protocol.md#8-parameters) | O7 |
| V7 | Tapcomfortknop uit → eco → aan | `V?` positie 1 | O7 |
| V8 | Laatste storing tonen (§8.1) | eerste gevulde `EN`-positie | O5 |
| V9 | Tellers | `HN` branderstarts loopt op na een brandercyclus | O1 (variant) |

## Fase 1 — Gedeelde transportlaag (vervallen)

Vervallen door het besluit geen code met de Xtreme te delen.

## Fase 2 — HRE lezen (gebouwd)

Twee bestanden:

| Bestand | Inhoud |
|---|---|
| [esphome/hre_ids.h](../../esphome/hre_ids.h) | Pure decoders zonder ESPHome-afhankelijkheden (te testen op een pc) voor alle leescommando's, plus versiedrempels, bedrijfs- en storingsteksten |
| [esphome/intergas-hre-monitor.yaml](../../esphome/intergas-hre-monitor.yaml) | UART, één poll-lambda (elke 100 ms: commando sturen of antwoord verwerken) en de entiteiten |

Gedrag zoals IDS: ontvangstbuffer legen vóór elk commando; 300 ms na het versturen het antwoord
lezen; precies de verwachte lengte, of één byte meer (dan vervalt de eerste byte); anders verwerpen.
`REV` wordt na een fout na 10 s herhaald. Alles behalve `S?`/`S2` wacht op een geldig `REV` en volgt
de IDS-versiedrempels ([protocol.md](./protocol.md)).

| Commando | Interval | Publiceert |
|---|---|---|
| `REV` | 3600 s (eerst) | hardware- en softwarerelease, ROM-checksums |
| `S?\r` | 5 s | S1, aanvoer (display), S2 retour, S3 warm water, S4, buiten, druk (alleen als bit 28.5), gevraagde aanvoer, ventilator gewenst/werkelijk, ventilator-PWM, ionisatiestroom, bedrijfstoestand (tekst), storing (vlag + tekst), alle IDS-bits uit byte 25/26/28 (incl. RF-thermostaat), verbinding |
| `S2\r` | 5 s | tapdebiet, pomp-PWM, kamersetpoint/-temperatuur/-override zone 1 en 2, buitentemperatuur-override, OpenTherm-thermostaatgegevens |
| `HN\r` | 60 s | alle tellers; gas- en watermeter volgens IC2/IC3/versie |
| `EN\r` | 600 s | storingstellers per code (tekst) |
| `V?\r` | 600 s | parameters 0-30, elk een alleen-lezen sensor |
| `L` + 0x1F + `?` | 600 s | parameter 31 |
| `V1\r` | 600 s | parameters 32-62, elk een alleen-lezen sensor |
| `CRC` | 3600 s | interrupttijd, belasting, netfrequentie, referentiespanning, checksums |
| `B?\r` | 3600 s | productiecode; RF-infoblok ruw (standaard uitgeschakeld) |
| `B2\r` | 3600 s | RF-infoblok 2 ruw (standaard uitgeschakeld) |

Dit zijn alle leescommando's die IDS v1.83 over de kabel stuurt; er wordt niets weggelaten.

**Klaar wanneer:** V1–V9 kloppen; geen lengtefouten in 24 uur logging.

## Fase 3 — HRE-teksten (gebouwd)

- Bedrijfscodes volgens [protocol.md §10](./protocol.md#10-bedrijfscodes-byte-24-van-s); onbekende
  codes als `Code <n>`.
- Storingsteksten volgens [protocol.md §11](./protocol.md#11-storingscodes).

## Fase 4 — Schrijven (pas na fase 2 op de eigen ketel)

| Toegestaan | Positie | Bereik (handleiding) | Voorwaarde |
|---|---|---|---|
| Max. CV-aanvoertemperatuur | 2 (CH_SET_MAX) | 30 – waarde van parameter 5. (positie 48) | V6 geslaagd |
| Tapwatertemperatuur | 3 (DHW_SET) | 40–65 | V6 geslaagd |
| Tapcomfortstand | 1 (COMFORT_MODE) | na V7 vastgesteld | V7 geslaagd |
| Ecodagen | 4 (ECO_DAYS) | 0–10 | V6 geslaagd |

**Nooit aanbieden:** posities 8–30 (installateursparameters, o.a. vermogen, toerentallen,
installatietype), alle uitgebreide parameters 31–63 (ventilator, voor-/naventilatie, hardware-,
boilertype, gasmeterconstante), en de commando's `SL`/`Sh`/`SH`/`SQ`/`W`/`N`/`I`.

Implementatie:

- Bericht `P` + positie + waarde (3 bytes, geen checksum, geen CR).
- Waarde vóór verzenden binnen het bereik klemmen; buiten bereik weigeren en loggen.
- Na elke schrijfactie `V?` opnieuw lezen; afwijking publiceren als fout.
- Geen herhaald schrijven vanuit Home Assistant-automatiseringen (parametergeheugen); de
  entiteiten alleen laten schrijven bij een handmatige wijziging.

**Klaar wanneer:** wijziging tapwatertemperatuur 60 → 59 → 60 wordt na elke stap door `V?` en het
display bevestigd.

## Fase 5 — Documentatie

- README: HRE als tweede ketelfamilie, met link naar deze map.
- GUIDE: X5-pinout en de spanningsmeting; verwijzing naar de HRE-testconfiguratie.

## Testconfiguratie

[intergas-hre-probe.yaml](./intergas-hre-probe.yaml) stuurt elke 60 s `S?`, `S2`, `V?`, `V1`,
`HN`, `EN`, `REV` (alleen leescommando's, geen `P`, geen test- of wiscommando's) en logt alle bytes
in beide richtingen. Gebruik dezelfde `secrets.yaml` als in de [README](../../README.md#sw-installation).

```
uvx esphome run docs/hre/intergas-hre-probe.yaml
uvx esphome logs docs/hre/intergas-hre-probe.yaml --device <IP van de ESP>
```
