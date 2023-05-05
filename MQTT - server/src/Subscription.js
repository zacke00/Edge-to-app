const mqtt = require('mqtt');
const client = mqtt.connect('mqtt://localhost:1883');

const topics = ['my/test/topic', 'my/second/topic'];

client.on('connect', () => {
    console.log('Client connected to the MQTT server.');

    // Iterate over the topics array and subscribe to each topic
    topics.forEach((topic) => {
        client.subscribe(topic, (err) => {
            if (err) {
                console.error('Failed to subscribe:', err);
            } else {
                console.log('Subscribed to:', topic);

                // Publish a message to the topic
                client.publish(topic, `Hello, MQTT! This is a message for topic "${topic}"`, (err) => {
                    if (err) {
                        console.error('Failed to publish:', err);
                    } else {
                        console.log('Published message to:', topic);
                    }
                });
            }
        });
    });
});