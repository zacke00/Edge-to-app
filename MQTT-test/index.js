const mqtt = require('mqtt');
const WebSocket = require('ws');

const MQTT_BROKER_URL = 'mqtt://127.0.0.1:1883';
const WEBSOCKET_PORT = 3002;

const mqttClient = mqtt.connect(MQTT_BROKER_URL);
const wsServer = new WebSocket.Server({ port: WEBSOCKET_PORT });

mqttClient.on('connect', () => {
    console.log('Connected to MQTT broker');
    mqttClient.subscribe('my/test/topic');
});

mqttClient.on('message', (topic, message) => {
    wsServer.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(message.toString());
        }
    });
});

wsServer.on('connection', (socket) => {
    console.log('WebSocket client connected');
});

console.log(`WebSocket server running on port ${WEBSOCKET_PORT}`);