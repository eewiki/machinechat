/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
   Changed to a beacon scanner to report iBeacon, EddystoneURL and EddystoneTLM beacons by beegee-tokyo
   Modified by SR to decode InPlay beacon temp data, add WiFi and HTTP to post the sensor data to Machinechat JEDI
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 

// Update these with values suitable for your network.
char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// Create a unique ID for the data from the MKRWIFI1010 running this code
const char* jediID = "InplayBleBeacon";

// Change the following IP to your computer's IP running the server, make sure the Port also match
// IP address of server or Raspberry Pi running Machinechat JEDI software
// If you changed the JEDI port number, replace 8100 with the new port
const char* yourLocalIp =  "http://192.168.1.23:8100/v1/data/mc";

byte inplay0 = 5;
int inplayT = 11;


int scanTime = 5; //In seconds
BLEScan *pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      if (advertisedDevice.haveName())
      {
        Serial.print("Device name: ");
        Serial.println(advertisedDevice.getName().c_str());
        Serial.println("");
      }

      if (advertisedDevice.haveServiceUUID())
      {
        BLEUUID devUUID = advertisedDevice.getServiceUUID();
        Serial.print("Found ServiceUUID: ");
        Serial.println(devUUID.toString().c_str());
        Serial.println("");
      }
      
      if (advertisedDevice.haveManufacturerData() == true)
      {
        std::string strManufacturerData = advertisedDevice.getManufacturerData();

        uint8_t cManufacturerData[100];
        strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

        if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00)
        {
          Serial.println("Found an iBeacon!");
          BLEBeacon oBeacon = BLEBeacon();
          oBeacon.setData(strManufacturerData);
          Serial.printf("iBeacon Frame\n");
          Serial.printf("ID: %04X Major: %d Minor: %d UUID: %s Power: %d\n", oBeacon.getManufacturerId(), ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()), oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower());
        }
        else
        {
          Serial.println("Found another manufacturers beacon!");
          Serial.printf("strManufacturerData: %d ", strManufacturerData.length());
          for (int i = 0; i < strManufacturerData.length(); i++)
          {
            Serial.printf("[%X]", cManufacturerData[i]);
          }
          Serial.printf("\n");
        }

      if (cManufacturerData[0] == inplay0 && cManufacturerData[1] == inplay0)
        {
          Serial.println("Mfr is InPlay");
          inplayT = (cManufacturerData[3]<<8) + cManufacturerData[4];
        }     
      }
    }
};

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  delay(5000);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}

void loop()
{
  // put your main code here, to run repeatedly:
  
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  Serial.print("InPlay beacon temp = ");
  Serial.println(inplayT);
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

  // prep for sending HTTP post to JEDI One
  String postData1; //Json string 
  StaticJsonDocument <200> doc;
  JsonObject context = doc.createNestedObject("context");
  context["target_id"] = String(jediID);
  JsonObject data = doc.createNestedObject("data");
  data["tempC"] = inplayT/100;
  serializeJson(doc, postData1);
  Serial.println(postData1);    //debug, can be commented out

// http post code
 if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
 
   HTTPClient http;
 
   http.begin(yourLocalIp);  //Specify destination for HTTP request
   http.addHeader("Content-Type", "application/json");             //Specify content-type header
 
   int httpResponseCode = http.POST(postData1);   //Send the actual POST request
 
   if(httpResponseCode>0){
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);   //Print return code
   }else{
    Serial.print("Error on sending request: ");
    Serial.println(httpResponseCode);
   }
 
   http.end();  //Free resources
 
 }else{
    Serial.println("Error in WiFi connection");
 }
  
  delay(9000);
}
