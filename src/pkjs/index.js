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
    return {schemaVersion: VERSION, ids: [], nextChunk: 1, recovery: true};
  }
}
function store(record) {
  var index = readIndex();
  if (index.ids.indexOf(String(record.id)) >= 0) return true;
  var key = CHUNK_PREFIX + String(index.nextChunk++).padStart(4, '0');
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
  if (record.schemaVersion !== VERSION || !record.id) { Pebble.sendAppMessage({ack: 0}); return; }
  if (store(record)) Pebble.sendAppMessage({ack: record.id});
});

// Exported for dependency-free tests under Node.
if (typeof module !== 'undefined') module.exports = {readIndex: readIndex, store: store};
