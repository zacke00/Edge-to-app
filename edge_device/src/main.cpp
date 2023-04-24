#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "Adafruit_LTR329_LTR303.h"
#include <time.h>

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

const char * ssid = "Student";
const char * password = "Kristiania1914";
//------------------------------------------------------------

//------------------------------------------------------------
//connection to API
const char * api_host = "172.26.106.75";
const int api_port = 3000;
const char* api_endpoint_danger = "/DANGER";
const char* api_endpoint_readings = "/Reading";
const char* PublicName = "plantation-one"; //  <- insert name here
//api data
DynamicJsonDocument doc(1024);
String payload;

//------------------------------------------------------------
//------------------------------------------------------------
//variables
  bool DangerousReading = false;
  
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
//setup

void setup() {
 Serial.begin(115200);
   if ( ! ltr.begin() ) {
    Serial.println("Couldn't find LTR sensor!");
    while (1) delay(10);
  }
  Serial.println("Found LTR sensor!");

  ltr.setGain(LTR3XX_GAIN_2);
  ltr.setIntegrationTime(LTR3XX_INTEGTIME_100);
  ltr.setMeasurementRate(LTR3XX_MEASRATE_200);

  
 //Wifi connection
  connectToWiFi(ssid, password);
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
  if (valid) {
    Serial.print("Visible + IR: "); Serial.println(visible_plus_ir);
    Serial.print("Visible: "); Serial.println(visible_plus_ir - infrared);
  } else {
    Serial.println("Invalid data");
  }



  // Check if the reading is dangerous
  if (Temperature_t > 33 || (Temperature_t <= 29 && (Humidity_h > 58 || Humidity_h < 48))) {
    DangerousReading = true;
  } else {
    DangerousReading = false;
  }
  Serial.println("DangroudReading = " + bool(DangerousReading));

  if(DangerousReading == true){
    Danger(Temperature_t, Humidity_h, visible_plus_ir);
  } else if (Temperature_t > 33 || Temperature_t <= 29 || Humidity_h > 58 || Humidity_h < 48)
  {
    safeReading(Temperature_t, Humidity_h, visible_plus_ir, "Warning");
  }else 
  {
    safeReading(Temperature_t, Humidity_h, visible_plus_ir, "Safe");
  }
}

//------------------------------------------------------------



//------------------------------------------------------------
//loop

void loop() {
  readings();

  delay(10000);

  // Toggle heater enabled state every 30 seconds
  // An ~3.0 degC temperature increase can be noted when heater is enabled
  if (loopCnt >= 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");

    loopCnt = 0;
  }
  loopCnt++;
}
