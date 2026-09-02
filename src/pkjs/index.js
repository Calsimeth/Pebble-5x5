'use strict';

// Protocol v1: type 1=record, 2=ack, 3=nack. Records are JSON chunks kept
// separately so one corrupt entry cannot erase the rest of local history.
var VERSION = 1;
var INDEX_KEY = 'historyIndex';
var CHUNK_PREFIX = 'historyChunk:';

function readIndex() {
  try {
    var raw = localStorage.getItem(INDEX_KEY);
    var index = raw ? JSON.parse(raw) : {schemaVersion: VERSION, ids: [], nextChunk: 1};
    if (!index || index.schemaVersion !== VERSION || !Array.isArray(index.ids)) throw Error('index schema');
    return index;
  } catch (e) {
    console.log('history index invalid; preserving chunks');
    var recovered = {schemaVersion: VERSION, ids: [], nextChunk: 1, recovery: true};
    for (var i = 0; typeof localStorage.length === 'number' && i < localStorage.length; i++) {
      var key = localStorage.key(i);
      if (!key || key.indexOf(CHUNK_PREFIX) !== 0) continue;
      try { var chunk = JSON.parse(localStorage.getItem(key)); (chunk.records || []).forEach(function(r) { if (r && r.id && recovered.ids.indexOf(String(r.id)) < 0) recovered.ids.push(String(r.id)); }); var n = parseInt(key.slice(CHUNK_PREFIX.length), 10); if (n >= recovered.nextChunk) recovered.nextChunk = n + 1; } catch (ignore) { console.log('history chunk ignored'); }
    }
    return recovered;
  }
}
function listChunkKeys() { var out=[]; for(var i=0;i<(localStorage.length||0);i++){var k=localStorage.key(i);if(k&&k.indexOf(CHUNK_PREFIX)===0)out.push(k);} return out; }
function readChunk(key) { try { var c=JSON.parse(localStorage.getItem(key)); if(!c||!Array.isArray(c.records)) return null; return c; } catch(e) { console.log('history chunk ignored'); return null; } }
function scanHistory() { var ids={}, max=0, corrupt=[]; listChunkKeys().forEach(function(k){var c=readChunk(k);if(!c){corrupt.push(k);return;} var n=parseInt(k.slice(CHUNK_PREFIX.length),10);if(n>max)max=n;c.records.forEach(function(r){if(r&&r.id&&ids[String(r.id)]===undefined)ids[String(r.id)]=r;});}); return {records:ids,maxChunk:max,corrupt:corrupt}; }
function completedRecords() { var s=scanHistory(); return Object.keys(s.records).map(function(k){return s.records[k];}).filter(function(r){return r && r.c===1 && Number.isFinite(r.t);}).sort(function(a,b){return a.t-b.t||String(a.id).localeCompare(String(b.id));}); }
function dateOf(t) { var d=new Date(t*1000); return {y:d.getUTCFullYear(),m:d.getUTCMonth()+1,d:d.getUTCDate()}; }
function monthDays(year, month) { var n=new Date(Date.UTC(year,month,0)).getUTCDate(), mask=[],a=[],b=[]; for(var i=0;i<Math.ceil(n/8);i++)mask[i]=a[i]=b[i]=0; completedRecords().forEach(function(r){var d=dateOf(r.t), bit=d.d-1;if(d.y===year&&d.m===month&&d.d<=n){console.log('HISTORY_RECORD id='+r.id+' workout='+(r.w===1?'B':'A')+' utc='+d.y+'-'+d.m+'-'+d.d);mask[Math.floor(bit/8)]|=1<<(bit%8);(r.w===1?b:a)[Math.floor(bit/8)]|=1<<(bit%8);}}); return {year:year,month:month,days:n,mask:mask,maskA:a,maskB:b}; }
var EXERCISES=['Squat','Bench','Row','OHP','Deadlift'];
function progress(exercise,page) { if(exercise<0||exercise>4||page<0)return {points:[],page:page,total:0}; var pts=[]; completedRecords().forEach(function(r){var i=r.e.indexOf(exercise);if(i>=0&&Number.isFinite(r.wt[i]))pts.push({t:r.t,w:r.wt[i]});}); var total=Math.ceil(pts.length/20), start=Math.max(0,pts.length-(page+1)*20); return {exercise:exercise,page:page,total:total,points:pts.slice(start, start+20)}; }
function requestId(p) { return Number.isInteger(p)&&p>0&&p<=0xffff ? p : 0; }
function sendQuery(p) {
  if (!p || p.type !== 'calendar_request' && p.type !== 'progress_request' || !requestId(p.id)) return;
  var out;
  if(p.type==='calendar_request' && Number.isInteger(p.year)&&p.year>=2000&&p.year<=2100&&Number.isInteger(p.month)&&p.month>=1&&p.month<=12) { console.log('CALENDAR_REQUEST id='+p.id+' year='+p.year+' month='+p.month); var c=monthDays(p.year,p.month), pack=function(x){return ((x[0]||0)|(x[1]||0)<<8|(x[2]||0)<<16|(x[3]||0)<<24)>>>0;}, mask=pack(c.mask); console.log('HISTORY_SCAN valid='+completedRecords().length); console.log('CALENDAR_RESPONSE id='+p.id+' mask='+mask+' a='+pack(c.maskA)+' b='+pack(c.maskB)); out={calendar_id:p.id,calendar_year:c.year,calendar_month:c.month,calendar_days:c.days,calendar_mask:mask,calendar_mask_a:pack(c.maskA),calendar_mask_b:pack(c.maskB)}; }
  if(p.type==='progress_request' && Number.isInteger(p.exercise)&&p.exercise>=0&&p.exercise<5&&Number.isInteger(p.page)&&p.page>=0) { console.log('PROGRESS_REQUEST id='+p.id+' exercise='+p.exercise+' page='+p.page); var g=progress(p.exercise,p.page), chunks=Math.max(1,Math.ceil(g.points.length/5)); for(var ci=0;ci<chunks;ci++){var part=g.points.slice(ci*5,ci*5+5);console.log('PROGRESS_RESPONSE id='+p.id+' exercise='+g.exercise+' page='+g.page+' total='+g.points.length+' chunk='+ci+'/'+chunks+' points='+part.length);out={progress_id:p.id,progress_exercise:g.exercise,progress_page:g.page,progress_total:g.total,progress_chunk_index:ci,progress_chunk_count:chunks,progress_point_count:part.length};part.forEach(function(pt,j){out['progress_t'+j]=pt.t;out['progress_w'+j]=pt.w;});try{Pebble.sendAppMessage(out);}catch(e){} } return; }
  if(out) try { Pebble.sendAppMessage(out); } catch(e) {}
}
function recordsEqual(a,b) { return !!a&&!!b&&['v','id','t','w','c','d'].every(function(k){return a[k]===b[k];})&&JSON.stringify(a.e)===JSON.stringify(b.e)&&JSON.stringify(a.wt)===JSON.stringify(b.wt)&&JSON.stringify(a.r)===JSON.stringify(b.r); }
function store(record) {
  var expected = record && record.w === 1 ? [0,3,4] : [0,1,2];
  if (!record || record.v !== VERSION || !record.id || !Array.isArray(record.e) || record.e.length !== 3 || record.e.some(function(x,i){return x!==expected[i];}) || !Array.isArray(record.wt) || record.wt.length !== 3 || record.wt.some(function(x){return !Number.isInteger(x)||x<=0;}) || !Array.isArray(record.r) || (record.w !== 0 && record.w !== 1) || record.r.length !== (record.w ? 11 : 15) || record.r.some(function(x){return x<0||x>5||x%1;}) || record.c !== 1 || (record.d||0)>31) return false;
  var index = readIndex();
  var scan=scanHistory(); Object.keys(scan.records).forEach(function(id){if(index.ids.indexOf(id)<0)index.ids.push(id);}); index.nextChunk=Math.max(index.nextChunk||1,scan.maxChunk+1);
  if (scan.records[String(record.id)]) { if(!recordsEqual(scan.records[String(record.id)],record)) return false; try { localStorage.setItem(INDEX_KEY,JSON.stringify(index)); return true; } catch(e){return false;} }
  var number = index.nextChunk;
  var key;
  do { key = CHUNK_PREFIX + String(number++).padStart(4, '0'); } while (localStorage.getItem(key) !== null);
  index.nextChunk = number;
  try {
    localStorage.setItem(key, JSON.stringify({schemaVersion: VERSION, records: [record]}));
    index.ids.push(String(record.id));
    localStorage.setItem(INDEX_KEY, JSON.stringify(index));
    console.log('HISTORY_CHUNK_WRITTEN id=' + record.id + ' key=' + key + ' index=ok'); return true;
  } catch (e) { console.log('history storage failed'); return false; }
}
Pebble.addEventListener('ready', function() { console.log('StrongLifts sync ready'); });
Pebble.addEventListener('appmessage', function(event) {
  var p = event && event.payload || {};
  if (p.type === 'calendar_request' || p.type === 'progress_request') { sendQuery(p); return; }
  if (!p.message) return;
  var record;
  try { record = JSON.parse(String(p.message)); } catch (e) { console.log('HISTORY_RECORD_REJECTED reason=json'); Pebble.sendAppMessage({ack: 0}); return; }
  console.log('HISTORY_RECORD_RX id=' + (record.id || 0) + ' workout=' + (record.w === 1 ? 'B' : 'A') + ' timestamp=' + (record.t || 0) + ' reps=' + ((record.r && record.r.length) || 0));
  if (record.v !== VERSION || !record.id || !Array.isArray(record.r)) { console.log('HISTORY_RECORD_REJECTED id=' + (record.id || 0) + ' reason=shape'); Pebble.sendAppMessage({ack: 0}); return; }
  if (store(record)) { console.log('HISTORY_RECORD_VALID id=' + record.id); console.log('HISTORY_ACK_SENT id=' + record.id); Pebble.sendAppMessage({ack: record.id}); }
  else console.log('HISTORY_RECORD_REJECTED id=' + record.id + ' reason=validation_or_storage');
});

// Exported for dependency-free tests under Node.
if (typeof module !== 'undefined') module.exports = {readIndex: readIndex, store: store, listChunkKeys:listChunkKeys, readChunk:readChunk, scanHistory:scanHistory, completedRecords:completedRecords, monthDays:monthDays, progress:progress, sendQuery:sendQuery, requestId:requestId, exerciseNames:EXERCISES, repairIndex:function(i,s){Object.keys(s.records).forEach(function(id){if(i.ids.indexOf(id)<0)i.ids.push(id);});i.nextChunk=Math.max(i.nextChunk||1,s.maxChunk+1);return i;}, recordsEqual:recordsEqual};
