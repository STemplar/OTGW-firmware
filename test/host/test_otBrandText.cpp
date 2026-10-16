#include <assert.h>
#include <stdio.h>
#include "../../src/OTGW-firmware/OTBrandText.h"
static uint32_t frame(uint8_t type, uint8_t id, uint8_t hb, uint8_t lb) {
  uint32_t f = (uint32_t(type) << 28) | (uint32_t(id) << 16) | (uint32_t(hb) << 8) | lb;
  uint32_t v=f; uint8_t p=0;
  while(v) {p ^= v&1; v>>=1;}
  return f | (uint32_t(p)<<31);
}
static void character(OTBrandText& b, uint8_t id, uint8_t index, uint8_t length, char c) {
  b.observe('T',frame(0,id,index,0),100);
  b.observe('B',frame(4,id,length,c),200);
}
int main() {
  OTBrandText b;
  // Reply HB is length, not requested index; incomplete text is never ready.
  character(b,93,0,6,'b'); character(b,93,1,6,'o');
  assert(!b.texts[0].ready);
  character(b,93,2,6,'i'); character(b,93,3,6,'l');
  character(b,93,4,6,'e'); character(b,93,5,6,'r');
  assert(b.texts[0].ready && b.texts[0].dirty);
  assert(!strcmp(b.texts[0].complete,"boiler"));
  b.texts[0].dirty=false;
  const char* s="boiler";
  for(uint8_t i=0;i<6;++i) character(b,93,i,6,s[i]);
  assert(!b.texts[0].dirty);
  // Sources remain independent; A overrides cannot replace the actual B character.
  character(b,94,0,1,'V'); character(b,95,0,1,'S');
  b.observe('A',frame(4,94,1,'X'),210);
  assert(!strcmp(b.texts[1].complete,"V"));
  assert(!strcmp(b.texts[2].complete,"S"));
  OTBrandText x;
  x.observe('T',frame(0,93,0,0),100);
  x.observe('R',frame(0,77,0,0),101);
  x.observe('B',frame(4,93,1,'X'),102);
  assert(!x.texts[0].ready);
  x.observe('B',frame(4,93,1,'X'),102); // no matching request
  assert(!x.texts[0].ready);
  x.observe('R',frame(0,93,0,0),100);
  x.observe('B',frame(4,93,1,'X'),1700); // timeout
  assert(!x.texts[0].ready);
  character(x,93,0,2,'a');
  x.observe('T',frame(0,93,1,0),100);
  x.observe('B',frame(6,93,2,0),200); // DATA_INVALID resets partial text
  char chars[51], mask[51]; uint8_t count; bool finished;
  x.snapshot(0,chars,mask,count,finished);
  assert(count==1 && !finished && !strcmp(mask,"10"));
  character(x,93,1,2,'b');
  assert(x.texts[0].ready && !strcmp(x.texts[0].complete,"ab"));
  OTBrandText invalid;
  x=invalid;
  character(x,93,0,1,0); character(x,93,0,51,'x');
  character(x,93,0,0,'x'); character(x,93,1,1,'x');
  assert(!x.texts[0].ready);
  x.observe('T',frame(0,93,0,0),100);
  x.observe('B',frame(4,93,1,'x')^0x80000000U,200);
  assert(!x.texts[0].ready);
  x.observe('T',frame(0,93,0,0),100);
  // Reserved header bit with corrected parity must still be rejected.
  x.observe('B',frame(4,93,1,'x') ^ 0x81000000U,200);
  assert(!x.texts[0].ready);
  // Largest accepted string (indices 0..49) fits and is terminated.
  for(uint8_t i=0;i<50;++i) character(x,95,i,50,'A');
  assert(strlen(x.texts[2].complete)==50);
  // Out-of-order responses can complete after an index-zero start.
  OTBrandText y;
  character(y,93,0,3,'a');character(y,93,2,3,'c');character(y,93,1,3,'b');
  assert(!strcmp(y.texts[0].complete,"abc"));
  // Request time survives millis rollover.
  y.observe('T',frame(0,94,0,0),0xfffffffeU);
  y.observe('B',frame(4,94,1,'R'),10);
  assert(!strcmp(y.texts[1].complete,"R"));
  character(y,93,0,1,'N');
  assert(!strcmp(y.texts[0].complete,"N") && y.texts[0].dirty);
  OTBrandText supported;
  assert(supported.texts[0].support==OTBrandText::Support::Unknown);
  supported.observe('T',frame(0,93,0,0),100);
  supported.observe('A',frame(7,93,0,0),200);
  assert(supported.texts[0].support==OTBrandText::Support::Unknown);
  supported.observe('B',frame(7,93,0,0),201);
  assert(supported.texts[0].support==OTBrandText::Support::Unsupported);
  assert(supported.texts[0].clearPending);
  supported.observe('T',frame(0,93,0,0),300);
  supported.observe('B',frame(6,93,1,0),400);
  assert(supported.texts[0].support==OTBrandText::Support::Supported);
  character(supported,93,0,1,'Z');
  assert(!supported.texts[0].clearPending && supported.texts[0].ready);
  supported.snapshot(0,chars,mask,count,finished);
  assert(count==1 && finished && !strcmp(chars,"Z"));
  printf("PASS: request/reply correlation, partial reads, duplicates, overrides, errors, parity, limits, order, rollover; RAM=%zu bytes\n",sizeof(OTBrandText));
}
