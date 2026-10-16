const fs=require('fs'),vm=require('vm'),assert=require('assert');
const script=fs.readFileSync(process.argv[2] || 'src/OTGW-firmware/data/index.js','utf8');
class Node {
  constructor(){this.children=[];this.dataset={};this.attributes={};this._text='';this.disabled=false;this.classList={contains:()=>false};}
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
d={profile:'remeha-tzerra-m-28c-plus',value_115:0x031e,value_131:0x0806,value_132:2,value_133:0xff01,value_0:0,value_5:0,value_3:11,value_127:0x0d10,tsp_entries:'1:80,21:0',fhb_entries:'0:11',oem_allowed:true,oem_readable:true};
ctx.renderBoilerDetails(d);
function findText(n,text){return n.attributes['aria-label']===text || n.children.some(c=>findText(c,text));}
assert(findText(nodes.boilerOEM,'dF: 8 dU: 6'));
assert(!nodes.boilerOEM.textContent.includes('0x0806'));
assert(nodes.boilerOEM.textContent.includes('0x0002'));assert(!findText(nodes.boilerOEM,'B'));
assert(nodes.boilerOperating.textContent.includes('Branden op CV-bedrijf'));assert(nodes.boilerOperating.textContent.includes('Temperatuurregeling'));
assert(nodes.boilerOperating.textContent.includes('No active fault reported'));assert(!nodes.boilerOperating.textContent.includes('PSU'));
assert(nodes.boilerParameters.textContent.includes('Aanvoertemperatuur: TSet'));assert(findText(nodes.boilerParameters,'0'));
assert(nodes.boilerHistory.textContent.includes('Meaning not yet confirmed'));
// Live Tzerra M 28c Plus readout: 255 = unsupported is hidden, signed values and option texts shown.
const live='1:80,2:60,3:1,4:2,5:1,'+[6,7,8,9,10,11,12,13,14,15,16].map(i=>i+':255').join(',')+',17:15,18:56,21:23,22:255,24:35,27:241,30:246,32:20,39:255,44:2';
ctx.renderBoilerDetails({...d,tsp_entries:live});const tp=nodes.boilerParameters.textContent;
assert(!tp.includes('TSP 6')&&!tp.includes('TSP 16')&&!tp.includes('TSP 22')&&!tp.includes('Minimum waterdruk'));
assert(tp.includes('TSP 5')&&tp.includes('Pompnadraaitijd')&&tp.includes('Startvolumestroom')&&tp.includes('Setpuntverhoging boiler'));
assert(findText(nodes.boilerParameters,'-15')&&findText(nodes.boilerParameters,'-10'));
assert(tp.includes('TSP 39')&&findText(nodes.boilerParameters,'255'),'P39 0-255 s keeps a real 255');
assert(tp.includes('Ketelregeling / SWW: CV aan / SWW aan')&&tp.includes('Ontluchtingscyclus: Modulerende pomp'));
assert(tp.includes('fabrieksinstelling: 39'),'P17=15 differs from 28c default');assert(!/Aanvoertemperatuur: TSet \(°C\) · fabriek/.test(tp));
ctx.renderBoilerDetails({...d,value_115:0x090e});assert(nodes.boilerOperating.textContent.includes('Waterdruk te laag'));
ctx.renderBoilerDetails({...d,tsp_entries:'6:255,7:255'});assert(nodes.boilerParameters.textContent.includes('No supported parameters reported'));
ctx.renderBoilerDetails({...d,profile:'generic',tsp_entries:'1:80,6:255'});assert(nodes.boilerParameters.textContent.includes('TSP 6'),'OT 4.2: generic READ-ACK 255 is a valid value');
ctx.renderBoilerDetails({...d,value_131:0x2401});assert(findText(nodes.boilerOEM,'dF: 36 dU: 1'));assert(nodes.boilerOEM.textContent.includes('Tzerra M 28c Plus'));
ctx.renderBoilerDetails({...d,value_131:0x2501});assert(!nodes.boilerOEM.textContent.includes('Tzerra M 28c Plus'));
ctx.boilerRawFormat='binary';ctx.renderBoilerDetails(d);assert(nodes.boilerOEM.textContent.includes('0000000000000010'));
ctx.renderBoilerDetails({...d,profile:'generic'});assert(!nodes.boilerOperating.textContent.includes('Branden op CV-bedrijf'));assert(!findText(nodes.boilerOEM,'dF: 8 dU: 6'));
// Context-dependent substatus zero must use the status/substatus pair.
ctx.renderBoilerDetails({...d,value_115:0x0800});assert(nodes.boilerOperating.textContent.includes('Wacht op branderstart'));
console.log('PASS: partial/complete/safe text, profile matching, contextual labels, LCD notation including zero, raw fallback and service uncertainty');
