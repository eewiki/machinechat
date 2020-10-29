// Code for NodeMCU boards with Espressif 8266 WiFi controller
// for measuring temperature and humidity using the i2c Amphenol T9602 sensor
// code based on https://github.com/NorthernWidget-Skunkworks/T9602_Library but heavily modified for this application


#include "Wire.h"
#include <ESP8266WiFi.h>    // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>

// Include the T9602 library
#include "T9602.h"

// Declare variables -- just as strings

String tdata;   //temperature string
String hdata;  // humidity string
String postData;  //string data to post to Jedi One
//


#include "arduino_secrets.h" 
// Wi-Fi settings - replace with your Wi-Fi SSID and password

char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)


// IP address of server or Raspberry Pi running Machinechat JEDI software
// If you changed the JEDI port number, replace 8100 with the new port
const char* host = "192.168.1.10:8100";


// Instantiate class
T9602 mySensor;

void setup(){

  // Configure status LED on NodeMCU board
  // Later, it will be programmed to blink every HTTP POST
  pinMode(LED_BUILTIN, OUTPUT);

  // Configure serial port for debug when ESP8266 board is connected
  // to your computer or laptop using USB port  
  Serial.begin(115200); 
  while(!Serial){} // Waiting for serial connection

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
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



  
  Serial.println();
  Serial.println("Start I2C scanner ...");
  Serial.print("\r\n");
  byte count = 0;
  byte i = 0;
  
  Wire.begin();
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0)
      {
      Serial.print("Found I2C Device: ");
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
      delay(1);
      }
  }
  Serial.print("\r\n");
  Serial.println("Finish I2C scanner");
  Serial.print("Found ");
  Serial.print(count, HEX);
  Serial.println(" Device(s).");
  Serial.print(count);
  Serial.print(i);

  
}

void loop() {
    // Take one reading every (delay + time to take reading) seconds
    // and print it to the screen
    mySensor.updateMeasurements();
        
    tdata = mySensor.getString();
    Serial.print("tdata Temp in C ");
    Serial.println(tdata);
    hdata = mySensor.getHstr(); //SR code modified to get Humidty data in string
    Serial.print("Humidity in % ");
    Serial.println(hdata);
    delay(5000); // Wait 5 seconds before the next reading, inefficiently   


    // Build a string with data to send to JEDI. The format is
    // {
    //    "context": {
    //        "target_id" : "Sensor1"
    //    },
    //    "data": {
    //        "metric1" : metric_value,
    //        "metric2" : metric_value
    //    }
    // }
    //
    // Replace metric1 with what ever data metrics that you are
    // sending to JEDI. Replace metric_value with the value of
    // the metric. If you have more than one sensor, set the
    // target_id with the name of the sensor.
  
    postData = "{\"context\":{\"target_id\":\"AmphT9602\"}, \"data\":{\"tempC\":" + tdata + ", " +
             "\"humi\":" + hdata +  " }}";  


    Serial.println(postData);  //serial print postData for debug

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

    // Blink the status LED
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);

  } else {
    Serial.println("Error in WiFi connection");
  }


    
}
