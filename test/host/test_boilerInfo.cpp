#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../src/OTGW-firmware/BoilerInfo.h"
static uint32_t frame(uint8_t type,uint8_t id,uint8_t hb=0,uint8_t lb=0) {
  uint32_t f=uint32_t(type)<<28|uint32_t(id)<<16|uint32_t(hb)<<8|lb,b=f;uint8_t p=0;
  while(b){p^=b&1;b>>=1;}return f|uint32_t(p)<<31;
}
static void reply(BoilerInfo& b,uint8_t id,uint8_t hb,uint8_t lb,uint32_t t=100) {
 b.observe('R',frame(0,id),t);b.observe('B',frame(4,id,hb,lb),t+1);
}
int main(){
 BoilerInfo b;char cmd[12];
 assert(!b.next(0,true,cmd,sizeof(cmd))); // No traffic: no speculative requests.
 reply(b,3,0,11);reply(b,127,13,16);
 assert(b.needsOem131()&&!b.profile()&&b.oemAllowed()); // Tzerra M Plus family: model needs dF/dU first
 { BoilerInfo v;reply(v,3,0,11);reply(v,127,13,14);assert(v.needsOem131()); // other observed product version
   reply(v,127,13,15);assert(!v.needsOem131());reply(v,127,12,16);assert(!v.needsOem131()); }
 assert(!b.next(1000,false,cmd,sizeof(cmd)));
 assert(b.next(1000,true,cmd,sizeof(cmd))&&!strcmp(cmd,"PM=3"));
 assert(!b.next(1001,true,cmd,sizeof(cmd)));
 reply(b,3,0,11,1002);
 assert(b.next(3000,true,cmd,sizeof(cmd))&&!strcmp(cmd,"PM=127"));reply(b,127,13,16,3001);
 assert(b.next(5000,true,cmd,sizeof(cmd))&&!strcmp(cmd,"PM=125"));reply(b,125,4,0,5001);
 assert(b.next(7000,true,cmd,sizeof(cmd))&&!strcmp(cmd,"PM=93"));
 for(unsigned i=0;i<3;++i){b.observe('R',frame(0,93,i),7100+i*100);b.observe('B',frame(4,93,3,'A'+i),7101+i*100);if(i<2)assert(!b.next(7200+i*100,true,cmd,sizeof(cmd)));}
 assert(b.next(9000,true,cmd,sizeof(cmd))&&!strcmp(cmd,"PM=94"));
 b.observe('R',frame(0,94),9001);b.observe('A',frame(7,94),9002);
 assert(!b.next(10000,true,cmd,sizeof(cmd))); // A is never evidence.
 b.observe('B',frame(7,94),9003);
 assert(b.support[94]==BoilerInfo::Unsupported);
 assert(b.next(11000,true,cmd,sizeof(cmd))&&!strcmp(cmd,"PM=95"));
 // A timeout is unknown, not unsupported; Remeha then reads dF/dU (ID131).
 assert(b.next(102000,true,cmd,sizeof(cmd))&&!strcmp(cmd,"PM=131"));
 assert(b.support[95]==BoilerInfo::Unknown);
 assert(!b.profile());reply(b,131,0x24,0x01,102001);assert(b.profile()&&!strcmp(b.profile()->id,"remeha-tzerra-m-28c-plus"));
 assert(!b.next(104000,true,cmd,sizeof(cmd))&&!b.busy);
 assert(!b.next(106000,true,cmd,sizeof(cmd))); // one boot cycle only
 { BoilerInfo v;reply(v,3,0,11);reply(v,127,13,14);reply(v,131,0x25,0x01);assert(v.needsOem131()&&!v.profile()); } // dF 37/dU 1: no profile
 { BoilerInfo m;reply(m,3,0,11);reply(m,127,27,1);assert(m.profile()&&!strcmp(m.profile()->id,"remeha-tzerra-ace-matic")&&!m.needsOem131());
   reply(m,127,23,1);assert(!m.profile()); // Remeha Tzerra 23/1: generic
   reply(m,127,27,1);reply(m,3,0,4);assert(!m.profile()); } // ID127 alone never selects a profile
 // Table consistency: every profile file exists in data/, carries the same id, and fits LittleFS.
 for(uint8_t i=0;i<kBoilerModelProfileCount;++i){const BoilerModelProfile& p=kBoilerModelProfiles[i];
   assert(strlen(p.file)+1<=31&&p.versionCount>=1&&p.versionCount<=4);
   char path[96];snprintf(path,sizeof(path),"src/OTGW-firmware/data/%s",p.file);FILE* f=fopen(path,"rb");assert(f);
   static char buf[65536];size_t n=fread(buf,1,sizeof(buf)-1,f);fclose(f);buf[n]=0;
   char want[96];snprintf(want,sizeof(want),"\"id\": \"%s\"",p.id);assert(strstr(buf,want));
   for(uint8_t k=0;k<i;++k) assert(strcmp(kBoilerModelProfiles[k].id,p.id)&&strcmp(kBoilerModelProfiles[k].file,p.file)); }
 { BoilerInfo a;char c[12];reply(a,3,0,11);reply(a,127,27,1);a.bootReadStarted=true;assert(a.start(BoilerInfo::Identification,0)); // Ace Matic: no PM=131
   const char* seq[]={"PM=3","PM=127","PM=125","PM=93","PM=94","PM=95"};uint32_t now=0;
   for(const char* s:seq){assert(a.next(now,true,c,sizeof(c))&&!strcmp(c,s));uint8_t id=atoi(s+3);a.observe('R',frame(0,id),now+1);
     if(id==3)a.observe('B',frame(4,3,0,11),now+2);else if(id==127)a.observe('B',frame(4,127,27,1),now+2);else a.observe('B',frame(7,id),now+2);now+=3000;}
   assert(!a.next(now,true,c,sizeof(c))&&!a.busy); }
 { BoilerInfo g;char c[12];reply(g,3,0,8);g.bootReadStarted=true;assert(g.start(BoilerInfo::Identification,0)); // no ID131 for other vendors
   const char* seq[]={"PM=3","PM=127","PM=125","PM=93","PM=94","PM=95"};uint32_t now=0;
   for(const char* s:seq){assert(g.next(now,true,c,sizeof(c))&&!strcmp(c,s));g.observe('R',frame(0,atoi(s+3)),now+1);g.observe('B',frame(7,atoi(s+3)),now+2);now+=3000;}
   assert(!g.next(now,true,c,sizeof(c))&&!g.busy); }
 assert(b.start(BoilerInfo::Identification,105000)); // explicit retry includes previously unsupported IDs
 assert(!b.start(BoilerInfo::Operating,105001));
 BoilerInfo t;reply(t,3,0,11);t.bootReadStarted=true;
 assert(t.start(BoilerInfo::Parameters,1000));
 assert(t.next(1000,true,cmd,sizeof(cmd))&&!strcmp(cmd,"PM=10"));reply(t,10,2,0,1001);
 reply(t,11,1,8,1100);assert(!t.next(4000,true,cmd,sizeof(cmd)));
 reply(t,11,0,42,4100);assert(!t.next(6000,true,cmd,sizeof(cmd))&&!t.busy);
 assert(t.tsp[0]==42&&t.tsp[1]==8);
 // OT 4.2 §5.3.6: DATA-INVALID answers an undefined index; it completes the run but stores no value.
 assert(t.start(BoilerInfo::Parameters,10000));
 assert(t.next(10000,true,cmd,sizeof(cmd))&&!strcmp(cmd,"PM=10"));reply(t,10,3,0,10001);
 reply(t,11,0,7,10100);t.observe('R',frame(0,11,1),10200);t.observe('B',frame(6,11,1,0),10201);
 assert(t.busy);reply(t,11,2,255,10300);assert(!t.next(13000,true,cmd,sizeof(cmd))&&!t.busy);
 assert(t.tsp[2]==255&&(t.tspSeen[0]&2)==0&&(t.tspSeen[0]&5)==5); // 255 is a READ-ACK value, index 1 has none
 // Vendor=0, a master member ID, and spoofed answers never enable OEM reads.
 BoilerInfo u;reply(u,2,0,11);reply(u,3,0,0);u.bootReadStarted=true;
 assert(!u.oemAllowed()&&!u.start(BoilerInfo::OEM,0));
 u.observe('B',frame(4,3,0,11),100);assert(!u.remeha());
 u.observe('R',frame(0,3),100);u.observe('B',frame(4,3,0,11)^0x80000000,101);assert(!u.remeha());
 assert(!u.has(131));
 BoilerInfo wrap;reply(wrap,3,0,11);wrap.bootReadStarted=true;
 assert(wrap.start(BoilerInfo::Operating,0xfffffff0U));
 assert(wrap.next(0xfffffff0U,true,cmd,sizeof(cmd)));
 assert(!wrap.next(20,true,cmd,sizeof(cmd))); // rollover does not cause timeout
 // Manual OEM actions use only reads present in the observed vendor list.
 BoilerInfo o;reply(o,3,0,8);o.bootReadStarted=true;
 assert(o.oemAllowed()&&!o.oemReadable()&&!o.start(BoilerInfo::OEM,1000));
 reply(o,3,0,11);assert(o.start(BoilerInfo::OEM,1000));
 for(unsigned id=131;id<=133;++id){assert(o.next(1000+(id-131)*2000,true,cmd,sizeof(cmd)));char expected[12];snprintf(expected,sizeof(expected),"PM=%u",id);assert(!strcmp(cmd,expected));reply(o,id,0,1,1001+(id-131)*2000);}
 assert(!o.next(7000,true,cmd,sizeof(cmd))&&!o.busy);
 assert(!boilerOEMReadAllowed(173,210)&&boilerOEMReadAllowed(173,220));
 { BoilerInfo q;reply(q,3,0,2);assert(!q.oemAllowed()&&!q.oemReadable()); // member 2: memory dump, excluded
   for(unsigned id=128;id<=202;++id) assert(!boilerOEMReadAllowed(2,id)); }
 // Loss of transport cannot keep a job busy forever.
 assert(o.start(BoilerInfo::Operating,10000));
 assert(!o.next(370000,false,cmd,sizeof(cmd))&&!o.busy);
 BoilerInfo master;master.observe('T',frame(1,2,0,2),100);assert(master.has(2)&&(master.values[2]&255)==2&&!master.oemAllowed());
 puts("PASS: PIC PM sequencing, indexed text/TSP completion, correlation, timeout, support, vendor gating, rollover");
}
