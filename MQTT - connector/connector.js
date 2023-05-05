const mqtt = require('mqtt');
const WebSocket = require('ws');

const MQTT_BROKER_URL = 'mqtt://127.0.0.1:1883';
const WEBSOCKET_PORT = 3002;

const mqttClient = mqtt.connect(MQTT_BROKER_URL);
const wsServer = new WebSocket.Server({ port: WEBSOCKET_PORT });

mqttClient.on('connect', () => {
    console.log('Connected to MQTT broker');
});

wsServer.on('connection', (socket) => {
    console.log('WebSocket client connected');

    const topic = 'controller/plantation-one/topic';
    mqttClient.subscribe(topic);

    mqttClient.on('message', (mqttTopic, mqttMessage) => {
        if (mqttTopic === topic) {
            socket.send(mqttMessage.toString());
        }
    });

    socket.on('close', () => {
        console.log('WebSocket client disconnected');
    });
});

console.log(`WebSocket server running on port ${WEBSOCKET_PORT}`);