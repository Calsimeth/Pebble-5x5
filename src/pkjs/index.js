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
function store(record) {
  var index = readIndex();
  if (index.ids.indexOf(String(record.id)) >= 0) return true;
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
if (typeof module !== 'undefined') module.exports = {readIndex: readIndex, store: store};
