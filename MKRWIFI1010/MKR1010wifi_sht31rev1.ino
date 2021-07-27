// code for mkr1010wifi to read sht31 sensor data and send to JEDI One over WiFi with http post
// filename: MKR1010wifi_sht31rev1
// update:
// 7/27/2021 rev1 - clean up code

#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h" 
#include <Wire.h>
#include "SHT31.h"

// Create a unique ID for the data from MKRWiFi1010 running this code
const char* jediID = "MKR1010WiFiSensor_SHT31";

//include Json library
#include <ArduinoJson.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

// IP address of server or Raspberry Pi running Machinechat JEDI software
// If you changed the JEDI port number, replace 8100 with the new port
char serverAddress[] = "192.168.1.7";  // server address
int port = 8100;

WiFiClient client;
HttpClient http = HttpClient(client, serverAddress, port);
 
SHT31 sht31 = SHT31();
 
void setup() {  
  Serial.begin(9600);
  while(!Serial);
  Serial.println("begin...");  
  sht31.begin();  

  // attempt to connect to Wifi network:
  WiFi.begin(ssid, pass);  
  Serial.print("Attempting to connect to network: ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  Serial.println("----------------------------------------");
  printData();
  Serial.println("----------------------------------------");
}
 
void loop() {
  // prep for sending HTTP to JEDI One
  String postData1; //Json string 

  // SHT31 sensor info
  float tempC = sht31.getTemperature();
  float humid = sht31.getHumidity();
  Serial.print("Temp = "); 
  Serial.print(tempC);
  Serial.println(" C"); 
  Serial.print("Hum = "); 
  Serial.print(humid);
  Serial.println("%"); 
  Serial.println();
  
  //Following code creates the serialized JSON string to send to JEDI One
  //using ArduinoJson library
  StaticJsonDocument <200> doc;
  JsonObject context = doc.createNestedObject("context");
  context["target_id"] = String(jediID);
  JsonObject data = doc.createNestedObject("data");
  data["tempC"] = tempC;
  data["humi"] = humid;
  serializeJson(doc, postData1);
  Serial.println(postData1);    //debug, can be commented out

  //HTTP post code start
  if (WiFi.status() == WL_CONNECTED) {
    //format http post to be compatible with JEDI one
    String contentType = "application/json";
    http.post("/v1/data/mc", contentType, postData1);
    // read the status code and body of the response
    int statusCode = http.responseStatusCode();
    String response = http.responseBody();
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
  } else {
    Serial.println("Error in WiFi connection");
  }
  
  delay(10000); //delay between sensor measurements
}

// wifi network info
void printData() {
  Serial.println("Board Information:");
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}
