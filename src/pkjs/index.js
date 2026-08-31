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
    return true;
  } catch (e) { console.log('history storage failed'); return false; }
}
Pebble.addEventListener('ready', function() { console.log('StrongLifts sync ready'); });
Pebble.addEventListener('appmessage', function(event) {
  var p = event && event.payload || {};
  if (!p.message) return;
  var record;
  try { record = JSON.parse(String(p.message)); } catch (e) { Pebble.sendAppMessage({ack: 0}); return; }
  if (record.v !== VERSION || !record.id || !Array.isArray(record.r)) { Pebble.sendAppMessage({ack: 0}); return; }
  if (store(record)) Pebble.sendAppMessage({ack: record.id});
});

// Exported for dependency-free tests under Node.
if (typeof module !== 'undefined') module.exports = {readIndex: readIndex, store: store, listChunkKeys:listChunkKeys, readChunk:readChunk, scanHistory:scanHistory, repairIndex:function(i,s){Object.keys(s.records).forEach(function(id){if(i.ids.indexOf(id)<0)i.ids.push(id);});i.nextChunk=Math.max(i.nextChunk||1,s.maxChunk+1);return i;}, recordsEqual:recordsEqual};
