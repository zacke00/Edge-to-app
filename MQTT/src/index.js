const aedes = require('aedes')();
const server = require('net').createServer(aedes.handle);
const port = 1883;

server.listen(port, function() {
    console.log('MQTT server started and listening on port', port);
});

// Handle client connections
aedes.on('client', function(client) {
    console.log('Client Connected:', client.id);
});

// Handle client disconnections
aedes.on('clientDisconnect', function(client) {
    console.log('Client Disconnected:', client.id);
});

// Handle published messages
aedes.on('publish', function(packet, client) {
    if (client) {
        console.log('Message published by:', client.id);
    } else {
        console.log('Message published by broker');
    }
    console.log('Topic:', packet.topic);
    console.log('Payload:', packet.payload.toString());
});