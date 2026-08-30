Pebble.addEventListener('ready', function() {
  console.log('StrongLifts phone component ready');
});

Pebble.addEventListener('appmessage', function(event) {
  var message = event && event.payload ? event.payload : {};
  Pebble.sendAppMessage({ack: message.message || 'received'});
});
