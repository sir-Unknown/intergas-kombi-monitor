# Home Assistant Intergas Kombi Kompakt HRE monitor

Read-only monitor for an Intergas **Kombi Kompakt HRE** (24/18 A, 28/24 A, 36/30 A, 36/48 A) via
the IDS diagnostic interface (connector **X5**, "Computer interface"). An ESP32 is connected
through optocouplers to the burner control board and publishes the boiler data to Home Assistant
via ESPHome.

The firmware only reads. It never sends write commands or the burner test programs.

The protocol, hardware details, sources and open questions are documented in
[docs/hre](./docs/hre/README.md).

> The original firmware of this project was written for the Intergas **Xtreme**, which uses a
> different IDS protocol. It is kept unchanged in [backup/xtreme](./backup/xtreme/esphome/) and is
> **not** suitable for the HRE.

## What is published

Everything Intergas Diagnostic Software v1.83 reads over the cable, read only:

- Temperatures: flow sensor S1, flow temperature (as shown on the display), return S2, hot water
  S3, optional S4 and outside sensor, requested flow temperature, room setpoint/temperature/override
  from the room thermostat (zone 1 and 2), outside temperature override.
- CV water pressure, fan speed (set/actual), fan and pump PWM, ionisation current, tap flow.
- Operating state, active fault code, pump/gas valve/ignition/flame/three-way valve, OpenTherm and
  on/off thermostat inputs, RF thermostat status.
- Operating hours and counters, gas meters CV/DHW and water meter (IC3 burner control), fault
  counters per fault code.
- All 63 parameters (positions 0-62) as separate sensors with their IDS names, including the
  installer parameters of the manual.
- Burner control diagnostics: hardware/software release, checksums, production code, load, mains
  frequency.

The decoders have been compared field by field against the original IDS bytecode, see
[docs/hre/verificatie.md](./docs/hre/verificatie.md). They have not been tested on a boiler yet.

## Physical connection

The adapter is described step by step in [GUIDE.md](./GUIDE.md) (written for the Xtreme X14
connector; the adapter is the same). For the HRE read
[docs/hre/hardware.md](./docs/hre/hardware.md) first: it covers connector X5, how to identify the
pins and the signal voltage by measurement, and one preparation while soldering.

* Connector: Molex Mini-Fit Jr 2×2 housing with female terminals, the same type as on old ATX
  power supplies ([housing 39-01-2040](https://www.conrad.nl/p/molex-39012040-mini-fit-jr-receptacle-housing-dual-row-4-circuits-ul-94v-2-natural-2448286),
  [terminals 5556](https://www.conrad.nl/p/molex-39000078-100-pcs-mini-fit-female-crimp-terminal-tin-sn-over-copper-cu-plated-brass-16-awg-bag-2448254)).
* Optocouplers: [4N25](https://www.conrad.nl/p/lite-on-optocoupler-fototransistor-4n25-dip-6-transistor-dc-1127375).
* Controller: [NodeMCU-32S (ESP32)](https://docs.platformio.org/en/latest/boards/espressif32/nodemcu-32s.html)
  or any ESP32-WROOM-32 board with GPIO16/17.
* Power the ESP from its own USB supply, never from X5.

## Schematics

The adapter schematic is designed with KiCad.
<img src="./images/schematic.png">

## SW Installation

1. Install the ESPHome add-on in Home Assistant.
2. Open an SSH session to Home Assistant, then:
   ```
   $ cd config
   $ git clone https://github.com/sir-Unknown/intergas-kombi-monitor.git esphome/intergas-kombi
   $ echo '<<: !include intergas-kombi/esphome/intergas-hre-monitor.yaml' > esphome/intergas-hre-monitor.yaml
   ```
   The configuration includes `intergas-kombi/esphome/hre_ids.h`, so keep the clone folder name
   `intergas-kombi`.
3. Create `config/esphome/secrets.yaml` with:
   ```
   esphome_hassio_api_encryption_key: my-32-byte-base64-encryption-key
   esphome_hassio_ota_pw: my-very-secure-ota-pw
   esphome_wifi_network_ssid: my-wifi-access-point
   esphome_wifi_network_password: my-wifi-password
   esphome_wifi_ap_fallback_password: my-fallback-password
   ```
   The API key is explained [here](https://esphome.io/components/api.html); one can be generated
   with `openssl rand -base64 32`.
4. In the ESPHome dashboard, compile and install the firmware, then add the device in Home
   Assistant.

For a first link test without the full firmware, use
[docs/hre/intergas-hre-probe.yaml](./docs/hre/intergas-hre-probe.yaml); it logs the raw bytes of all
read commands.

## External references

All sources (Intergas Diagnostic Software v1.83, installation manual, forum threads and community
projects) are listed with what was taken from them in [docs/hre/bronnen.md](./docs/hre/bronnen.md).

## Final Note

Nothing in this project is supported by Intergas, and there is no guarantee that it works in your
environment. Mistakes while connecting the adapter may damage your boiler or void your warranty.
Use at your own risk.
