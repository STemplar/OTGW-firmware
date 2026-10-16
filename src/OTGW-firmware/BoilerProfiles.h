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
