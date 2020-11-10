// Code for boards with Espressif 8266 WiFi controller
// for measuring temperature and humidity using the i2c Amphenol T9602 sensor.
// Amphenol T9602 sensor code based on https://github.com/NorthernWidget-Skunkworks/T9602_Library but modified 
// for this example application


#include "Wire.h"
#include <ESP8266WiFi.h>    // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>

// Include the T9602 library
#include "T9602.h"

// Create a unique ID for the data from each ESP8266 running this code
const char* jediID = "ESP8266AmphT9602Sensor1";

//include Json library
#include <ArduinoJson.h>

#include "arduino_secrets.h" 
// Wi-Fi settings - update arduino_secrets.h with your Wi-Fi SSID and password

char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)


// IP address of server or Raspberry Pi running Machinechat JEDI software
// If you changed the JEDI port number, replace 8100 with the new port
const char* host = "192.168.1.10:8100";


// Instantiate class
T9602 mySensor;

void setup(){

  // Configure status LED on board
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
    Serial.printf("[SETUP] WAIT.... %d...\n", t);
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
  Serial.println("sketch : ESP8266_T9602_JEDI_sketch ");

  
}

void loop() {

    String postData1; //Json string 

    
    // This is the minimum amount of time to wait before
    // reading the sensor
    delay(1000);
    
    // Take one reading every (delay + time to take reading) seconds
    // and print it to the screen
    mySensor.updateMeasurements();


    //float sensor data
    float tempC = mySensor.getTemperature();
    float humid = mySensor.getHumidity();
    //print sensor data to serail monitor, can be commented out
    Serial.print("float tempC: ");
    Serial.println(tempC);
    Serial.print("float humid: ");
    Serial.println(humid);

    
    //Following code creates the serialized JSON string to send to JEDI One
    //using ArduinoJson library
    StaticJsonDocument <200> doc;

    JsonObject context = doc.createNestedObject("context");
    context["target_id"] = String(jediID);

    JsonObject data = doc.createNestedObject("data");
    data["tempC"] = tempC;
    data["humi"] = humid;
    
    serializeJson(doc, postData1);
    //This prints the JSON to the serial monitor screen
    //and can be commented out
    Serial.println(postData1);    


  if (WiFi.status() == WL_CONNECTED) {

    WiFiClient client;
    HTTPClient http;

    // Send the data to JEDI using HTTP.
    String address = String("http://") + String(host) + String("/v1/data/mc");
    http.begin(client, address);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(postData1);

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

  // Wait for 5 seconds before repeating the loop
  delay(5000);    
}
