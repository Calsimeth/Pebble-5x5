var assert = require('assert');
var data = {}; global.localStorage = {getItem:k=>data[k]||null,setItem:(k,v)=>{data[k]=v;}};
global.Pebble = {addEventListener:()=>{},sendAppMessage:()=>{}};
var sync = require('../src/pkjs/index.js');
assert(sync.store({schemaVersion:1,id:4,workout:0})); assert(sync.store({schemaVersion:1,id:4}));
assert.strictEqual(JSON.parse(data.historyIndex).ids.length,1); data.historyIndex='{bad';
assert(sync.store({schemaVersion:1,id:5})); assert(data['historyChunk:0001']);
console.log('sync js tests passed');
