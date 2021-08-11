/* Code based on below Sparkfun code and library
 *  filename: MKR1010wifiPanSNGCJA5_AQI.ino
 *  8/10/2021 initial code based on PanSNGCJA5_AQI.ino
 *  8/10/2021 update: add in wifi code 
 

// below info from Sparkfum example code "Example1_BasicReadings.ino"
*/

/*
  Reading 1.0 to 10um sized particulate matter from the Panasonic SN-GCJA5
  By: Nathan Seidle
  SparkFun Electronics
  Date: September 8th, 2020
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to output the "mass densities" of PM1, PM2.5, and PM10 readings as well as the 
  "particle counts" for particles 0.5 to 10 microns in size.

  What is PM and why does it matter?
  https://www.epa.gov/pm-pollution/particulate-matter-pm-basics

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/17123

  Hardware Connections:
  The SN-GCJA5 requires 5V for power and 3.3V I2C lines. We recommend using a BlackBoard 
  running at 3.3V: https://www.sparkfun.com/products/16282 or a level converter: https://www.sparkfun.com/products/12009
  Connect the red/black wires of the SN-GCJA5 to 5V/GND
  Connect the Yellow wire to SCL (board must be set to 3.3V to avoid damage to sensor)
  Connect the Blue wire to SDA (board must be set to 3.3V to avoid damage to sensor)
  Open the serial monitor at 115200 baud (note: below code changed to 9600 baud)
*/
#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h" 
#include <Wire.h>

#include <ArduinoJson.h>   //include Json library

#include "SparkFun_Particle_Sensor_SN-GCJA5_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_Particle_SN-GCJA5

// Create a unique ID for the data from the MKRWIFI1010 running this code
const char* jediID = "MKR1010WiFiSensor_SNGCJA5";

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


SFE_PARTICLE_SENSOR myAirSensor;

float pm2_5Avg = 0.0;  //determine 10 second PM2.5 average
float pm10Avg = 0.0;  //determine 10 second PM2.5 average
float AQI2_5;
float AQI10;
int loop_ctr = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("begin...");  
  Serial.println(F("Panasonic SN-GCJA5 Example"));

  Wire.begin();

  if (myAirSensor.begin() == false)
  {
    Serial.println("The particle sensor did not respond. Please check wiring. Freezing...");
    while (1)
      ;
  }
  Serial.println("Sensor started");
  Serial.println("PM:1.0, 2.5, 10, Counts: 0.5, 1, 2.5, 5, 7.5, 10,");

  // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
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

void loop()
{
  // prep for sending HTTP post to JEDI One
  String postData1; //Json string 

  // get particle sensor data
  float pm1_0 = myAirSensor.getPM1_0();
  Serial.print(pm1_0, 2); //Print float with 2 decimals
  Serial.print(",");

  float pm2_5 = myAirSensor.getPM2_5();
  pm2_5Avg = pm2_5Avg + pm2_5;
  Serial.print(pm2_5, 2);
  Serial.print(",");

  float pm10 = myAirSensor.getPM10();
  pm10Avg = pm10Avg + pm10;
  Serial.print(pm10, 2);
  Serial.print(",");

  unsigned int pc0_5 = myAirSensor.getPC0_5();
  Serial.print(pc0_5);
  Serial.print(",");

  unsigned int pc1_0 = myAirSensor.getPC1_0();
  Serial.print(pc1_0);
  Serial.print(",");

  unsigned int pc2_5 = myAirSensor.getPC2_5();
  Serial.print(pc2_5);
  Serial.print(",");

  unsigned int pc5_0 = myAirSensor.getPC5_0();
  Serial.print(pc5_0);
  Serial.print(",");

  unsigned int pc7_5 = myAirSensor.getPC7_5();
  Serial.print(pc7_5);
  Serial.print(",");

  unsigned int pc10 = myAirSensor.getPC10();
  Serial.print(pc10);
  Serial.print(",");

  Serial.println();

 loop_ctr = loop_ctr + 1;
 Serial.print("loop_ctr = ");
 Serial.println(loop_ctr);
 if (loop_ctr > 9)  {
   Serial.print("pm2_5Avg = ");
   pm2_5Avg = pm2_5Avg/10;
   Serial.print(pm2_5Avg);
   Serial.print("   pm10Avg = ");
   pm10Avg = pm10Avg/10;
   Serial.println(pm10Avg);
   AQI2_5 = CalcAQI25(pm2_5Avg);   
   Serial.print("AQI PM2.5 = ");
   Serial.println(AQI2_5);
   AQI10 = CalcAQI10(pm10Avg);   
   Serial.print("AQI PM10 = ");
   Serial.println(AQI10);  
   // round and create int version of AQI10
   AQI10 = AQI10 + 0.5;
   int AQI10int = (int) AQI10;
   Serial.println(AQI10int); 

    //Following code creates the serialized JSON string to send to JEDI One
    //using ArduinoJson library
    StaticJsonDocument <200> doc;
    JsonObject context = doc.createNestedObject("context");
    context["target_id"] = String(jediID);
    JsonObject data = doc.createNestedObject("data");
    data["aqiPM2_5"] = AQI2_5;
    data["aqiPM10"] = AQI10int;
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


   // reset PM averages and loop counter 
   pm2_5Avg = 0;
   pm10Avg = 0;
   loop_ctr = 0;
 }   

  delay(1000); //The sensor has new data every second
}


/*****
 * purpose: calculate PM2.5 AQI
 * parameters:
 *    float Avg2_5   average PM2.5
 *    
 * return value:
 *    float AQI       PM2.5 AQI
 ****/
float CalcAQI25(float Avg2_5)
 {
  float AQI;
  if (Avg2_5 < 12.0) {
    AQI = 4.17 * Avg2_5;  //calculation for PM2.5 0.0 to 12.0 "Good"
  } else if (Avg2_5 < 35.4) {
    AQI = (2.10 * (Avg2_5 - 12.1)) + 51;  //calculation for PM2.5 12.1 to 35.4 "Moderate"  
  } else if (Avg2_5 < 55.4) {
    AQI = (2.46 * (Avg2_5 - 35.5)) + 101;   //calculation for PM2.5 35.5 to 55.4 "Unhealthy for Sensitive Groups"
  } else if (Avg2_5 < 150.4) {
    AQI = (0.52 * (Avg2_5 - 55.5)) + 151;  //calculation for PM2.5 55.5 to 150.4 "Unhealthy"
  } else if (Avg2_5 < 250.4) {
    AQI = (0.99 * (Avg2_5 - 150.5)) + 201;  //calculation for PM2.5 150.5 to 250.4 "Very Unhealthy"
  } else if (Avg2_5 < 350.4) {
    AQI = (0.99 * (Avg2_5 - 250.5)) + 301; //calculation for PM2.5 250.5 to 350.4 "Hazardous"
  } else if (Avg2_5 < 500.4) {
    AQI = (0.66 * (Avg2_5 - 350.5)) + 401; //calculation for PM2.5 350.5 to 500.4 "Hazardous"
  } else {
    AQI = 501.0;
  }
  return AQI;
 }

 /*****
 * purpose: calculate PM10 AQI
 * parameters:
 *    float Avg10   average PM2.5
 *    
 * return value:
 *    float AQI       PM10 AQI
 ****/
float CalcAQI10(float Avg10)
 {
  float AQI;
  if (Avg10 < 54.0) {
    AQI = 0.93 * Avg10;  //calculation for PM10 0.0 to 54.0 "Good"
  } else if (Avg10 < 154) {
    AQI = (0.49 * (Avg10 - 55)) + 51;  //calculation for PM10 55 to 154 "Moderate"  
  } else if (Avg10 < 254) {
    AQI = (0.49 * (Avg10 - 155)) + 101;   //calculation for PM10 155 to 254 "Unhealthy for Sensitive Groups"
  } else if (Avg10 < 354) {
    AQI = (0.49 * (Avg10 - 255)) + 151;  //calculation for PM10 255 to 354 "Unhealthy"
  } else if (Avg10 < 424) {
    AQI = (1.43 * (Avg10 - 355)) + 201;  //calculation for PM10 355 to 424 "Very Unhealthy"
  } else if (Avg10 < 504) {
    AQI = (1.25 * (Avg10 - 425)) + 301; //calculation for PM10 425 to 504 "Hazardous"
  } else if (Avg10 < 604) {
    AQI = (1.0 * (Avg10 - 505)) + 401; //calculation for PM10 505 to 604 "Hazardous"
  } else {
    AQI = 501.0;
  }
  return AQI;
 }

 // print out wifi network info
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
