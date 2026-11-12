# Implementatieplan HRE-ondersteuning

Uitgangspunten:

- De Xtreme-ondersteuning blijft ongewijzigd werken.
- Eerst alleen lezen; schrijven pas na verificatie op de eigen ketel.
- Alles wat in [onzekerheden.md](./onzekerheden.md) open staat, wordt vóór het betreffende
  werkpakket geverifieerd.

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
| V1 | `REV` lezen | `ic3…`/`ic2…`, `Vx.xx` | O1 |
| V2 | CV-bedrijf, servicetoets indrukken (werkelijke aanvoertemperatuur) | `S?` bytes 2–3 ≈ display; bytes 0–1 = S1 | O15 |
| V3 | Warm water tappen | byte 24 = 204; bytes 6–7 stijgen; `S2` bytes 0–1 tapdebiet; tijdens tappen +-toets (tapwaterhoeveelheid) | O6, O10 |
| V4 | Na tappen | byte 24 → nadraaien of rust; tapcomfort aan: periodiek 51 | O10 |
| V5 | Toestel uit: display toont druk | `S?` bytes 12–13 ≈ display; byte 28 bit 5 = 1 | — |
| V6 | Servicemenu (code 15) doorlopen, **verlaten zonder opslaan** | elke waarde = `V?`/`V1` op de positie uit [protocol.md §8](./protocol.md#8-parameters) | O7 |
| V7 | Tapcomfortknop uit → eco → aan | `V?` positie 1 | O7 |
| V8 | Laatste storing tonen (§8.1) | eerste gevulde `EN`-positie | O5 |
| V9 | Tellers | `HN` branderstarts loopt op na een brandercyclus | O1 (variant) |

## Fase 1 — Gedeelde transportlaag

Doel: één implementatie van planner en UART-statusmachine voor beide ketelfamilies.

- Commando-eigenschappen per entry: checksum aan/uit, verwachte lengte (32/16/1).
- Vóór elk verzoek de ontvangstbuffer legen.
- Antwoord accepteren bij exact de verwachte lengte; bij verwacht + 1 de eerste byte weggooien
  (IDS-gedrag) en het voorval loggen; anders verwerpen.
- Xtreme-tabel ongewijzigd (checksum aan).

**Klaar wanneer:** Xtreme-gedrag ongewijzigd (zelfde commando's, zelfde bytes op de lijn, zelfde
publicaties); een gesimuleerd HRE-antwoord van 31/32/33/34 bytes wordt verworpen/verwerkt/
ingekort verwerkt/verworpen.

## Fase 2 — HRE lezen

Nieuwe configuratie `intergas-hre-monitor.yaml` met HRE-decoders:

| Commando | Interval | Publiceert |
|---|---|---|
| `S?\r` | 5 s | S1, aanvoer (display), S2 retour, S3 warm water, S4, S5 buiten (NaN bij −51,00/0x7FFF), druk (alleen als bit 28.5), temp_set, ventilator set/act (rpm), ventilator-PWM, ionisatiestroom, bedrijfscode als tekst, bits 26/28 (alleen IDS-bits), RF-bits 25, actieve storing |
| `S2\r` | 5 s | tapdebiet, pomp-PWM, kamersetpoint/-temperatuur zone 1, buitentemperatuur-override |
| `HN\r` | 60 s | tellers; variant op basis van `REV` |
| `EN\r` | 600 s | storingshistorie |
| `V?\r`, `V1\r` | 3600 s en na elke schrijfactie | parameters als alleen-lezen sensoren met HRE-namen |
| `REV`, `B?\r` | 3600 s | versie, productiecode |

Niet in deze fase: bits uit O4 (pas na verificatie), entiteiten uit
[verschillen-xtreme.md §7](./verschillen-xtreme.md#7-xtreme-only-entiteiten-die-voor-de-hre-vervallen).

**Klaar wanneer:** V1–V9 kloppen; geen "CRC error" of lengtefouten in 24 uur logging.

## Fase 3 — HRE-teksten

- Bedrijfscodes volgens [protocol.md §10](./protocol.md#10-bedrijfscodes-byte-24-van-s); onbekende
  codes als `Code: <n>`.
- Storingsteksten volgens [protocol.md §11](./protocol.md#11-storingscodes).

**Klaar wanneer:** geen Xtreme-teksten in de HRE-configuratie.

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
