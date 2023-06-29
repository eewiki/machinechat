/*
 WiFi/MQTT enabled relay example implemented on DFRobot Beetle ESP32C3
 filename: BeetleESP32cMqttRelay.ino
 project hardware: DFRobot Beetle ESP32C3 board and Seeed Grove Relay board
 project function: connects to the Machinechat MQTT broker and subscribes to multiple topics (including a virtual button with two states "true" and "false").
 The virtual button topic is monitored and used to control the state of the relay


*/

#include "WiFi.h"
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 

// Update these with values suitable for your network.
char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// MQTT server info for JEDI Pro on Odyssey Ubuntu machine
const char* mqttServer = "192.168.1.23";
const int mqttPort = 1883;

StaticJsonDocument<256> doc;

int led = 10;

int data1 = 5;  //data1 of MQTT json message
int data2 = 5;  //data2 of MQTT json message
int data3 = 5;  //data3 of MQTT json message
int data4 = 5;  //data3 of MQTT json message
bool button = true;  //boolean variable of MQTT message
int msg = 0;
const char* timestamp = "dummy data";  //the is the MQTT message timestamp (this works)
String recv_payload;
const char* subtopic1 = "datacache/Button1boolean";                //test button control
const char* subtopic2 = "datacache/SeeedLoRaE5sensorTwo";  //LoRaE5 temp humidity sensor
const char* subtopic3 = "datacache/A84041F1B186136D"; //LHT65N garage temp humidity sensor

// wio terminal wifi 
// Beetle esp32c3 wifi
WiFiClient wclient;

PubSubClient client(wclient); // Setup MQTT client

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// mqtt message callback
void callback(char* topic, byte* payload, unsigned int length) {

  //print out message topic and payload  
  Serial.print("New message from Subscribe topic: ");
  Serial.println(topic);
  Serial.print("MQTT payload length = ");
  Serial.println(length);
  Serial.print("Subscribe JSON payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);     // print mqtt payload
    //if payload[i] == "t"
  }
  Serial.println();

  msg = 1;  //set message flag = 1 when new subscribe message received

  
  //check subscribe topic for message received and decode    
  
  //********************************//
  //if message from topic subtopic1
  //********************************//
  if (strcmp(topic, subtopic1)== 0) {  

    Serial.print("decode payload from topic ");
    Serial.println(topic);
    deserializeJson(doc, (const byte*)payload, length);   //parse MQTT message
    button = doc["value"];    // boolean button value
    Serial.print("Button boolean value = ");
    Serial.println(button);

    //control state of relay    
    if (button == 1)
    {
      digitalWrite(led, HIGH);
      digitalWrite(6, HIGH);
      delay(100);  
    }
    else
    {
      digitalWrite(led, LOW);
      digitalWrite(6, LOW);
      delay(100);    
    }    
  }
  
  //********************************//
  //if message from topic subtopic2
  //********************************//
  if (strcmp(topic, subtopic2)== 0) { 
    Serial.print("decode payload from topic ");
    Serial.println(topic);
    deserializeJson(doc, (const byte*)payload, length);   //parse MQTT message
    data1 = doc["temperature"];    

    data2 = doc["humidity"];    // 
    
    Serial.print("data1 is LoRaE5 temperature = ");
    Serial.println(data1);
    Serial.print("data2 is LoRaE5 humidity = ");
    Serial.println(data2);     
  }
    
  //********************************//
  //if message from topic subtopic3
  //********************************//
  if (strcmp(topic, subtopic3)== 0) {   
 
    Serial.print("decode payload from topic ");
    Serial.println(topic);
    deserializeJson(doc, (const byte*)payload, length);   //parse MQTT message
    data3 = doc["TempF"];
    data4 = doc["Hum_SHT"];
    Serial.print("data3 = TempF = ");
    Serial.println(data3);
    Serial.print("data4 = Hum_SHT = ");
    Serial.println(data4);
  }
}

//connect to mqtt broker on JEDI One and subscribe to topics
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a unique client ID using the Wio Terminal MAC address
    String MACadd = WiFi.macAddress();
    MACadd = "WioTerminal" + MACadd;  
    String clientID = MACadd;

    // Attempt to connect
    if (client.connect(clientID.c_str())) {
      Serial.println("connected");
      // set up MQTT topic subscription     note: topic needs to be "datacache/" + Device on JEDI One 
      Serial.println("subscribing to topics:");
      Serial.println(subtopic1);        
      client.subscribe(subtopic1);        
      Serial.println(subtopic2);
      client.subscribe(subtopic2); 
      Serial.println(subtopic3);
      client.subscribe(subtopic3); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(led,OUTPUT);
  //pinMode(7, INPUT);   //switch input
  pinMode(6, OUTPUT);   //relay control input 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, 1883);  //set mqtt server
  client.setCallback(callback);
}

void loop() {
  
   // try reconnecting to WiFi if not connected   
  if (WiFi.status() != WL_CONNECTED) {
        Serial.println("not connected to WiFi, try reconnecting");
        WiFi.disconnect();
        delay(5000);
        setup_wifi();
      }
 
  // check if connected to mqtt broker
  if (!client.connected()) {
    reconnect();
  }
  if (msg == 1) {                         // check if new callback message
    
    msg = 0;      // reset message flag
  }
  Serial.println("debug - in main loop");

  delay(1000);

  client.loop();

}
