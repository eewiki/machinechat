#include <WebOTA.h>
#include <ArduinoJson.h>

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

uint8_t temp_farenheit;

// Create a unique ID for the data from each NodeMCU running this code
const char* jediID = "WorkShop-ESP32-Lab3";

// Wi-Fi settings - replace with your Wi-Fi SSID and password
const char* host     = "REMOTICON-OTA"; // Used for MDNS resolution
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

  init_wifi(ssid, password, host);

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

  serializeJson(doc, postData);

  //This prints the JSON to the serial monitor screen
  Serial.println(postData);
}
