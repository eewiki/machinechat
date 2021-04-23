/*
 Basic ESP8266 MQTT subscribe example based on "mqtt_esp8266.ino" example in https://github.com/knolleary/pubsubclient
 This sketch demonstrates subscribing to sensor data using the Machinechat JEDI One MQTT broker
 with the ESP8266 board/library.
 It uses WiFi to connect to the JEDI One MQTT broker then:
 
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example in link
 above for how to achieve the same result without blocking the main loop.
 
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 

// Update these with values suitable for your network.
char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
const char* mqttServer = "192.168.1.11";
const int mqttPort = 1883;

StaticJsonDocument<256> doc;

int data1;  //data1 of MQTT json message
int data2;  //data2 of MQTT json message
int data3;  //data3 of MQTT json message
int msg = 0;
const char* timestamp = "dummy data";  //the is the MQTT message timestamp
//const char* current_ts = "blank";
String recv_payload;

WiFiClient espClient;
PubSubClient client(espClient);

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Subscribe topic: ");
  Serial.println(topic);
  Serial.print("Subscribe JSON payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);     // print mqtt payload
  }
  Serial.println();

  msg = 1;  //message flag = 1 when new subscribe message received

  deserializeJson(doc, (const byte*)payload, length);   //parse MQTT message
  data1 = doc["data1"];    // data1 is humidity
  data2 = doc["data2"];    // data2 is temp
  data3 = doc["data3"];    // data3 is empty
  timestamp = doc["timestamp"];    //mqtt message timestamp
  //Serial.println(timestamp);     //debug print
  
  recv_payload = String(( char *) payload);  // put payload in string for future use
  //Serial.println("print recv_payload string");   //debug info
  //Serial.println(recv_payload);                  //debug info
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a unique client ID using the ESP8266 MAC address
    String MACadd = WiFi.macAddress();
    MACadd = "ESP8266cli" + MACadd;  //add "ESP8266cli" to MAC address
    //Serial.println(MACadd);        // debug print
    String clientID = MACadd;

    // Attempt to connect
    if (client.connect(clientID.c_str())) {
      Serial.println("connected");
      // set up MQTT topic subscription
      client.subscribe("datacache/T960981B2D");  // topic needs to be "datacache/" + Device on JEDI One 
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
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, 1883);  //set mqtt server
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (msg == 1) {                         // check if new callback message
    //Serial.println("message flag = 1");  //debug print
    msg = 0;                             // reset message flag
    Serial.print("decoded timestamp = ");
    Serial.println(timestamp);
    Serial.print("decoded data1 = ");
    Serial.println(data1);
    Serial.print("decoded data2 = ");
    Serial.println(data2);
  }
  
  // blink LED to show esp8266 active in main loop
  digitalWrite(BUILTIN_LED, HIGH);   // Turn the LED off
  delay(1000);
  digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on
  delay(1000);
  client.loop();

    

}
