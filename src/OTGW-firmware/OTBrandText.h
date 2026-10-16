#pragma once
#include <stdint.h>
#include <string.h>

// ID93-95: request HB is character index; reply HB is string length
// (the v4.2 example returns 6 for "boiler"). Only actual B replies are used.
class OTBrandText {
public:
  static constexpr uint8_t Capacity = 50;
  enum class Support : uint8_t { Unknown, Supported, Unsupported };
  struct Text {
    char collecting[Capacity + 1] = {};
    char complete[Capacity + 1] = {};
    uint64_t received = 0;
    uint8_t length = 0;
    bool ready = false;
    bool dirty = false;
    bool clearPending = false;
    Support support = Support::Unknown;
  };
  Text texts[3];

  void observe(char source, uint32_t frame, uint32_t now) {
    if (source == 'E') { pending = false; return; }
    uint32_t bits = frame;
    uint8_t parity = 0;
    while (bits) { parity ^= bits & 1U; bits >>= 1; }
    if (parity || ((frame >> 24) & 15U)) { pending = false; return; }
    const uint8_t type = (frame >> 28) & 7U;
    const uint8_t id = (frame >> 16) & 255U;
    const uint8_t hb = (frame >> 8) & 255U;
    const uint8_t lb = frame & 255U;
    if (source == 'T' || source == 'R') {
      pending = type == 0 && id >= 93 && id <= 95 && hb < Capacity && lb == 0;
      requestId = id; requestIndex = hb; requestTime = now;
      return;
    }
    if (source != 'B') return;
    const bool match = pending && id == requestId &&
      static_cast<uint32_t>(now - requestTime) <= 1500U;
    pending = false;
    if (!match) return;
    Text& text = texts[id - 93];
    if (type == 7) {
      text.support = Support::Unsupported;
      text.received = 0; text.length = 0; text.ready = false;
      text.complete[0] = '\0'; text.dirty = false; text.clearPending = true;
      return;
    }
    if (type == 4 || type == 6) text.support = Support::Supported;
    if (type != 4 || hb == 0 || hb > Capacity || requestIndex >= hb ||
        lb < 32 || lb > 126) {
      // A failed character must not leave an older partial read completable.
      if (requestIndex < Capacity) text.received &= ~(uint64_t(1) << requestIndex);
      return;
    }
    if (requestIndex == 0 || text.length != hb) {
      text.received = 0;
      memset(text.collecting, 0, sizeof(text.collecting));
      text.length = hb;
    }
    text.collecting[requestIndex] = static_cast<char>(lb);
    text.received |= uint64_t(1) << requestIndex;
    const uint64_t expected = (uint64_t(1) << text.length) - 1;
    if (text.received != expected) return;
    text.collecting[text.length] = '\0';
    if (!text.ready || strcmp(text.complete, text.collecting) != 0) {
      memcpy(text.complete, text.collecting, sizeof(text.complete));
      text.dirty = true;
    }
    text.ready = true;
    text.clearPending = false;
  }
  void snapshot(uint8_t slot, char* characters, char* mask, uint8_t& count, bool& finished) const {
    const Text& text = texts[slot];
    count = 0;
    for (uint8_t i = 0; i < text.length; ++i) {
      const bool present = (text.received & (uint64_t(1) << i)) != 0;
      characters[i] = present ? text.collecting[i] : ' ';
      mask[i] = present ? '1' : '0';
      if (present) ++count;
    }
    characters[text.length] = '\0'; mask[text.length] = '\0';
    finished = text.length > 0 && count == text.length;
  }
private:
  bool pending = false;
  uint8_t requestId = 0, requestIndex = 0;
  uint32_t requestTime = 0;
};
