// Decoders for the IDS interface (connector X5) of the Intergas Kombi Kompakt HRE.
//
// Protocol as implemented by Intergas Diagnostic Software v1.83 (cable_interface.pyo);
// see docs/hre/protocol.md. Every decoder here is compared against the original IDS bytecode,
// see docs/hre/verificatie.md. Plain C++, no ESPHome dependencies, so it can be tested on a PC.
#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace hre {

// All read commands IDS sends over the cable. HRE commands carry no checksum.
// Not listed on purpose: 'P' (write parameter), 'SL'/'Sh'/'SH'/'SQ' (burner test programs),
// 'W'/'N' (clear fault/statistics counters) and 'I' (write info block).
static const uint8_t CMD_STATUS[] = {'S', '?', '\r'};
static const uint8_t CMD_STATUS2[] = {'S', '2', '\r'};
static const uint8_t CMD_VERSION[] = {'R', 'E', 'V'};
static const uint8_t CMD_INTERNALS[] = {'C', 'R', 'C'};
static const uint8_t CMD_HOURS[] = {'H', 'N', '\r'};
static const uint8_t CMD_FAULTS[] = {'E', 'N', '\r'};
static const uint8_t CMD_PARAMS[] = {'V', '?', '\r'};
static const uint8_t CMD_PARAM_31[] = {'L', 31, '?'};  // 1-byte reply
static const uint8_t CMD_PARAMS_EXT[] = {'V', '1', '\r'};
static const uint8_t CMD_INFO[] = {'B', '?', '\r'};
static const uint8_t CMD_INFO2[] = {'B', '2', '\r'};  // 16-byte reply

static const size_t REPLY_LEN = 32;
static const size_t MAX_REPLY_LEN = 32;

inline int16_t s16(const uint8_t *b, size_t i) { return (int16_t) (b[i] | (b[i + 1] << 8)); }
inline uint16_t u16(const uint8_t *b, size_t i) { return (uint16_t) (b[i] | (b[i + 1] << 8)); }
inline uint32_t u32(const uint8_t *b, size_t i) {
  return (uint32_t) b[i] | ((uint32_t) b[i + 1] << 8) | ((uint32_t) b[i + 2] << 16) | ((uint32_t) b[i + 3] << 24);
}
inline bool bit(uint8_t v, int n) { return (v >> n) & 1; }

// Temperatures are signed 1/100 °C. IDS shows the raw value; here 0x7FFF ("not available")
// and anything below -30 °C (absent or open sensor: IDS logs show -31.77 .. -51.00) become NAN.
inline float temperature(const uint8_t *b, size_t i) {
  int16_t raw = s16(b, i);
  if (raw == INT16_MAX || raw < -3000)
    return NAN;
  return raw / 100.0f;
}

inline std::string hex(const uint8_t *b, size_t n) {
  std::string out;
  char tmp[3];
  for (size_t i = 0; i < n; i++) {
    snprintf(tmp, sizeof(tmp), "%02x", b[i]);
    out += tmp;
  }
  return out;
}

// ---- "S?\r": status --------------------------------------------------------------------------
struct Status {
  float s1_flow_sensor;      // bytes 0-1, sensor S1 (IDS t1)
  float flow;                // bytes 2-3, flow temperature as shown on the display (IDS t6)
  float s2_return;           // bytes 4-5, sensor S2 (IDS t2)
  float s3_dhw;              // bytes 6-7, sensor S3 (IDS t3)
  float s4_boiler;           // bytes 8-9, optional sensor S4 (IDS t4)
  float s5_outside;          // bytes 10-11, optional outside sensor (IDS t5)
  float pressure;            // bytes 12-13, bar; NAN without pressure sensor
  float setpoint;            // bytes 14-15, requested flow temperature
  float fan_speed_set;       // bytes 16-17, rpm
  float fan_speed;           // bytes 18-19, rpm
  float fan_pwm;             // bytes 20-21
  float ionisation_current;  // bytes 22-23, µA
  uint8_t code;              // byte 24, operating code
  bool rf_bound, rf_communication, rf_battery_info, rf_battery_ok;         // byte 25
  bool opentherm, roomtherm, tap_switch, gp_switch, pump, three_way_valve;  // byte 26
  bool fault_active;         // byte 27 == 128
  bool gas_valve, spark, ionisation, pressure_sensor;                      // byte 28
  uint8_t fault_code;        // byte 29, valid when fault_active
};

inline Status decode_status(const uint8_t *b) {
  Status s;
  s.s1_flow_sensor = temperature(b, 0);
  s.flow = temperature(b, 2);
  s.s2_return = temperature(b, 4);
  s.s3_dhw = temperature(b, 6);
  s.s4_boiler = temperature(b, 8);
  s.s5_outside = temperature(b, 10);
  s.pressure_sensor = bit(b[28], 5);
  // IDS shows 0 above 3.7 bar instead of looking at the sensor bit; without a sensor the raw
  // value is around 6 bar.
  s.pressure = s.pressure_sensor ? s16(b, 12) / 100.0f : NAN;
  s.setpoint = s16(b, 14) / 100.0f;
  s.fan_speed_set = s16(b, 16);
  s.fan_speed = s16(b, 18);
  s.fan_pwm = s16(b, 20) / 10.0f;
  s.ionisation_current = s16(b, 22) / 100.0f;
  s.code = b[24];
  s.rf_bound = bit(b[25], 0);
  s.rf_communication = bit(b[25], 1);
  s.rf_battery_info = bit(b[25], 2);
  s.rf_battery_ok = bit(b[25], 3);
  s.gp_switch = bit(b[26], 0);
  s.tap_switch = bit(b[26], 1);
  s.roomtherm = bit(b[26], 2);
  s.pump = bit(b[26], 3);
  s.three_way_valve = bit(b[26], 4);
  s.opentherm = bit(b[26], 7);
  s.fault_active = b[27] == 128;
  s.gas_valve = bit(b[28], 0);
  s.spark = bit(b[28], 1);
  s.ionisation = bit(b[28], 2);
  s.fault_code = b[29];
  return s;
}

// ---- "S2\r": extra status --------------------------------------------------------------------
struct Status2 {
  float tap_flow;             // bytes 0-1
  float pump_pwm;             // byte 2, %
  uint8_t ot_master_id;       // byte 3, OpenTherm thermostat
  float room_override_zone1;  // bytes 4-5
  float room_setpoint_zone1;  // bytes 6-7
  float room_temperature_zone1;  // bytes 8-9
  float room_override_zone2;  // bytes 10-11
  float room_setpoint_zone2;  // bytes 12-13
  float room_temperature_zone2;  // bytes 14-15
  float outside_override;     // bytes 16-17
  uint8_t ot_version;         // byte 18
  uint8_t ot_type;            // byte 19
};

inline Status2 decode_status2(const uint8_t *b) {
  Status2 s;
  s.tap_flow = s16(b, 0) / 100.0f;
  s.pump_pwm = (200 - b[2]) / 2.0f;
  s.ot_master_id = b[3];
  s.room_override_zone1 = temperature(b, 4);
  s.room_setpoint_zone1 = temperature(b, 6);
  s.room_temperature_zone1 = temperature(b, 8);
  s.room_override_zone2 = temperature(b, 10);
  s.room_setpoint_zone2 = temperature(b, 12);
  s.room_temperature_zone2 = temperature(b, 14);
  s.outside_override = temperature(b, 16);
  s.ot_version = b[18];
  s.ot_type = b[19];
  return s;
}

// ---- "REV": version --------------------------------------------------------------------------
// Hardware release up to the first 'V', then the software release, ROM checksums in bytes 24-27
// and 28-31. As in IDS the software release is 4 characters ("V2.5"), or 5 ("V1.84") when the
// number is above 3 or the reply contains "ic3"; the version thresholds depend on this.
struct Version {
  std::string hardware;
  std::string software;
  std::string rom_checksum1, rom_checksum2;
  bool ic3;
  float software_number;
};

inline Version decode_version(const uint8_t *b) {
  const std::string reply((const char *) b, REPLY_LEN);
  Version v;
  size_t pos = reply.find('V');
  if (pos == std::string::npos)
    pos = REPLY_LEN;
  v.hardware = reply.substr(0, pos);
  v.ic3 = v.hardware.compare(0, 3, "ic3") == 0;
  while (!v.hardware.empty() && v.hardware.back() == ' ')
    v.hardware.pop_back();
  v.software = reply.substr(pos, 4);
  const float short_number = v.software.size() > 1 ? std::strtof(v.software.c_str() + 1, nullptr) : 0.0f;
  if (short_number > 3.0f || reply.find("ic3") != std::string::npos)
    v.software = reply.substr(pos, 5);
  v.software_number = v.software.size() > 1 ? std::strtof(v.software.c_str() + 1, nullptr) : 0.0f;
  v.rom_checksum1 = hex(b + 24, 4);
  v.rom_checksum2 = hex(b + 28, 4);
  return v;
}

// Conditions IDS (statistics.pyo) uses before sending a command.
inline bool has_params(const Version &v) { return v.ic3 || v.software_number >= 2.2f; }
inline bool has_ext_params(const Version &v) { return v.ic3 && v.software_number >= 1.31f; }
inline bool has_info(const Version &v) { return v.ic3 || v.software_number > 2.5f; }
inline bool has_info2(const Version &v) { return v.ic3 && v.software_number >= 1.9f; }

// ---- "CRC": burner control internals ---------------------------------------------------------
struct Internals {
  float interrupt_time, interrupt_load, main_load;  // load in %
  float net_frequency;                              // Hz
  float voltage_reference;                          // V
};

inline Internals decode_internals(const uint8_t *b, const Version &v) {
  Internals r;
  const float time_raw = u16(b, 0), freq_raw = u16(b, 6);
  r.main_load = u16(b, 4) / 8.0f;
  if (v.ic3) {
    r.interrupt_time = time_raw / 8.0f;
    r.interrupt_load = time_raw > 0 ? u16(b, 2) / time_raw * 100.0f : NAN;
    r.net_frequency = freq_raw > 0 ? 2000.0f / freq_raw : NAN;
    r.voltage_reference = u16(b, 8) * 3.3f / 1024.0f;
  } else {
    r.interrupt_time = time_raw / 5.0f;
    r.interrupt_load = u16(b, 2) / 6.25f;
    r.net_frequency = freq_raw > 0 ? 2000.0f / freq_raw : 0.0f;
    r.voltage_reference = u16(b, 8) * 5.0f / 1024.0f;
  }
  return r;
}

// ---- "HN\r": operating hours and counters ----------------------------------------------------
// IC2: counters only. IC3: + gas meters and high bytes. IC3 >= V1.83: + water meter, DHW starts.
struct Hours {
  uint32_t power_on_hours, power_cycles, ch_hours, dhw_hours;
  uint32_t burner_starts, ignition_failures, flame_losses, resets;
  float gas_ch, gas_dhw, water;  // NAN when not supported
  float dhw_burner_starts;       // NAN when not supported
};

inline Hours decode_hours(const uint8_t *b, const Version &v) {
  Hours h;
  h.power_on_hours = u16(b, 0);
  h.power_cycles = u16(b, 2);
  h.ch_hours = u16(b, 4);
  h.dhw_hours = u16(b, 6);
  h.burner_starts = u16(b, 8);
  h.ignition_failures = u16(b, 10);
  h.flame_losses = u16(b, 12);
  h.resets = u16(b, 14);
  h.gas_ch = h.gas_dhw = h.water = h.dhw_burner_starts = NAN;
  if (!v.ic3)
    return h;
  h.power_on_hours += (uint32_t) b[30] << 16;
  h.burner_starts += (uint32_t) b[31] << 16;
  h.gas_ch = (float) (u32(b, 16) / 10000.0);
  h.gas_dhw = (float) (u32(b, 20) / 10000.0);
  if (v.software_number >= 1.83f) {
    h.water = (float) ((b[24] | (b[25] << 8) | ((uint32_t) b[28] << 16)) / 10000.0);
    h.dhw_burner_starts = b[26] | (b[27] << 8) | ((uint32_t) b[29] << 16);
  }
  return h;
}

// ---- "EN\r": fault counters ------------------------------------------------------------------
// One counter per fault code 0-31 ("number of times" in IDS), stored as count - 1 (0xFF = 0).
inline uint8_t fault_count(const uint8_t *b, size_t code) { return (uint8_t) (b[code] + 1); }

// ---- "V?\r" (positions 0-31, signed) and "V1\r" (positions 32-63, unsigned) ---------------------
inline int8_t param(const uint8_t *b, size_t pos) { return (int8_t) b[pos]; }
static const size_t PARAM_COUNT = 31;      // positions 0-30 of "V?"
static const size_t EXT_PARAM_COUNT = 31;  // positions 32-62 of "V1"; position 31 via CMD_PARAM_31

// ---- "B?\r": info block ----------------------------------------------------------------------
// Bytes 25-31: production token, year, month, line, serial 1-3; formatted as IDS does.
inline std::string decode_production_code(const uint8_t *b) {
  static const char printable[] =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r\x0b\x0c";
  std::string out;
  char tmp[8];
  for (int i = 0; i < 7; i++) {
    const uint8_t v = b[25 + i];
    if (i == 3 && v < sizeof(printable) - 1) {
      out += printable[v];
      continue;
    }
    snprintf(tmp, sizeof(tmp), (i == 3 || i == 4) ? "%u" : "%02u", (unsigned) v);
    out += tmp;
  }
  return out;
}

// ---- Texts -----------------------------------------------------------------------------------
// Operating code (byte 24 of the status), labels from IDS v1.83.
inline const char *operating_text(uint8_t code) {
  switch (code) {
    case 0: return "CV-vraag OpenTherm";
    case 15: return "Boiler extern";
    case 24: return "Vorstbeveiliging";
    case 37: return "CV-vraag RF-thermostaat";
    case 51: return "Tapwater intern (warmhouden)";
    case 85: return "Sensortest";
    case 102: return "CV-bedrijf";
    case 126: return "Rust";
    case 153: return "Nadraaien boiler";
    case 170: return "Service";
    case 204: return "Tapwater";
    case 205: return "Tapwater (HReco)";
    case 231: return "Nadraaien CV";
    case 240: return "Boiler intern";
    case 255: return "Opwarmen";
    default: return nullptr;
  }
}

// Fault codes from the installation manual §8.2 and "HRE Parameters v6" (iC3).
inline const char *fault_text(uint8_t code) {
  switch (code) {
    case 0: return "Sensorfout na zelftest";
    case 1: return "Temperatuur te hoog";
    case 2: return "Verwisseling S1 en S2";
    case 4: return "Geen vlamsignaal";
    case 5: return "Slecht vlamsignaal";
    case 6: return "Vlamdetectiefout";
    case 8: return "Ventilatortoerental niet juist";
    case 10: return "S1 open";
    case 11: return "S1 kortsluiting";
    case 12: return "S1 temperatuursprong -10 °C";
    case 13: return "S1 temperatuursprong +10 °C";
    case 14: return "S1 geen temperatuurverandering";
    case 20: return "S2 open";
    case 21: return "S2 kortsluiting";
    case 22: return "S2 temperatuursprong -10 °C";
    case 23: return "S2 temperatuursprong +10 °C";
    case 24: return "S2 geen temperatuurverandering";
    case 25: return "Rookgassensor te lang gesloten";
    case 26: return "Rookgassensor niet gesloten";
    case 27: return "Kortsluiting buitenvoeler";
    case 29:
    case 30: return "Gasklep-relaisfout / vlamdetectiefout";
    default: return nullptr;
  }
}

}  // namespace hre
