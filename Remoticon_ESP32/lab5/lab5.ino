#include <WebOTA.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HTTPClient.h>

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

uint8_t temp_farenheit;

// Data wire is plugged into pin 13
#define ONE_WIRE_BUS 13

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

// Create a unique ID for the data from each NodeMCU running this code
const char* jediID = "WorkShop-ESP32-Lab5";

// Wi-Fi settings - replace with your Wi-Fi SSID and password
const char* mdns     = "REMOTICON-OTA"; // Used for MDNS resolution
//const char* ssid     = "DemoWiFi";
//const char* password = "DemoWiFi";

// IP address of server running Machinechat JEDI software
// If you changed the JEDI port number, replace 8100 with the new port
//const char* host = "192.168.1.210:8100";

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }

  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) {
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }

  if (!sensors.getAddress(insideThermometer, 0) {
    Serial.println("Unable to find address for Device 0"); 
  }
  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);
 
  Serial.print("Device 0 Resolution: ");
  Serial.println(sensors.getResolution(insideThermometer), DEC);

  init_wifi(ssid, password, mdns);

  // Wait for Wi-Fi connection and show progress on serial monitor
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.flush();

  // Defaults to 8080 and "/webota"
  //webota.init(80, "/update");
}

// the loop function runs over and over again forever
void loop() {
  String postData;
  int md = 1000;

  webota.delay(md);
  webota.handle();

  temp_farenheit = temprature_sens_read();

  sensors.requestTemperatures();

  float tempC = sensors.getTempC(insideThermometer);
  float tempF = DallasTemperature::toFahrenheit(tempC);

  Serial.print(" | ESP32 Temp[F]: ");
  Serial.print(temp_farenheit);
  Serial.println(" |");

  //Following code creates the serialized JSON string to send to JEDI One
  //using ArduinoJson library
  StaticJsonDocument <200> doc;

  JsonObject context = doc.createNestedObject("context");
  context["target_id"] = String(jediID);

  JsonObject data = doc.createNestedObject("data");
  data["ESP32_tempF"] = temp_farenheit;
  data["1W_tempF"] = tempF;

  serializeJson(doc, postData);

  //This prints the JSON to the serial monitor screen
  Serial.println(postData);

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    String address = String("http://") + String(host) + String("/v1/data/mc");
    http.begin(client, address);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    // Free resources
    http.end();
  }
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) {
      Serial.print("0");
    }
    Serial.print(deviceAddress[i], HEX);
  }
}
