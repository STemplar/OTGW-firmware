const fs=require('fs'),vm=require('vm'),assert=require('assert');
const script=fs.readFileSync(process.argv[2] || 'src/OTGW-firmware/data/index.js','utf8');
class Node {
  constructor(){this.children=[];this.dataset={};this.attributes={};this.style={};this._text='';this.disabled=false;this.className='';}
  get classList(){const n=this,list=()=>n.className.split(' ').filter(Boolean);return{contains:c=>list().includes(c),
    toggle:(c,f)=>{const s=new Set(list()),on=f===undefined?!s.has(c):!!f;if(on)s.add(c);else s.delete(c);n.className=[...s].join(' ');return on;}};}
  set textContent(s){this._text=String(s);this.children=[];} get textContent(){return this._text+this.children.map(n=>n.textContent).join('');}
  appendChild(n){this.children.push(n);return n;} setAttribute(k,v){this.attributes[k]=v;}
}
const nodes={};const document={getElementById:id=>nodes[id]||(nodes[id]=new Node()),createElement:()=>new Node(),querySelectorAll:()=>[],addEventListener:()=>{}};
const ctx={document,console,fetch:()=>Promise.reject(new Error('fixture prevents network')),Number,Array};
vm.createContext(ctx);
vm.runInContext(script.slice(script.indexOf('function formatBoilerText('),script.indexOf('function startBoilerInfoPolling()')),ctx);
vm.runInContext(script.slice(script.indexOf('// Boiler values and manufacturer descriptions')),ctx);
assert.equal(ctx.formatBoilerText({brand_support:'unknown'},'brand').status,'Support not yet determined');
assert.equal(ctx.formatBoilerText({brand_support:'unsupported'},'brand').value,'Not supported by this boiler');
let d={brand_support:'supported',brand_text:'Br   ',brand_mask:'11000',brand_received:2,brand_expected:5,brand_complete:false};
assert.equal(ctx.formatBoilerText(d,'brand').value,'Br···');assert(ctx.formatBoilerText(d,'brand').status.includes('2/5'));
d.brand_text='<img src=x>';d.brand_mask='1'.repeat(11);d.brand_received=11;d.brand_expected=11;d.brand_complete=true;
ctx.renderBoilerInfo(d);assert.equal(nodes['boiler-brand-value'].textContent,'<img src=x>');assert.equal(nodes['boiler-brand-status'].textContent,'');
assert.equal(ctx.formatBoilerText({serial_text:'A'.repeat(50),serial_mask:'1'.repeat(50),serial_received:50,serial_expected:50,serial_complete:true},'serial').value.length,50);
ctx.boilerProfileData=JSON.parse(fs.readFileSync('src/OTGW-firmware/data/tzerra-m-28c-plus.json','utf8'));
d={profile:'remeha-tzerra-m-28c-plus',profile_file:'tzerra-m-28c-plus.json',value_115:0x031e,value_131:0x0806,value_132:2,value_133:0xff01,value_0:0,value_5:0,value_3:11,value_127:0x0d10,tsp_entries:'1:80,21:0',fhb_entries:'0:11',oem_allowed:true,oem_readable:true};
ctx.renderBoilerDetails(d);
function findText(n,text){return n.textContent.includes(text);}
// Rows of a block; parameter rows sit in three column blocks (.boiler-col), read top to bottom per column.
function rowNodes(n){return n.children.flatMap(c=>c.className==='boiler-col'?c.children:[c]);}
// One string per row: label|value (the visible cells); the meaning is the third cell (tooltip / tap).
function rowsOf(n){return rowNodes(n).map(r=>r.children.slice(0,2).map(c=>c.textContent).join('|'));}
function hasRow(n,text){return rowsOf(n).includes(text);}
function rowNode(n,text){return rowNodes(n).find(r=>r.children.slice(0,2).map(c=>c.textContent).join('|')===text);}
function meaningOf(n,text){const r=rowNode(n,text);return r?r.children[2].textContent:null;}
assert(findText(nodes.boilerOEM,'dF: 8 dU: 6'));
assert(!nodes.boilerOEM.textContent.includes('0x0806'));
assert(nodes.boilerOEM.textContent.includes('0x0002'));
assert(nodes.boilerOperating.textContent.includes('Branden op CV-bedrijf'));assert(nodes.boilerOperating.textContent.includes('Temperatuurregeling'));
assert(nodes.boilerOperating.textContent.includes('No active fault reported'));assert(!nodes.boilerOperating.textContent.includes('PSU'));
assert(nodes.boilerParameters.textContent.includes('Aanvoertemperatuur: TSet'));assert(findText(nodes.boilerParameters,'0'));
assert(nodes.boilerHistory.textContent.includes('Meaning not yet confirmed'));
// Live Tzerra M 28c Plus readout: 255 = unsupported is hidden, signed values and option texts shown.
const live='1:80,2:60,3:1,4:2,5:1,'+[6,7,8,9,10,11,12,13,14,15,16].map(i=>i+':255').join(',')+',17:15,18:56,21:23,22:255,24:35,27:241,30:246,32:20,39:255,44:2';
ctx.renderBoilerDetails({...d,tsp_entries:live});const tp=nodes.boilerParameters.textContent;const P=nodes.boilerParameters;
assert(!tp.includes('TSP ')&&!rowsOf(P).some(r=>/^P(6|16|22) /.test(r))&&!tp.includes('Minimum waterdruk'));
assert(hasRow(P,'P5 Pompnadraaitijd|1 min')&&tp.includes('Startvolumestroom')&&tp.includes('Setpuntverhoging boiler'));
assert(findText(nodes.boilerParameters,'-15')&&findText(nodes.boilerParameters,'-10'));
assert(hasRow(P,'P39 Rookgasklep looptijd|255 s')&&meaningOf(P,'P39 Rookgasklep looptijd|255 s')==='Instelbereik: 0 tot 255 seconden\nFabrieksinstelling 28c Plus: 0','P39 0-255 s keeps a real 255');
// Tooltip as in manual Tab.34: setting range (one line per option) and factory setting; the cell shows the value only.
assert(hasRow(P,'P3 Ketelregeling / SWW|1')&&meaningOf(P,'P3 Ketelregeling / SWW|1')==='Instelbereik:\n0 = CV uit / SWW uit\n1 = CV aan / SWW aan\n2 = CV aan / SWW uit\n3 = CV uit / SWW aan\nFabrieksinstelling 28c Plus: 1');
assert(hasRow(P,'P44 Ontluchtingscyclus|2')&&meaningOf(P,'P44 Ontluchtingscyclus|2').includes('2 = Modulerende pomp'));
const p17='P17 Maximum volumestroom van de ventilator (CV)|15';
assert(hasRow(P,p17)&&meaningOf(P,p17)==='Fabrieksinstelling 28c Plus per gassoort:\nG25 (L-gas): 39\nG20 (H-gas): 39\nG25.3 (K-gas): 39\nG31 (Propaan): 39','P17=15 differs from 28c default (G25 base)');
assert(rowNode(P,p17).classList.contains('boiler-changed')&&rowNode(P,p17).classList.contains('has-meaning'));
assert(rowNode(P,p17).children[0].children[0].className==='boiler-pnum'&&rowNode(P,p17).children[0].children[0].textContent==='P17','P-number in a fixed-width part');
assert(rowNode(P,p17).title.startsWith('P17 Maximum volumestroom van de ventilator (CV)\nFabrieksinstelling 28c Plus per gassoort:'),'tooltip: full label and meaning');
assert(meaningOf(P,'P21 Startvolumestroom (niet wijzigen)|23')==='Fabrieksinstelling 28c Plus: G25 (L-gas): 23');
assert(meaningOf(P,'P27 Instelpunt van de stooklijn (minimum buitentemperatuur)|-15 °C')==='Instelbereik: -30 tot 0 °C (alleen met buitensensor)\nFabrieksinstelling 28c Plus: -15','footnote in parentheses');
assert(hasRow(P,'P1 Aanvoertemperatuur: TSet|80 °C')&&!rowNode(P,'P1 Aanvoertemperatuur: TSet|80 °C').classList.contains('boiler-changed'));
// One wide column: rows directly in the block, in P-number order.
assert(P.children.every(r=>r.className.startsWith('otmonrow boiler-row')));
assert.deepEqual(rowsOf(P).map(r=>Number(r.match(/^P(\d+)/)[1])),rowsOf(P).map(r=>Number(r.match(/^P(\d+)/)[1])).slice().sort((a,b)=>a-b));
ctx.renderBoilerDetails({...d,value_115:0x090e});assert(nodes.boilerOperating.textContent.includes('Waterdruk te laag'));
// With a profile the 32 manual parameters are fixed cells; unread cells are empty, never highlighted.
ctx.renderBoilerDetails({...d,tsp_entries:'6:255,7:255'});const E=nodes.boilerParameters;
assert(E.children.length===32);assert(rowNodes(E).every(r=>r.children[1].textContent==='---'&&!r.classList.contains('boiler-changed')));
assert(hasRow(E,'P17 Maximum volumestroom van de ventilator (CV)|---')&&meaningOf(E,'P17 Maximum volumestroom van de ventilator (CV)|---').startsWith('Fabrieksinstelling 28c Plus per gassoort:\nG25 (L-gas): 39'));
// Cells keep their position while values arrive: the same label sits in the same column/row before and after.
const pos=n=>rowNodes(n).map(r=>r.children[0].textContent);const before=pos(E);
ctx.renderBoilerDetails({...d,tsp_entries:'1:80,17:15'});assert.deepEqual(pos(nodes.boilerParameters),before);
ctx.renderBoilerDetails({...d,tsp_entries:'',support_11:2});assert(nodes.boilerParameters.children.length===0,'ID11 UNKNOWN-DATAID: no fixed cells, unsupported row omitted');
ctx.renderBoilerDetails({...d,profile:'generic',tsp_entries:'1:80,6:255'});assert(nodes.boilerParameters.textContent.includes('TSP 6'),'OT 4.2: generic READ-ACK 255 is a valid value');
ctx.renderBoilerDetails({...d,value_131:0x2401});assert(findText(nodes.boilerOEM,'dF: 36 dU: 1'));assert(nodes.boilerOEM.textContent.includes('Tzerra M 28c Plus'));
ctx.renderBoilerDetails({...d,value_131:0x2501});assert(!nodes.boilerOEM.textContent.includes('Tzerra M 28c Plus'));
// Tzerra Ace Matic: Tab.54/55 status texts, flat substatus list, raw ID5, no dF/dU block.
const tz=ctx.boilerProfileData;ctx.boilerProfileData=JSON.parse(fs.readFileSync('src/OTGW-firmware/data/tzerra-ace-matic.json','utf8'));
ctx.renderBoilerDetails({profile:'remeha-tzerra-ace-matic',profile_file:'tzerra-ace-matic.json',value_115:0x031e,value_0:1,value_5:0x0001,value_127:0x1b01,value_131:0x2401,oem_allowed:true});
assert(nodes.boilerOperating.textContent.includes('Bedrijf in de verwarmingsmodus')&&nodes.boilerOperating.textContent.includes('Werking op de ingestelde richttemperatuur'));
assert(!nodes.boilerOperating.textContent.includes('E:01')&&!findText(nodes.boilerOEM,'dF: 36 dU: 1')&&nodes.boilerOEM.textContent.includes('No OEM fields documented'));
ctx.renderBoilerDetails({profile:'remeha-tzerra-ace-matic',value_115:0x115f});assert(nodes.boilerOperating.textContent.includes('waterdruk voldoende'));
ctx.renderBoilerDetails({...d,profile:'remeha-tzerra-ace-matic'});assert(!nodes.boilerParameters.textContent.includes('Aanvoertemperatuur'));
ctx.boilerProfileData=tz;
// Manual view: P-number + description, value with unit/option; no TSP label, no 7-segment, unsupported rows omitted.
ctx.renderBoilerDetails({...d,tsp_entries:'1:80,3:1,17:15,27:241'});const pv=nodes.boilerParameters.textContent;
const Q=nodes.boilerParameters;assert(hasRow(Q,'P1 Aanvoertemperatuur: TSet|80 °C')&&!pv.includes('TSP 1'));
assert(hasRow(Q,'P3 Ketelregeling / SWW|1')&&hasRow(Q,'P27 Instelpunt van de stooklijn (minimum buitentemperatuur)|-15 °C'));
// Status and substatus keep their short meaning in the cell; the OEM meaning (model name) is the tooltip.
ctx.renderBoilerDetails({...d,value_115:0x0000,value_131:0x2401});
assert(hasRow(nodes.boilerOperating,'Status|0 · Rust')&&hasRow(nodes.boilerOperating,'Substatus|0 · Rust'));
assert(hasRow(nodes.boilerOEM,'Configuration|dF: 36 dU: 1')&&meaningOf(nodes.boilerOEM,'Configuration|dF: 36 dU: 1')==='Tzerra M 28c Plus');
// Rows use the OT monitor row style; a row without meaning is not tappable.
ctx.renderBoilerDetails({...d,profile:'generic',value_127:0x0d10});
assert(nodes.boilerIdentityDetails.children.every(r=>r.className==='otmonrow boiler-row'));
assert(rowNodes(nodes.boilerParameters).every(r=>r.className.startsWith('otmonrow boiler-row')));
// Ventilation and solar member IDs are not shown, even when observed.
ctx.renderBoilerDetails({...d,value_74:0x0005,value_103:0x0007});
assert(!nodes.boilerIdentityDetails.textContent.includes('Ventilation')&&!nodes.boilerIdentityDetails.textContent.includes('Solar'));
// Fault history: hidden when the model profile says the boiler has none; shown otherwise.
ctx.renderBoilerDetails(d);assert(nodes.boilerHistoryBlock.hidden===true&&nodes.boilerReadHistory.hidden===true);
ctx.renderBoilerDetails({...d,profile:'generic'});assert(nodes.boilerHistoryBlock.hidden===false&&nodes.boilerReadHistory.hidden===false);
ctx.renderBoilerDetails({...d,value_131:undefined,support_131:2});assert(!nodes.boilerOEM.textContent.includes('Not supported'));
ctx.renderBoilerInfo({...d,brand_support:'unsupported'});assert(nodes['boiler-brand-row'].style.display==='none');
ctx.renderBoilerInfo({...d,brand_support:'supported',brand_text:'Remeha',brand_mask:'111111',brand_received:6,brand_expected:6,brand_complete:true});assert(nodes['boiler-brand-row'].style.display==='');
ctx.renderBoilerDetails(d);assert(nodes.boilerOEM.textContent.includes('0x0002')); // raw values are always hex (toggle removed)
ctx.renderBoilerDetails({...d,profile:'generic'});assert(!nodes.boilerOperating.textContent.includes('Branden op CV-bedrijf'));assert(!findText(nodes.boilerOEM,'dF: 8 dU: 6'));
// Context-dependent substatus zero must use the status/substatus pair.
ctx.renderBoilerDetails({...d,value_115:0x0800});assert(nodes.boilerOperating.textContent.includes('Wacht op branderstart'));
// ID132 (TASK-1244): only 0x0101-0x0103 are service message A/B/C; any other value stays raw, no meaning.
[[0x0101,'A'],[0x0102,'B'],[0x0103,'C']].forEach(([v,l])=>{ctx.renderBoilerDetails({...d,value_132:v});
  assert(hasRow(nodes.boilerOEM,'Service message|'+l)&&meaningOf(nodes.boilerOEM,'Service message|'+l)==='');});
[[0x0104,'0x0104'],[0x0000,'0x0000'],[0x0001,'0x0001']].forEach(([v,raw])=>{ctx.renderBoilerDetails({...d,value_132:v});
  assert(hasRow(nodes.boilerOEM,'Service message|'+raw)&&!nodes.boilerOEM.textContent.includes('not yet confirmed'));});
{const sm=tz.service_messages;delete tz.service_messages;ctx.renderBoilerDetails({...d,value_132:0x0101}); // profile without the key: as before
  assert(hasRow(nodes.boilerOEM,'Service message|0x0101')&&meaningOf(nodes.boilerOEM,'Service message|0x0101').includes('not yet confirmed'));tz.service_messages=sm;}
// Generic view: once ID10 is known, TSP 0..count-1 are fixed cells; values fill in raw (255 included).
ctx.renderBoilerDetails({...d,profile:'generic',value_10:45<<8,tsp_entries:''});const G=nodes.boilerParameters;
assert(G.children.length===45);assert(rowNodes(G).every(r=>r.children[1].textContent==='---'));
const gpos=rowNodes(G).map(r=>r.children[0].textContent);assert(gpos[0]==='TSP 0'&&gpos[44]==='TSP 44');
ctx.renderBoilerDetails({...d,profile:'generic',value_10:45<<8,tsp_entries:'1:80,6:255'});
assert.deepEqual(rowNodes(nodes.boilerParameters).map(r=>r.children[0].textContent),gpos,'cells do not move');
assert(hasRow(nodes.boilerParameters,'TSP 6|255')&&hasRow(nodes.boilerParameters,'TSP 1|80')&&hasRow(nodes.boilerParameters,'TSP 0|---'));
// Nothing received yet: '---' (identification, operating status, OEM, text fields).
ctx.renderBoilerDetails({profile:'generic',oem_allowed:false});
assert(hasRow(nodes.boilerIdentityDetails,'Slave member ID|---')&&hasRow(nodes.boilerOperating,'Diagnostic status|---'));
assert.equal(ctx.formatBoilerText({brand_support:'unknown'},'brand').value,'---');
// Member ID names from the OTA register (TASK-1245): known code shows the company; 0 and unassigned codes only the number.
ctx.boilerMemberNames=JSON.parse(fs.readFileSync('src/OTGW-firmware/data/member-ids.json','utf8'));
ctx.renderBoilerDetails({...d,value_3:0x410B,value_2:140});
assert(hasRow(nodes.boilerIdentityDetails,'Slave member ID|11 · BDR Thermea Group B.V.')&&hasRow(nodes.boilerIdentityDetails,'Master member ID|140 · Plugwise B.V.'));
assert(meaningOf(nodes.boilerIdentityDetails,'Master member ID|140 · Plugwise B.V.')==='Plugwise B.V.','full company name in the tooltip');
assert(meaningOf(nodes.boilerIdentityDetails,'Slave member ID|11 · BDR Thermea Group B.V.')==='BDR Thermea Group B.V. · merken: Baxi, De Dietrich, Remeha, Brötje, Chappée, Baymak','brands where a source is recorded');
ctx.renderBoilerDetails({...d,value_3:0,value_2:211});assert(hasRow(nodes.boilerIdentityDetails,'Slave member ID|0')&&hasRow(nodes.boilerIdentityDetails,'Master member ID|211'));
ctx.boilerMemberNames=null;ctx.renderBoilerDetails({...d,value_3:11});assert(hasRow(nodes.boilerIdentityDetails,'Slave member ID|11'),'without the file only numbers');
// P19/P20: per gas type plus CLV overpressure (Tab.34-41); highlight compares with G25 (L-gas).
ctx.boilerProfileData=tz;ctx.renderBoilerDetails({...d,tsp_entries:'19:14,20:80'});
assert(meaningOf(nodes.boilerParameters,'P19 Minimum volumestroom van de ventilator (CV + SWW)|14')==='Fabrieksinstelling 28c Plus per gassoort:\nG25 (L-gas): 11\nG20 (H-gas): 11\nG25.3 (K-gas): 11\nG31 (Propaan): 14\nBij CLV-overdruk: G20 15, G25 15, G25.3 15, G31 15');
assert(rowNode(nodes.boilerParameters,'P19 Minimum volumestroom van de ventilator (CV + SWW)|14').classList.contains('boiler-changed')&&!rowNode(nodes.boilerParameters,'P20 Minimum volumestroom van de ventilator (offset)|80').classList.contains('boiler-changed'));
console.log('PASS: partial/complete/safe text, profile matching, contextual labels, manual-style parameter rows, unsupported rows omitted, raw fallback and service uncertainty');
