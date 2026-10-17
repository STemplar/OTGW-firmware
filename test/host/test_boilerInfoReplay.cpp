// Replays the reads measured on a Remeha Tzerra M 28c Plus (17-10-2026, TASK-1241) against the real
// BoilerInfo.h. Only the platform is emulated: thermostat, PIC and boiler. The PIC follows OTGW 5.8
// gateway.asm: one priority message, kept until a response with that ID arrives and otherwise retried
// in the next available slot (HandleResponse, SendPriorityMsg); a new PM replaces it and resets the
// TSP index (SetPrioMessage); TSP/FHB traversal runs from index 0 to count-1 (TSPReadEntry).
// A slot is the thermostat's ID19 request, which this boiler answers with UNKNOWN-DATAID.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/OTGW-firmware/BoilerInfo.h"

static uint32_t frame(uint8_t type, uint8_t id, uint8_t hb = 0, uint8_t lb = 0) {
  uint32_t f = uint32_t(type) << 28 | uint32_t(id) << 16 | uint32_t(hb) << 8 | lb, b = f; uint8_t p = 0;
  while (b) { p ^= b & 1; b >>= 1; }
  return f | uint32_t(p) << 31;
}

// Boiler answers as captured: type and data for a read of id (index in HB for indexed IDs).
static uint32_t boilerReply(uint8_t id, uint8_t index) {
  switch (id) {
    case 3: return frame(4, 3, 0x41, 0x0B);
    case 127: return frame(4, 127, 13, 16);
    case 125: return frame(4, 125, 3, 0);
    case 131: return frame(4, 131, 0x24, 0x01);
    case 10: return frame(4, 10, 45, 0);
    case 11: return index ? frame(4, 11, index, uint8_t(index * 3)) : frame(6, 11, 0, 0);
    case 25: return frame(4, 25, 0x2B, 0x21);
    case 93: case 94: case 95: case 12: case 13: case 19: return frame(7, id, index, 0);
    default: return frame(4, id, 0, 1); // PIC alternatives ID116-123: counters
  }
}

struct Bench {
  BoilerInfo b;
  uint32_t now = 0, firstSlot, period;
  int pm = -1; uint8_t tspIndex = 0, tspCount = 0;
  uint8_t passiveId = 0; uint32_t passiveAt = 0;
  char sent[16][8] = {}; uint32_t sentAt[16] = {}; unsigned sentCount = 0;
  Bench(uint32_t first, uint32_t every) : firstSlot(first), period(every) {
    b.observe('T', frame(0, 25), 0); b.observe('B', boilerReply(25, 0), 1); // bus alive
    b.bootReadStarted = true;
  }
  void exchange(char reqSource, uint8_t id, uint8_t index) {
    b.observe(reqSource, frame(0, id, index), now);
    const uint32_t reply = boilerReply(id, index);
    b.observe('B', reply, now); // firmware stamps both with millis() at receipt: time never runs backwards
    if (pm == id) { // HandleResponse: any response with the priority ID clears it
      const bool gateway = reqSource == 'R';
      pm = -1;
      if (gateway && id == 10 && ((reply >> 28) & 7) == 4 && (reply >> 8 & 255)) { tspCount = reply >> 8 & 255; pm = 11; }
      else if (gateway && id == 11 && ++tspIndex < tspCount) pm = 11;
    }
  }
  void tick() {
    char cmd[12];
    if (b.next(now, true, cmd, sizeof(cmd))) {
      assert(!strncmp(cmd, "PM=", 3) && sentCount < 16);
      strcpy(sent[sentCount], cmd); sentAt[sentCount++] = now;
      pm = atoi(cmd + 3); tspIndex = tspCount = 0; // SetPrioMessage
    }
    if (now % 1200 == 0) exchange('T', 25, 0); // routine thermostat polling, always supported
    if (passiveId && now == passiveAt) exchange('T', passiveId, 0);
    if (now >= firstSlot && (now - firstSlot) % period == 0) { // thermostat sends ID19: a slot
      b.observe('T', frame(0, 19), now);
      if (pm >= 0) exchange('R', uint8_t(pm), pm == 11 ? tspIndex : 0);
      else exchange('R', 116, 0);
    }
  }
  void run(uint32_t limit) {
    for (; b.busy && now < limit; now += 100) tick();
    printf("  result after %u s: %s; sent:", unsigned(now / 1000), b.result);
    for (unsigned i = 0; i < sentCount; ++i) printf(" %s@%us", sent[i], unsigned(sentAt[i] / 1000));
    printf("\n");
  }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0); // keep the trace when an assert aborts
  const int only = argc > 1 ? atoi(argv[1]) : 0; // run one scenario, e.g. to show each failure on old code
  // 1. Identification read 12:20:31: first slot after 12.6 s, then every 23.4 s; the thermostat itself
  //    reads ID3 at 4.5 s. Every ID must be requested once and answered.
  if (!only || only == 1) { puts("identification, slot every 23.4 s:"); Bench t(12600, 23400); t.passiveId = 3; t.passiveAt = 4500;
    assert(t.b.start(BoilerInfo::Identification, 0)); t.run(3600000);
    const char* want[] = {"PM=3", "PM=127", "PM=125", "PM=93", "PM=94", "PM=95", "PM=131"};
    assert(t.sentCount == 7);
    for (unsigned i = 0; i < 7; ++i) assert(!strcmp(t.sent[i], want[i]));
    assert(t.b.support[125] == BoilerInfo::Supported && t.b.values[125] == 0x0300);
    for (unsigned id = 93; id <= 95; ++id) assert(t.b.support[id] == BoilerInfo::Unsupported);
    assert(t.b.values[131] == 0x2401 && t.b.profile() && !strcmp(t.b.profile()->id, "remeha-tzerra-m-28c-plus"));
    assert(!t.b.busy && !strcmp(t.b.result, "read finished")); }

  // 2. Parameters read 12:38:39: PM=10, then the PIC reads ID11 index 0..44, one slot each (46 slots, ~18 min).
  if (!only || only == 2) { puts("parameters, 46 slots:"); Bench t(3000, 23400); assert(t.b.start(BoilerInfo::Parameters, 0)); t.run(3600000);
    assert(t.sentCount == 1 && !strcmp(t.sent[0], "PM=10"));
    for (unsigned i = 1; i <= 44; ++i) assert((t.b.tspSeen[i >> 3] & (1U << (i & 7))) && t.b.tsp[i] == uint8_t(i * 3));
    assert((t.b.tspSeen[0] & 1) == 0); // index 0: DATA-INVALID, no value
    assert(!t.b.busy && !strcmp(t.b.result, "read finished") && t.now > 1000000); }

  // 3. After a PIC restart: no slot for 200 s (unsupported IDs relearned, admin messages first).
  if (!only || only == 3) { puts("identification, first slot after 200 s:"); Bench t(200000, 23400); assert(t.b.start(BoilerInfo::Identification, 0)); t.run(3600000);
    assert(t.b.support[125] == BoilerInfo::Supported && t.b.values[131] == 0x2401);
    assert(!t.b.busy && !strcmp(t.b.result, "read finished")); }

  // 4. No slots at all: each request gets the full 300 s safety limit, then the read ends with missing replies.
  if (!only || only == 4) { puts("identification, no slots:"); Bench t(0xFFFFFFF0U, 23400); assert(t.b.start(BoilerInfo::Identification, 0)); t.run(7200000);
    assert(t.sentCount >= 2);
    for (unsigned i = 1; i < t.sentCount; ++i) assert(t.sentAt[i] - t.sentAt[i - 1] >= 300000);
    assert(t.b.support[125] == BoilerInfo::Unknown);
    assert(!t.b.busy && !strcmp(t.b.result, "read finished with missing replies")); }

  puts("PASS: replay of measured slot timing (identification, parameters, PIC restart, no slots)");
}
