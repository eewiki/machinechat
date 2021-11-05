/*!
 * filename: MKR1010wifiACcurrentSEN0287.ino
 * date: 11/5/2021
 * description: projects monitors AC current in DFRobot SEN0287 and sends data over WiFi to JEDI One with "state" of current sensor 
 * as as "1" for ON and "0" for OFF. State being monitored is that of a sump pump turning on and off
 * 
 * AC current measurement code based on below DFRobot code for their SEN0287 AC current sensor
 * @file readACCurrent.
 * @n This example reads Analog AC Current Sensor.

 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (https://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @get from https://www.dfrobot.com

 Created 2016-3-10
 By berinie Chen <bernie.chen@dfrobot.com>

 Revised 2019-8-6
 By Henry Zhao<henry.zhao@dfrobot.com>
*/
#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h" 
#include <Wire.h>  //this is not needed

#include <ArduinoJson.h>   //include Json library
// Create a unique ID for the data from the MKRWIFI1010 running this code
const char* jediID = "MKR1010WiFiSensor_Sump";

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status
static int wifiConnectTry = 0;

// IP address of server or Raspberry Pi running Machinechat JEDI software
// If you changed the JEDI port number, replace 8100 with the new port
char serverAddress[] = "192.168.1.7";  // server address
int port = 8100;

WiFiClient client;
HttpClient http = HttpClient(client, serverAddress, port);



const int ACPin = A0;         //set arduino signal read pin for AC current sensor
#define ACTectionRange 5;    //set Non-invasive AC Current Sensor tection range (5A,10A,20A)

// VREF: Analog reference
// For Arduino UNO, Leonardo and mega2560, etc. change VREF to 5
// For Arduino Zero, Due, MKR Family, ESP32, etc. 3V3 controllers, change VREF to 3.3
#define VREF 3.3

unsigned long start, finished, elapsed;
int Istate = 0; //current sensor state 1 = ON, 0 = OFF
int loop_ctr = 0;  //counter for wifi connects

// routine for reading current from DFRobot code for SEN0287
float readACCurrentValue()
{
  float ACCurrtntValue = 0;
  float peakVoltage = 0;  
  float voltageVirtualValue = 0;  //Vrms
  for (int i = 0; i < 5; i++)
  {
    peakVoltage += analogRead(ACPin);   //read peak voltage
    delay(1);
  }
  peakVoltage = peakVoltage / 5;   //average out peak voltage
  peakVoltage = peakVoltage - 5.12; //calibrate out 0 current ADC reading (about 5mV)
  if (peakVoltage < 0.03) peakVoltage = 0; // zero out 0 current measurement
 
  voltageVirtualValue = peakVoltage * 0.707;    //change the peak voltage to the Virtual Value of voltage
  /*The circuit is amplified by 2 times, so it is divided by 2.*/
  voltageVirtualValue = (voltageVirtualValue / 1024 * VREF ) / 2;  
  ACCurrtntValue = voltageVirtualValue * ACTectionRange;

  return ACCurrtntValue;
}

void setup() 
{
  Serial.begin(115200);
  
  pinMode(LED_BUILTIN, OUTPUT); //turn LED ON and OFF based on state of current
    // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    wifiConnect();
   
}

void loop() 
{
    // prep for sending HTTP post to JEDI One
  String postData1; //Json string 
  StaticJsonDocument <200> doc;
  
  float ACCurrentValue = readACCurrentValue(); //read AC Current Value
  Serial.print(ACCurrentValue);
  Serial.println(" A");

   // try reconnecting if not connected   
  if (WiFi.status() != WL_CONNECTED) {
        Serial.println("not connected to WiFi, try reconnecting");
        WiFi.disconnect();
        WiFi.end();
        delay(5000);
        wifiConnect();
      }

   Serial.print("WiFi status code: ");
   Serial.println(WiFi.status());
   Serial.print("WiFi reconnect trys = ");
   Serial.println(wifiConnectTry);


  //check if AC current is ON and current Istate = 0
  if ((ACCurrentValue > 0.05) && (Istate == 0)){
    digitalWrite(LED_BUILTIN, HIGH);
    start = millis();
    Istate = 1;
    //Following code creates the serialized JSON string to send to JEDI One
    //using ArduinoJson library
    //StaticJsonDocument <200> doc;  //move to start of loop
    JsonObject context = doc.createNestedObject("context");
    context["target_id"] = String(jediID);
    JsonObject data = doc.createNestedObject("data");
    data["Isensor"] = Istate;
    data["MilliStamp"] = start;
    serializeJson(doc, postData1);
    Serial.println(postData1);    //debug, can be commented out

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

    // try disconnecting from server after posting data
    http.stop();
    Serial.println("disconnect from server");

  }
  //check if AC current has turned off and current Istate = 1 
  if ((ACCurrentValue < 0.05) && (Istate == 1)) {
    digitalWrite(LED_BUILTIN, LOW);
    finished = millis();
    elapsed = finished - start;
    Istate = 0;
    Serial.print("On time = ");
    Serial.println(elapsed);

    //Following code creates the serialized JSON string to send to JEDI One
    //using ArduinoJson library
    //StaticJsonDocument <200> doc;  //move to start of loop
    JsonObject context = doc.createNestedObject("context");
    context["target_id"] = String(jediID);
    JsonObject data = doc.createNestedObject("data");
    data["Isensor"] = Istate;
    data["MilliStamp"] = finished;
    serializeJson(doc, postData1);
    Serial.println(postData1);    //debug, can be commented out

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

    // try disconnecting from server after posting data
    http.stop();
    Serial.println("disconnect from server");    
  }
  
  
  delay(1000);
}


// connect to wifi network info
void wifiConnect() {
//  static int wifiConnectTry = 0;
      if (WiFi.status() != WL_CONNECTED) {
        // try connecting
        WiFi.begin(ssid, pass); 
        wifiConnectTry++;
        Serial.print(wifiConnectTry);
        Serial.print(" Attempt to connect to network: ");
        Serial.println(ssid);        
      }
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
