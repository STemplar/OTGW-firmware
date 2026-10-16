#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "BoilerProfiles.h"

// Real boiler replies only. No dynamic allocation, writes or synthetic responses.
// PIC 6.8 owns indexed string/TSP/FHB traversal after a single PM request.
class BoilerInfo {
public:
  enum Support : uint8_t { Unknown, Supported, Unsupported };
  enum Group : uint8_t { Identification, Operating, Parameters, History, OEM };
  uint16_t values[256] = {};
  uint8_t support[256] = {};
  uint8_t tsp[256] = {}, fhb[256] = {};
  uint8_t tspSeen[32] = {}, fhbSeen[32] = {};
  bool busSeen = false, bootReadStarted = false, busy = false;
  const char* result = "idle";

  bool has(uint8_t id) const { return (seen[id >> 3] & (1U << (id & 7))) != 0; }
  bool remeha() const { return has(3) && (values[3] & 255) == 11; }
  bool tzerra() const { return remeha() && has(127) && values[127] == 0x0d10; }
  bool oemAllowed() const {
    if (!has(3)) return false;
    // Observed slave member IDs with OEM read traffic. Not brand-name guesses.
    switch (values[3] & 255) {
      case 2: case 4: case 5: case 8: case 11: case 27: case 33:
      case 131: case 161: case 173: case 211: return true;
      default: return false;
    }
  }
  bool oemReadable() const {
    if (!has(3)) return false;
    for (uint16_t id = 128; id < 256; ++id) if (boilerOEMReadAllowed(values[3] & 255, id)) return true;
    return false;
  }
  bool start(Group requested, uint32_t now) {
    if (busy || !busSeen || (requested == OEM && !oemReadable())) return false;
    if (requested == Identification) bootReadStarted = true;
    group = requested; position = 0; waiting = false; busy = true; result = "queued"; timedOut = false;
    started = lastSend = now; sent = false; remaining = 0; textReceived = 0;
    if (group == Parameters) memset(tspSeen, 0, sizeof(tspSeen));
    if (group == History) memset(fhbSeen, 0, sizeof(fhbSeen));
    return true;
  }
  // Caller admits a command only when its serial queue is empty and live gateway mode is known.
  bool next(uint32_t now, bool transportReady, char* command, size_t size) {
    if (busy && uint32_t(now - started) >= 360000U) { finish("read timed out"); return false; }
    if (!transportReady) return false;
    if (!bootReadStarted && busSeen && !busy) {
      bootReadStarted = true; start(Identification, now);
    }
    if (!busy) return false;
    if (waiting) {
      if (uint32_t(now - lastSend) < (group == Parameters || group == History ? 300000U : (target >= 93 && target <= 95 ? 90000U : 15000U))) return false;
      waiting = false; timedOut = true; result = "read timed out";
    }
    if (sent && uint32_t(now - lastSend) < 2000U) return false;
    const uint8_t* ids = identification;
    uint8_t count = sizeof(identification);
    if (group == Operating) { ids = operating; count = sizeof(operating); }
    if (group == Parameters) { ids = parameters; count = sizeof(parameters); }
    if (group == History) { ids = history; count = sizeof(history); }
    if (group == OEM) {
      count = 128;
      while (position < count && !boilerOEMReadAllowed(values[3] & 255, 128 + position)) ++position;
    }
    if (position >= count) { finish(timedOut ? "read finished with missing replies" : "read finished"); return false; }
    target = group == OEM ? uint8_t(128 + position++) : ids[position++]; remaining = 0; textReceived = 0;
    const int n = snprintf(command, size, "PM=%u", target);
    if (n < 0 || size_t(n) >= size) { finish("command buffer too small"); return false; }
    waiting = sent = true; lastSend = now; result = "reading";
    return true;
  }
  void observe(char source, uint32_t frame, uint32_t now) {
    uint32_t bits = frame; uint8_t parity = 0;
    while (bits) { parity ^= bits & 1U; bits >>= 1; }
    if (source == 'E' || parity || ((frame >> 24) & 15U)) { pending = false; return; }
    const uint8_t type = (frame >> 28) & 7, id = frame >> 16;
    const uint16_t data = frame;
    if (source == 'T' || source == 'R') {
      if (source == 'T' && id == 2 && type == 1) { values[2] = data; seen[0] |= 4; }
      pending = type == 0; requestId = id; requestIndex = data >> 8; requestAt = now; return;
    }
    if (source != 'B') return;
    busSeen = true;
    const bool matched = pending && requestId == id && uint32_t(now - requestAt) <= 1500U;
    pending = false;
    if (!matched || (type != 4 && type != 6 && type != 7)) return;
    support[id] = type == 7 ? Unsupported : Supported;
    if (type == 4) {
      values[id] = data; seen[id >> 3] |= 1U << (id & 7);
      const uint8_t index = data >> 8;
      if (id == 11) { tsp[index] = data; tspSeen[index >> 3] |= 1U << (index & 7); }
      if (id == 13) { fhb[index] = data; fhbSeen[index >> 3] |= 1U << (index & 7); }
    } else if (type == 7) seen[id >> 3] &= ~(1U << (id & 7));
    if (!busy || !waiting) return;
    if (id == target && id >= 93 && id <= 95 && type == 4) {
      const uint8_t length = data >> 8;
      if (!length || length > 50 || requestIndex >= length) { waiting = false; return; }
      if (requestIndex == 0 || remaining != length) textReceived = 0;
      remaining = length;
      textReceived |= uint64_t(1) << requestIndex;
      if (textReceived == (uint64_t(1) << length) - 1) waiting = false;
      return;
    }
    if ((group == Parameters || group == History) && id == target && type == 4 && (data >> 8) != 0) {
      remaining = data >> 8; return; // PIC owns the indexed traversal.
    }
    const uint8_t entryId = group == Parameters ? 11 : 13;
    if ((group == Parameters || group == History) && id == entryId && remaining && type == 4) {
      const uint8_t* bitmap = group == Parameters ? tspSeen : fhbSeen;
      for (uint16_t i = 0; i < remaining; ++i) if (!(bitmap[i >> 3] & (1U << (i & 7)))) return;
      waiting = false; return;
    }
    if (id == target) waiting = false;
  }
private:
  uint8_t seen[32] = {};
  bool timedOut = false;
  bool pending = false, waiting = false, sent = false;
  uint8_t requestIndex = 0;
  uint64_t textReceived = 0;
  uint8_t requestId = 0, target = 0, position = 0, remaining = 0;
  uint32_t requestAt = 0, started = 0, lastSend = 0;
  Group group = Identification;
  static constexpr uint8_t identification[] = {3, 127, 125, 93, 94, 95};
  static constexpr uint8_t operating[] = {5, 115};
  static constexpr uint8_t parameters[] = {10};
  static constexpr uint8_t history[] = {12};
  void finish(const char* message) { busy = waiting = false; remaining = 0; result = message; }
};
// C++11 definitions: Arduino's ESP8266 toolchain predates inline variables.
constexpr uint8_t BoilerInfo::identification[];
constexpr uint8_t BoilerInfo::operating[];
constexpr uint8_t BoilerInfo::parameters[];
constexpr uint8_t BoilerInfo::history[];
