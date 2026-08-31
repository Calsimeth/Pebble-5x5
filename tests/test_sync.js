var assert = require('assert');
var data = {}; global.localStorage = {get length(){return Object.keys(data).length;}, key:i=>Object.keys(data)[i]||null, getItem:k=>data[k]||null,setItem:(k,v)=>{data[k]=v;}};
global.Pebble = {addEventListener:()=>{},sendAppMessage:()=>{}};
var sync = require('../src/pkjs/index.js');
var valid = {v:1,id:4,t:1,w:0,e:[0,1,2],wt:[180,180,260],r:[5,5,5,5,5,5,5,5,5,5,5,5,5,5,5],c:1,d:0};
assert(sync.store(valid)); assert(sync.store(valid));
assert.strictEqual(JSON.parse(data.historyIndex).ids.length,1); data.historyIndex='{bad';
assert(sync.store(Object.assign({}, valid, {id:5}))); assert(Object.keys(data).some(function(k){return k.indexOf('historyChunk:')===0;}));
console.log('sync js tests passed');
