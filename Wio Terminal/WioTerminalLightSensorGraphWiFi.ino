/*
 * Filename: WioTerminalLightSensorWiFi.ino
 * purpose: read light sensor on Wio Terminal, display on TFT screen and HTTP post to machinechat JEDI One
 */
 
#include <rpcWiFi.h>
#include <HTTPClient.h>
#include "arduino_secrets.h" 
#include <ArduinoJson.h>

#include"seeed_line_chart.h" //include the library
#include"TFT_eSPI.h"
#include"Free_Fonts.h" //include the header file
TFT_eSPI tft;

// for LCD line chart
#define max_size 50 //maximum size of data
doubles data; //Initilising a doubles type to store data
TFT_eSprite spr = TFT_eSprite(&tft);  // Sprite 

// Create a unique ID for the data from Wio Terminal running this code
const char* jediID = "WioTerminalLightSensor";
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char password[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// Change the following IP to your computer's IP running the server, make sure the Port also match
// IP address of server or Raspberry Pi running Machinechat JEDI software
// If you changed the JEDI port number, replace 8100 with the new port
const char* yourLocalIp =  "http://192.168.1.7:8100/v1/data/mc";


void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_WHITE); //Black background
  spr.createSprite(320,120); //try half screen
  tft.setFreeFont(FF19); //select font
  tft.setTextColor(TFT_BLACK);
  tft.drawString("Light Sensor",10,120);//prints string at (10,10)

  pinMode(WIO_LIGHT, INPUT);  //Wio Terminal internal light sensor
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(500);
    Serial.println("Connecting..");
  }
  Serial.print("Connected to the WiFi network with IP: ");
  Serial.println(WiFi.localIP());   
}
 
void loop() {
   //prep for sending HTTP to JEDI One
   String postData1; //Json string   
   tft.setFreeFont(FF24); //select font
   tft.setTextColor(TFT_RED);   
   int light = analogRead(WIO_LIGHT);
   char value[7] = "     ";

   Serial.print("Light value: ");  //debug, can be commented out
   Serial.println(light);
   itoa(light,value,10);  //convert light sensor integer value to character
   tft.drawString(value,60,170);//prints data at (x,y)
   spr.fillSprite(TFT_WHITE); 
   if (data.size() == max_size) {
        data.pop();//this is used to remove the first read variable
    }
    data.push(1.0 * light); //read variables and store in data

  //Settings for the line graph
    auto content = line_chart(20, 10); //(x,y) where the line graph begins
         content
                .height(tft.height() - 140) //header.height() * 1.5) //actual height of the line chart
                .width(tft.width() - content.x() * 2) //actual width of the line chart
                .based_on(0.0) //Starting point of y-axis, must be a float
                .show_circle(false) //drawing a cirle at each point, default is on.
                .value(data) //passing through the data to line graph
                .color(TFT_PURPLE) //Setting the color for the line
                .draw();
 
    spr.pushSprite(0, 0);
   
   //Following code creates the serialized JSON string to send to JEDI One
   //using ArduinoJson library
   StaticJsonDocument <200> doc;
   JsonObject context = doc.createNestedObject("context");
   context["target_id"] = String(jediID);
   JsonObject data = doc.createNestedObject("data");
   data["lightSen"] = light;
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
   
   delay(5000);
   tft.setTextColor(TFT_WHITE);   //"clear" old light sensor data value
   tft.drawString(value,60,170);       
}
