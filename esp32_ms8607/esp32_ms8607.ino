//Arduino code for ESP32 modules to send environmental sensor data
//from a MS8607 module (i2c connect) every x seconds to Machinechat's JEDI One software
//The serial monitor will show status
//
//Note: Program will not proceed until a valid sensor is detected
//
//v1 November 12, 2020 SR

#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MS8607.h>
#include <ArduinoJson.h>

Adafruit_MS8607 ms8607;

// Create a unique ID for the data from each ESP32 running this code
const char* jediID = "ESP32_MS8607";

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
  
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }
  

  // Try to initialize sensor!
  if (!ms8607.begin()) {
    Serial.println("Failed to find MS8607 chip");
    while (1) { delay(10); }
  }
  Serial.println("MS8607 Found!");


  ms8607.setHumidityResolution(MS8607_HUMIDITY_RESOLUTION_OSR_8b);
  Serial.print("Humidity resolution set to ");
  switch (ms8607.getHumidityResolution()){
    case MS8607_HUMIDITY_RESOLUTION_OSR_12b: Serial.println("12-bit"); break;
    case MS8607_HUMIDITY_RESOLUTION_OSR_11b: Serial.println("11-bit"); break;
    case MS8607_HUMIDITY_RESOLUTION_OSR_10b: Serial.println("10-bit"); break;
    case MS8607_HUMIDITY_RESOLUTION_OSR_8b: Serial.println("8-bit"); break;
  }
  // ms8607.setPressureResolution(MS8607_PRESSURE_RESOLUTION_OSR_4096);
  Serial.print("Pressure and Temperature resolution set to ");
  switch (ms8607.getPressureResolution()){
    case MS8607_PRESSURE_RESOLUTION_OSR_256: Serial.println("256"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_512: Serial.println("512"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_1024: Serial.println("1024"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_2048: Serial.println("2048"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_4096: Serial.println("4096"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_8192: Serial.println("8192"); break;
  }
  Serial.println("");



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

  

  //Serial.println("Checking MS8607");
  unsigned status;


}

// This is the main section and will loop continuously
void loop() {

  String postData;

  // This is the minimum amount of time to wait before
  // reading the sensor
  delay(1000);


// MS8607 test code code, can be commented out
    sensors_event_t temp, pressure, humidity;
    ms8607.getEvent(&pressure, &temp, &humidity);
    Serial.print("Temperature: ");Serial.print(temp.temperature); Serial.println(" degrees C");
    Serial.print("Pressure: ");Serial.print(pressure.pressure); Serial.println(" hPa");
    Serial.print("Humidity: ");Serial.print(humidity.relative_humidity); Serial.println(" %rH");
    Serial.println("");

// sensor and json code
  // Read the sensor, convert readings and set variables
  float tempC = temp.temperature;
  Serial.print("float tempC = ");
  Serial.println(tempC);
  float humi = humidity.relative_humidity;
  Serial.print("float humi = ");
  Serial.println(humi);
  float pressure1 = pressure.pressure; // / 100.0F;
  Serial.print("float pressure = ");
  Serial.println(pressure1);
  //float tempF = (tempC * 1.8) + 32.0F;
  //float pressure_inches = pressure * 0.0295300586F;


  
  //Following code creates the serialized JSON string to send to JEDI One
  //using ArduinoJson library
  StaticJsonDocument <200> doc;

  JsonObject context = doc.createNestedObject("context");
  context["target_id"] = String(jediID);

  JsonObject data = doc.createNestedObject("data");
  data["tempC"] = tempC;
  data["humi"] = humi;
  data["pressure"] = pressure1;

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

  
  // Wait for 5 seconds before repeating the loop
  delay(5000);
}
