#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "Adafruit_LTR329_LTR303.h"
#include <Adafruit_DotStar.h>
#include <PubSubClient.h>

//------------------------------------------------------------
//SHT31 humidity, temperature and light sensor
bool enableHeater = false;

uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_LTR329 ltr = Adafruit_LTR329();
//------------------------------------------------------------

//------------------------------------------------------------
//Wifi
const char * home = "Telenor2437tal";
const char * homepass = "pgvfheslvafvk";

const char * ssid = "testingwifi";    //  <--- Change to connect to Wi-Fi
const char * password = "12345abc";   //  <--- Change to connect to Wi-Fi
//------------------------------------------------------------

//------------------------------------------------------------
//connection to API

const char * api_host = "192.168.81.194"; //    <------ Change to send to API
const int api_port = 3000;
const char* api_endpoint_danger = "/DANGER";
const char* api_endpoint_readings = "/Reading";
const char* PublicName = "plantation-one"; //  <- insert name here
 // <--- change this topic for new MQTT connection with a new controller
//api data
DynamicJsonDocument doc(1024);
String payload;

//------------------------------------------------------------

//------------------------------------------------------------
//MQTT
WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_server = "192.168.81.194"; //   <------ Change to send to MQTT (same as api_host. i could change the connection but it would look messy)
const int mqtt_port = 1883;


//------------------------------------------------------------

//------------------------------------------------------------
// DotStar
#define NUMPIXELS 1
#define DATAPIN    33
#define CLOCKPIN   21

Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_RGB);

//------------------------------------------------------------

//------------------------------------------------------------
//variables
  bool DangerousReading = false;
  int SafeTempTop = 33;
  int SafeTempLow = 28;
  int SafeHumTop = 58;
  int SafeHumLow = 48;

unsigned long lastApiCall = 0;
unsigned long lastReading = 0;
const unsigned long apiInterval = 60000; // 1 minute
  const unsigned long readingInterval = 1000; // 1 second
//------------------------------------------------------------

//------------------------------------------------------------
// LED pin
const int LED_PIN = LED_BUILTIN;
//------------------------------------------------------------


//------------------------------------------------------------
//connecting to wifi
void connectToWiFi(const char * ssid, const char * pwd)
{
  Serial.println("Connecting to WiFi network: " + String(ssid));
  WiFi.begin(ssid, pwd); // start connecting to the wifi network

  while (WiFi.status() != WL_CONNECTED) 
  {
    // Blink LED while we're connecting:
    digitalWrite( LED_PIN, !digitalRead(LED_PIN) );
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
//------------------------------------------------------------

//------------------------------------------------------------
// send data to API
void sendPayloadToAPI(String payload, String endpoint) {
  // Create a WiFiClient object to establish a TCP connection to the API server
  WiFiClient client;

  // Connect to the API server
  if (!client.connect(api_host, api_port)) {
    Serial.println("Error connecting to API server");
    return;
  }

  // TCP call to the API server
  client.print(String("POST ") + endpoint + " HTTP/1.1\r\n" +
               "Host: " + api_host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + payload.length() + "\r\n" +
               "Connection: close\r\n\r\n" +
               payload + "\r\n");

  // Wait for the server to respond
  while (!client.available()) {
    delay(10);
  }

  // Read the response from the server and print it to the serial monitor
  String response = "";
  while (client.available()) {
    response += client.readString();
  }
  Serial.println("Payload:");
  Serial.println(payload);
  Serial.println("Response:");
  Serial.println(response);

  // Disconnect from the API server
  client.stop();
}
//------------------------------------------------------------
//------------------------------------------------------------
//MQTT

void connectToMQTT() {
  client.setServer(mqtt_server, mqtt_port);

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//------------------------------------------------------------
//setup

void setup() {
 Serial.begin(115200);
   if ( ! ltr.begin() ) {
    Serial.println("Couldn't find LTR sensor!");
    while (1) delay(10);
  }
  Serial.println("Found LTR sensor!");
  strip.begin();
  strip.setBrightness(20);
  strip.show();
  ltr.setGain(LTR3XX_GAIN_2);
  ltr.setIntegrationTime(LTR3XX_INTEGTIME_100);
  ltr.setMeasurementRate(LTR3XX_MEASRATE_200);

  
 //Wifi connection
  connectToWiFi(ssid, password);
  connectToMQTT();
//SHT31
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
//------------------------------------------------------------

}
//------------------------------------------------------------


//------------------------------------------------------------
//danger reading
void Danger(int TempReading, int HumReading, int LightReading) {
  Serial.println("Danger");

  doc["Name"] = PublicName;
  doc["Temperature"] = TempReading;
  doc["Humidity"] = HumReading;
  doc["Light"] = LightReading;
  
  serializeJson(doc, payload);
  sendPayloadToAPI(payload, api_endpoint_danger);
  payload="";
  
}
//------------------------------------------------------------


//------------------------------------------------------------
//safe reading
void safeReading(int TempReading, int HumReading, int LightReading, String Status) {
  Serial.println("Safe reading");
  doc["Name"] = PublicName;
  doc["Temperature"] = TempReading;
  doc["Humidity"] = HumReading;
  doc["Light"] = LightReading;
  doc["Safety"] = Status;
  
  serializeJson(doc, payload);
  sendPayloadToAPI(payload, api_endpoint_readings);
  payload="";
}
//------------------------------------------------------------


//------------------------------------------------------------
//Init for readings
void readings() {
  int Temperature_t = sht31.readTemperature();
  int Humidity_h = sht31.readHumidity();
  uint16_t visible_plus_ir, infrared;
  bool valid;
  
      valid = ltr.readBothChannels(visible_plus_ir, infrared);
  if (!valid) {
    Serial.println("Invalid data");
  }

  if (client.connected()) {
    String topic = "controller/" + String(PublicName) + "/topic";
    String message = "{\"name\":\"" + String(PublicName) + "\",\"temperature\":" + String(Temperature_t) + ",\"humidity\":" + String(Humidity_h) + ",\"light\":" + String(visible_plus_ir) + "}";
    client.publish(topic.c_str(), message.c_str());
  } else {
    connectToMQTT();
  }
}
//------------------------------------------------------------

//------------------------------------------------------------
//send data to api
void sendDataToApi(){
  int Temperature_t = sht31.readTemperature();
  int Humidity_h = sht31.readHumidity();
  uint16_t visible_plus_ir, infrared;
  bool valid;

    valid = ltr.readBothChannels(visible_plus_ir, infrared);
  if (valid) {
    Serial.print("Visible + IR: "); Serial.println(visible_plus_ir);
    Serial.print("Visible: "); Serial.println(visible_plus_ir - infrared);
  } else {
    Serial.println("Invalid data");
  }



  // Check if the reading is dangerous
  if (Temperature_t > SafeTempTop || (Temperature_t <= SafeTempLow && (Humidity_h > SafeHumTop || Humidity_h < SafeHumLow))) {
    DangerousReading = true;
    strip.setPixelColor(0, 0, 0, 255);
    strip.show();

  } else {
    DangerousReading = false;
  }


  if(DangerousReading == true){
    Danger(Temperature_t, Humidity_h, visible_plus_ir);
  } else if (Temperature_t > SafeTempTop || Temperature_t <= SafeTempLow || Humidity_h > SafeHumTop || Humidity_h < SafeHumLow)
  {
    strip.setPixelColor(0, 0, 255, 255);
    strip.show();
    safeReading(Temperature_t, Humidity_h, visible_plus_ir, "Warning");
  }else 
  {
    strip.setPixelColor(0, 0, 255, 0);
    strip.show();
    safeReading(Temperature_t, Humidity_h, visible_plus_ir, "Safe");
  }

}

//------------------------------------------------------------
//------------------------------------------------------------
//loop

void loop() {
  delay(100);
  unsigned long currentMillis = millis();
client.loop();


    if (currentMillis - lastReading >= readingInterval) {
    readings(); // Send readings to MQTT server
    lastReading = currentMillis;
  }


    if (currentMillis - lastApiCall >= apiInterval) {
      sendDataToApi();
    lastApiCall = currentMillis;
  }

}
