# Verschillen tussen de Xtreme-code (backup) en de HRE

Referentie: [backup/xtreme/esphome/IntergasXtremeMonitor.h](../../backup/xtreme/esphome/IntergasXtremeMonitor.h),
[backup/xtreme/esphome/IntergasXtremeMonitor.cpp](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp),
[backup/xtreme/esphome/intergas-xtreme-monitor.yaml](../../backup/xtreme/esphome/intergas-xtreme-monitor.yaml) en
[protocol.md](./protocol.md). De Xtreme-code volgt het protocol van **IDS X Range 1.05**
(`cable_interface_icx`): XOR-checksum, `H0`–`H7`, `E0`, `D2`, `REW`, `V0`–`V?`, `G`/`Z`/`Q`.

## 1. Transport

| Aspect | Xtreme-code | HRE | Gevolg op een HRE |
|---|---|---|---|
| Checksum op verzoek | `crc_needed = true` voor bijna alle commando's ([H:177-214](../../backup/xtreme/esphome/IntergasXtremeMonitor.h#L177)); `add_command_crc` voegt XOR-byte toe | geen | Na `S?\r` komt een extra byte 0x61 (`a`); effect onbekend |
| Checksum op antwoord | `verify_crc` behandelt de laatste byte als XOR ([CPP:455](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L455)) | geen; 32 databytes | Een geldig HRE-antwoord wordt vrijwel altijd als "CRC error" afgewezen; er wordt niets gepubliceerd (**afgeleid**: kans ≈ 255/256 per antwoord) |
| Antwoordlengte | accepteert ≥ 32 bytes en leest alles wat er staat ([CPP:438-452](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L438)) | exact 32; bij 33 eerste byte weggooien | Restbytes van een vorig antwoord worden meegeparsed |
| Buffer legen vóór verzoek | nee | IDS: ja | Zie hierboven |

## 2. Commando's

| Xtreme-code | Op HRE |
|---|---|
| `REV` (zonder checksum) | Bestaat; zelfde indeling (hardware-release, `Vx.xx`, ROM-checksums) |
| `REW` | Bestaat niet in IDS voor HRE (DSP-versie, alleen X-lijn) |
| `B?\r` | Bestaat (infoblok); productiecode bytes 25–31 |
| `H0\r`–`H7\r` | HRE gebruikt `HN\r`; `Hx` onbekend op HRE |
| `S?\r` | Bestaat; andere temperatuurindeling (§3) |
| `S2\r` | Bestaat; **zelfde indeling** als Xtreme-decoder |
| `D2\r` | Bestaat niet op HRE (Tweakers, RichieB: "D2 zit alleen in de ICX-versie") |
| `E0\r`, `G1\r`, `B1\r` | HRE: `EN\r`; `G`/`B1` alleen X-lijn |
| `V0\r`–`V?\r` (16 blokken) | HRE: alleen `V?\r` (0–31) en `V1\r` (32–63, IC3) |
| `CRC` | Bestaat; IC3-formules (§9 protocol) |

## 3. Statusdecodering `S?` ([CPP:718-782](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L718))

| Bytes | Xtreme-code publiceert als | HRE-betekenis |
|---|---|---|
| 0–1 | Heat Exchanger Temperature (S0) | S1 aanvoersensor (ruw) |
| 2–3 | Delivery Temperature (S1) | Aanvoertemperatuur zoals display (berekend) |
| 4–5 | — (uitgecommentarieerd, "Xtreme has no return temperature sensor", [CPP:727](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L727)) | **S2 retour** — ontbreekt |
| 6–7 | Hot water Temperature (S3) | S3 warm water ✓ |
| 8–9 | Cold Water Temperature (S7) | S4 boiler/koudwater (optioneel) |
| 10–11 | Flue gas Temperature (S5) | **S5 buitenvoeler** (HRE heeft geen rookgassensor) |
| 14–15 | temperature_setpoint ✓ | idem |
| 16–17 | **Outside Temperature (S6)** ([CPP:732](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L732)) | **Gewenst ventilatortoerental** (rpm) → "buitentemperatuur" van bijv. 48,50 °C |
| 18–19 | **Solar Boiler Outflow temperature** ([CPP:733](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L733)) | **Werkelijk ventilatortoerental** |
| 20–21 | — | Ventilator-PWM |
| 22–23 | Heater IO Current ✓ | idem |
| 24 | status: 51 = "Hot water delivery", 204 = "Hot water ramp down", 0/102 = "Central Heating active" ([CPP:756-760](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L756)) | 204 = **tapwater**, 51 = **tapwater int.**, 0 = OpenTherm-vraag, 102 = CV (§10 protocol) |
| 25 | — | RF-thermostaatbits |
| 26, 28 | bitvelden ✓ | IDS-bits identiek; bits 26.5/6 en 28.3/4/6/7 niet in IDS (zie onzekerheden) |
| 27 bit 7 / 29 | storing actief / storingscode ✓ | IDS test `byte 27 == 128` |

Op de Xtreme komt het ventilatortoerental uit `D2` ([CPP:796-806](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L796)); de HRE kent `D2` niet.

## 4. Storingsteksten ([CPP:1015-1044](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L1015))

| Code | Xtreme-tekst | HRE-betekenis |
|---|---|---|
| 2 | Temperature too high during DHW demand | Verwisseling S1 en S2 |
| 3 | Flue gas temperature too high | Niet in HRE-lijst |
| 7, 9, 15, 16, 18, 19, 28, 31 | diverse Xtreme-teksten | Niet in HRE-lijst |
| 10–14 | Sensor fault / Mounting fault | S1-fouten (open, kort, sprong −/+10 °C, geen verandering) |
| 20–24 | — | S2-fouten |
| 29 | Gas valve error | Gasklep-relaisfout / vlamdetectiefout |
| 30 | Sensor S3 fault | Gasklep-relaisfout / vlamdetectiefout |

## 5. Parameters en schrijfacties

Xtreme-code: `set_param(param, value)` → `P` + param + waarde + XOR
([CPP:255-258](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L255), [CPP:527-535](../../backup/xtreme/esphome/IntergasXtremeMonitor.cpp#L527)),
refresh via `V<param/32>`.

Op een HRE betekent dezelfde positie iets anders. Als de branderautomaat de extra XOR-byte negeert
en positie > 31 accepteert zoals IDS dat doet, schrijven Home Assistant-instellingen naar de
volgende HRE-parameters (**afgeleid**, niet getest — juist daarom niet flashen):

| Xtreme-entiteit (positie) | HRE-parameter op die positie | Risico |
|---|---|---|
| P001 Installation Type (1) | COMFORT_MODE | gering |
| P002 Display Option (2) | CH_SET_MAX | gering |
| P009 Boiler Input load (9) | Parameter 2 (pomp continu) | middel |
| P010 Maximum CH output (10) | Parameter 3 (max. CV-vermogen) | middel |
| P011 Minimum CH output (11) | Parameter 4 (max. WW-vermogen) | middel |
| P012 Fan speed CH ignition (12) | Parameter 5 | gering |
| P030 CH pump setting (30) | Parameter F. (starttoerental WW) | **hoog** (ontsteking) |
| P031–P036 pomp-/tijdinstellingen | DHW_GRADIENT_CONTROL, DHW_HOTWATER_LIMIT, FANSPEED_IGNITION, FAN_SLOPE_INCREASE, FANSPEED_POSTPURGE, HARDWARE_TYPE | **hoog** |
| P037–P040 | BETA_CH_FLOW, Parameter O., CASCADE_REACTION, FLAME_MODULATION | **hoog** |
| P050 Max CH flow temperature (50) | GASMETER_CONST_MSB | middel |
| P051 / P052 / P053 heatcurve | Parameter c. / 3. / P. | middel |
| P056 / P057 / P059 | BOOSTER_SUPPLY_OFFSET / FLAME_MODULATION_H / HIGH_POWER | **hoog** |
| P060 LT zone max | Parameter L (legionella) | gering |
| P070–P104, P255 | buiten 0–63; gedrag onbekend | onbekend |

## 6. Instelbereiken in Home Assistant (YAML)

| Entiteit | Huidig bereik | HRE (handleiding §7.1/§7.2) |
|---|---|---|
| P050 max. CV-aanvoer ([YAML:1137](../../backup/xtreme/esphome/intergas-xtreme-monitor.yaml#L1137)) | 10–90 | CH_SET_MAX 30–90 |
| P078 tapwatertemperatuur ([YAML:1333](../../backup/xtreme/esphome/intergas-xtreme-monitor.yaml#L1333)) | 10–65 | DHW_SET 40–65 |
| P011 min. CV-vermogen ([YAML:1025](../../backup/xtreme/esphome/intergas-xtreme-monitor.yaml#L1025)) | 0–100 | Parameter c 20–50 |
| P001 opties ([YAML:716](../../backup/xtreme/esphome/intergas-xtreme-monitor.yaml#L716)) | "Xtreme Combi" … | 0 Kombi, 1 Solo + boiler, 2 Tap, 3 Solo |
| P255 factory reset ([YAML:975](../../backup/xtreme/esphome/intergas-xtreme-monitor.yaml#L975)) | aanwezig | bestaat niet in HRE-IDS; weglaten |

## 7. Xtreme-only entiteiten die voor de HRE vervallen

LT/HT-zone, relais RE1/RE2/alarm (P090/P091/P097), X10-uitgang (P104), ext1/ext2-ingangen
(P100/P101), DSP-versie (`REW`), pomptoerental uit `D2`, kleppositie, "Solar Boiler Outflow".

## 8. Hergebruik

Besloten is de Xtreme-code **niet** te hergebruiken of te delen: de HRE-firmware is een aparte,
eenvoudige implementatie ([esphome/hre_ids.h](../../esphome/hre_ids.h),
[esphome/intergas-hre-monitor.yaml](../../esphome/intergas-hre-monitor.yaml)). Dit document dient
alleen nog als onderbouwing waarom de Xtreme-firmware niet op een HRE gebruikt mag worden.
