const mqtt = require('mqtt');
const client = mqtt.connect('mqtt://localhost:1883');

const topic = 'my/test/topic';

client.on('connect', () => {
    console.log('Client connected to the MQTT server.');

    // Subscribe to a topic
    client.subscribe(topic, (err) => {
        if (err) {
            console.error('Failed to subscribe:', err);
        } else {
            console.log('Subscribed to:', topic);

            // Publish a message to the topic
            client.publish(topic, 'Hello, MQTT!', (err) => {
                if (err) {
                    console.error('Failed to publish:', err);
                } else {
                    console.log('Published message to:', topic);
                }
            });
        }
    });
});

// Handle received messages
client.on('message', (topic, message) => {
    console.log('Received message:', message.toString(), 'from topic:', topic);
});

// Handle client errors
client.on('error', (err) => {
    console.error('Client error:', err);
});