#pragma once
#include <stdint.h>

// Observations from the boiler matrix, 2026-10-13. A vendor entry is not a model guarantee.
// Only IDs observed as reads are eligible. Write-only observations never authorize a read.
inline bool boilerOEMReadAllowed(uint8_t member, uint8_t id) {
  switch (member) {
    // Member 2 removed: its 128-202 'reads' (Bulex FAS, Remeha Quinta 28c) are a memory dump from an
    // ID sweep (LB of ID n equals HB of ID n+1), not OEM data IDs.
    case 4: return id == 128 || id == 129 || id == 130 || id == 131 || id == 132 || id == 133 || id == 134 || id == 135 || id == 136 || id == 189;
    case 5: return id == 130 || id == 133 || id == 134 || id == 141 || id == 142 || id == 143 || id == 146 || id == 151 || id == 153 || id == 154 || id == 155 || id == 156 || id == 157 || id == 158 || id == 159 || id == 160 || id == 170 || id == 171 || id == 172;
    case 11: return id >= 131 && id <= 133;
    case 27: return id == 128 || id == 151;
    case 33: return id >= 128 && id <= 128;
    case 131: return id >= 128 && id <= 128;
    case 161: return id == 128 || id == 142 || id == 143 || id == 144 || id == 145 || id == 148 || id == 149 || id == 160 || id == 161 || id == 200;
    case 173: return id == 200 || id == 202 || id == 204 || id == 205 || id == 220;
    case 211: return id == 128 || id == 130 || id == 149 || id == 150 || id == 151 || id == 160 || id == 161 || id == 162 || id == 165 || id == 173 || id == 200 || id == 202 || id == 203 || id == 204;
    default: return false;
  }
}

// Model profiles: the single source for boiler-model recognition and for the profile file the web
// UI loads (data/<file>). A model matches on real boiler READ-ACKs only: slave member ID (ID3 LB),
// product type/version (ID127) and, when oem131 is set, the dF/dU type code in OEM ID131.
// OT 4.2 leaves ID5 LB, ID115 and IDs 128-255 to the manufacturer, so their meanings live in the
// profile file, never in generic code. Keep file names <= 30 chars (LittleFS path limit).
struct BoilerModelProfile {
  const char* id;           // also the "id" field inside the profile file
  const char* file;         // profile file on LittleFS
  uint8_t member;           // ID3 LB
  uint8_t productType;      // ID127 HB
  uint8_t versions[4];      // accepted ID127 LB values
  uint8_t versionCount;
  uint16_t oem131;          // required ID131 value (HB=dF, LB=dU); 0 = not used
};
static const BoilerModelProfile kBoilerModelProfiles[] = {
  // Remeha Tzerra M 28c Plus: type plate dF 36 / dU 1; ID127 13/14 and 13/16 seen in the matrix.
  { "remeha-tzerra-m-28c-plus", "tzerra-m-28c-plus.json", 11, 13, {14, 16}, 2, 0x2401 },
  // Remeha Tzerra Ace Matic: ID127 27/1 (matrix, 3 logs).
  { "remeha-tzerra-ace-matic", "tzerra-ace-matic.json", 11, 27, {1}, 1, 0 },
};
static const uint8_t kBoilerModelProfileCount = sizeof(kBoilerModelProfiles) / sizeof(kBoilerModelProfiles[0]);
