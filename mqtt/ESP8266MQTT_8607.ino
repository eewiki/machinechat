//Arduino code for ESP8266 modules to send environmental sensor data over MQTT
//from a MS8607 module (i2c connect) every x seconds to Machinechat's MQTT broker 
//running on the JEDI One platform. The serial monitor will show status and debug messages
//
//Note: Program will not proceed until a valid sensor is detected
//
//v1 January,25 2021 SR


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 
//add in MS8607 related code
#include <Adafruit_Sensor.h>
#include <Adafruit_MS8607.h>

Adafruit_MS8607 ms8607;
 

// Wi-Fi settings - replace with your Wi-Fi SSID and password
char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

const char* mqttServer = "192.168.1.10";
const int mqttPort = 1883;
//const char* mqttUser = "YourMqttUser";
//const char* mqttPassword = "YourMqttUserPassword";
int loopCtr = 0; //set up loop counting variable
 
WiFiClient espClient;
PubSubClient client(espClient);

 
void setup() {
 
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

   
  WiFi.begin(ssid, pass);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  // set server to JEDI mqtt server 
  client.setServer(mqttServer, mqttPort);
//  client.setCallback(callback);

Serial.print("MAC address: ");
Serial.println(WiFi.macAddress()); // print out MAC address
String MACadd = WiFi.macAddress();
MACadd = "ESP" + MACadd;  //add "ESP" to MAC address
Serial.println(MACadd);

// create unique client ID using MAC address for MQTT broker and convert to char array
// example "ESP40:F5:20:27:11:82"
String clientID = MACadd;
int str_len = clientID.length() + 1; 
// Prepare the character array (the buffer) 
char clientIDchar[str_len];
// Copy it over 
clientID.toCharArray(clientIDchar, str_len);


 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect(clientIDchar)) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
     }
  }

}


 
void loop() {

  StaticJsonDocument<200> doc; //humidity json 
  StaticJsonDocument<200> doc1; //temperature json
  StaticJsonDocument<200> doc2; //pressure json
  String publishData;


// MS8607 test code code, can be commented out
    sensors_event_t temp, pressure, humidity;
    ms8607.getEvent(&pressure, &temp, &humidity);
    Serial.print("Temperature: ");Serial.print(temp.temperature); Serial.println(" degrees C");
    Serial.print("Pressure: ");Serial.print(pressure.pressure); Serial.println(" hPa");
    Serial.print("Humidity: ");Serial.print(humidity.relative_humidity); Serial.println(" %rH");

  // measure and publish humidity data
    Serial.println("in humidity loop");
    float humi = humidity.relative_humidity;
    int dataInt = round_int(humi); //round and convert to integer since high accuracy not necessary
    doc["humidity"] = dataInt;
    serializeJson(doc, publishData);
    Serial.println(publishData); //for testing, can be deleted
    int str_len = strLength(publishData);
   // Prepare the character array (the buffer) 
    char publishChar[str_len];
    // Copy it over 
    publishData.toCharArray(publishChar, str_len);
    client.publish("MS8607", publishChar);  
    publishData = ""; //clear string

delay(1000);

    // measure and publish temperature data
    Serial.println("in temp loop");
    float tempF = temp.temperature*1.8 + 32;
    dataInt = round_int(tempF); //round and convert to integer since high accuracy not necessary
    doc1["tempF"] = dataInt;
    serializeJson(doc1, publishData);
    Serial.println(publishData); //for testing, can be deleted
    str_len = strLength(publishData);
    // Prepare the character array (the buffer) 
    publishChar[str_len];
    // Copy it over 
    publishData.toCharArray(publishChar, str_len);
    client.publish("MS8607", publishChar);  
    publishData = ""; //clear string    

delay(1000);

  // measure and publish pressure data
    Serial.println("in pressure loop");
    float press = pressure.pressure;
    dataInt = round_int(press); //round and convert to integer since high accuracy not necessary
    doc2["pressure"] = dataInt;
    serializeJson(doc2, publishData);
    Serial.println(publishData); //for testing, can be deleted
    str_len = strLength(publishData);
   // Prepare the character array (the buffer) 
    publishChar[str_len];
    // Copy it over 
    publishData.toCharArray(publishChar, str_len);
    client.publish("MS8607", publishChar);  
    publishData = ""; //clear string
  
 
  Serial.println(" ");
  delay(8000);
}

// integer rounding function
int round_int(float x){
  int result;
  if ( x > 0 ){
    result = x + 0.5;
  } else {
    result = x - 0.5;
  }
  return result;
}

// build string length for char array function
int strLength(String strX) {
  int result;
  result = strX.length() + 1;
  return result;
}
