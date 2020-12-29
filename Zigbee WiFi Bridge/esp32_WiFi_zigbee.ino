//Arduino code for ESP32 Thing Plus modules to bridge environmental sensor data
//from Xbee3 Zigbee modules over WiFi to Machinechat's JEDI One software.
//A Zigbee coordinator echoes Zigbee sensor node data over the serial connection to the ESP32.
//The serial debig monitor will show status
//
//
//
//v1 December 10, 2020 SR

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// serial 2 interface code
#define RXD2 16
#define TXD2 17
int incomingByte = 0; // for incoming serial data
String message;

// initialize variables for test/debug
int param1 = 99;
int param2 = 99;
int param3 = 99;
String param0 = "99";
String paramS = "99";
String znode = "99";


#include "arduino_secrets.h" 
// Wi-Fi settings - replace with your Wi-Fi SSID and password
char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

// IP address of PC or Raspberry Pi running Machinechat JEDI software
// If you changed the JEDI port number, replace 8100 with the new port
const char* host = "192.168.1.10:8100";

void setup(void) {

  // Configure serial port for debug when ESP32 board is connected
  // to computer using USB port
  Serial.begin(115200);
  // Configure serial2 port for connecting to Xbee3 module
  Serial2.begin(57600, SERIAL_8N1, RXD2, TXD2); //serial2 interfaces to zigbee coordinator

  
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }

   // Initiate Wi-Fi connection setup
  WiFi.begin(ssid, pass);

  // Show status on serial monitor
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  // Wait for Wi-Fi connection and show progress on serial monitor
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

 }


// This is the main section and will loop continuously

void loop() {

  String postData;

// serial2 receives the zigbee sensor data from uart on xbee  
// code is written to handle up to 3 sensor parameters per message from zigbee node
// if less than 3 parameters im message, value of 0 assigned to parameter

  if (Serial2.available())  {
    // read the incoming bytes on serial2:
    message = Serial2.readStringUntil('\n');
    // print serial2 message to serial debug interface
    Serial.println("message = " + message);
    
    String word0 = getValue(message, ':', 0);  //message header
    String word1 = getValue(message, ':', 1);  //zigbee 64bit node ID

    // check if message from zigbee node
    if (word0 = "MsgFrom") {
      Serial.println("have a match =" + word0);
      String word2 = getValue(message, ':', 2);  //zigbee sensor name
      String word5 = getValue(message, ':', 5);
      String word7 = getValue(message, ':', 7);
      String word9 = getValue(message, ':', 9);

      param0 = word1.substring(10); // unique part of zigbee 64bit ID
      paramS = word2.substring(0,4); //unique part of sensor name
      param1 = word5.toInt();  //sensor parameter1
      param2 = word7.toInt();  //sensor parameter2
      param3 = word9.toInt();  //sensor parameter3    
    }


// sensor and json code
  // convert readings and set variables
  String sName = paramS;
  String zNode = param0;
  Serial.print("zigbee node = ");
  Serial.println(zNode);
  Serial.print("sensor name = ");
  Serial.println(sName);

  znode = "abc";
  sName = "123";

  znode = param0;
  sName = paramS;
  
  //Following code creates the serialized JSON string to send to JEDI One
  //using ArduinoJson library
  StaticJsonDocument <200> doc;

  JsonObject context = doc.createNestedObject("context");
  context["target_id"] = sName + znode;

  JsonObject data = doc.createNestedObject("data");
  data["data2"] = param2;
  data["data3"] = param3;
  data["data1"] = param1;

  serializeJson(doc, postData);

  //This prints the JSON to the serial monitor screen
  //and can be commented out
  Serial.println(postData);



// wifi json code

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClient client;
    HTTPClient http;

    // Send the data to JEDI using HTTP.
    String address = String("http://") + String(host) + String("/v1/data/mc");
    http.begin(client, address);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(postData);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.print("received payload:  ");
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();  //Close connection

  } else {
    Serial.println("Error in WiFi connection");
  }

  
  // Wait for 1 seconds before repeating the loop
  delay(1000);
  }
}


//string manipulation function
 String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
