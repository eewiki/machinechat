#include <SPI.h>
#include <WiFi101.h>
#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>

#include <Wire.h>
#include "Adafruit_MCP9808.h"

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 MCP9808_TempSensor = Adafruit_MCP9808();

// Create a unique ID for the data from each device running this code
const char* jediID = "SAM_IOT";

char serverAddress[] = "192.168.3.104";
int port = 8100;

// Initialize the Ethernet client library
WiFiClient wlan;
HttpClient client = HttpClient(wlan, serverAddress, port);
int status = WL_IDLE_STATUS;

uint16_t Light_Sensor_Pin = A1;    // select the input pin for the potentiometer
float Light_Sensor = 0;  // variable to store the value coming from the sensor

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  delay(3000);
  Serial.println("Startup - Delay USB");

  if (!MCP9808_TempSensor.begin(0x18)) {
    Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
    while (1);
  }
    
  Serial.println("Found MCP9808!");

  MCP9808_TempSensor.setResolution(3); // sets the resolution mode of reading, the modes are defined in the table bellow:
  // Mode Resolution SampleTime
  //  0    0.5°C       30 ms
  //  1    0.25°C      65 ms
  //  2    0.125°C     130 ms
  //  3    0.0625°C    250 ms

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();
}

void loop() {
  String postData;
  MCP9808_TempSensor.wake();   // wake up, ready to read!
  //Serial.println (MCP9808_TempSensor.getResolution());
  float MCP9808_TempSensor_F = MCP9808_TempSensor.readTempF();

  Light_Sensor = (analogRead(Light_Sensor_Pin) * 1650)/4095;

  Serial.print(" | Temp[F]: ");
  Serial.print(MCP9808_TempSensor_F, 4);
  Serial.print(" | Light: ");
  Serial.print(Light_Sensor);
  Serial.println(" |");

  StaticJsonDocument <200> doc;
  
  JsonObject context = doc.createNestedObject("context");
  context["target_id"] = String(jediID);

  JsonObject data = doc.createNestedObject("data");
  data["MCP9808"] = MCP9808_TempSensor_F;
  data["TEMPT6000"] = Light_Sensor;

  serializeJson(doc, postData);

  //This prints the JSON to the serial monitor screen
  Serial.println(postData);
  
  if (status == WL_CONNECTED) {
    String contentType = "application/json";

    client.post("/v1/data/mc", contentType, postData);

    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
  
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
  }

  delay(1000);
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
