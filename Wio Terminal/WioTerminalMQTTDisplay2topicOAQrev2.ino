/*
 Wio Terminal MQTT data display example 
 filename: WioTerminalMQTTDisplay2topicOAQ.ino
 project based on WioTerminalMQTTDisplay.ino which is a sketch that demonstrates using the Wio Terminal to mqtt subscribe to air quality sensor data on the 
 machinechat JEDI One IoT data platform. It uses WiFi to connect to the JEDI One MQTT broker.
 Ozone air quality sensor data - Renesas
 Particle air quality sensor date - Panasonic
 11/10/2021 - this project takes sketch WioTerminalMQTTDisplay.ino and modifies to subscribe to 2 topics and changes topics to outdoor air quality sensors for 
 ozone and particulate matter to display AQI
 11/16/2021 - clean up code and change to version to "_rev1", also add 3rd subtopic for testing
 3/30/2022 - rev2 change back to subscribe to just two topics (comment out subtopic1
*/

#include <rpcWiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include"seeed_line_chart.h" //include the library

TFT_eSPI tft;

// for LCD line chart
#define max_size 50 //maximum size of data
doubles data[2]; //Initilising a doubles type to store data
TFT_eSprite spr = TFT_eSprite(&tft);  // Sprite 


char value[7] = "      "; //initial values
char value2[7] = "      ";
char stamp[40] = "2021-09-01T13:04:34"; 

//#include <ESP8266WiFi.h>
//#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 

// Update these with values suitable for your network.
char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// MQTT server info for JEDI One
const char* mqttServer = "192.168.1.7";
const int mqttPort = 1883;

StaticJsonDocument<256> doc;

int data1 = 0;  //data1 of MQTT json message
int data2 = 0;  //data2 of MQTT json message
int data3 = 0;  //data3 of MQTT json message
int data4 = 0;  //data3 of MQTT json message
int msg = 0;
const char* timestamp = "dummy data";  //the is the MQTT message timestamp (this works)
String recv_payload;
const char* subtopic1 = "datacache/T960981B2D";                //pump house humidity, temp sensor
const char* subtopic2 = "datacache/MKR1010_Z4510oaq2S31PMOD";  //ZMOD4510 air quality/ozone sensor
const char* subtopic3 = "datacache/MKR1010WiFiSensor_SNGCJA5"; //SNGCJA5x air quality particl sensor

// wio terminal wifi 
WiFiClient wclient;
PubSubClient client(wclient); // Setup MQTT client

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

// mqtt message callback
void callback(char* topic, byte* payload, unsigned int length) {

  //print out message topic and payload  
  Serial.print("New message from Subscribe topic: ");
  Serial.println(topic);
  Serial.print("Subscribe JSON payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);     // print mqtt payload
  }
  Serial.println();

  msg = 1;  //set message flag = 1 when new subscribe message received

  
  //check subscribe topic for message received and decode    
  
  //********************************//
  //if message from topic subtopic1
  //********************************//
  if (strcmp(topic, subtopic1)== 0) {  
    // "clear" old data by rewriting old info in white
    tft.setTextColor(TFT_WHITE);   //"clear" old data by rewriting old info in white
    tft.setFreeFont(FF19);
    tft.drawString(value,200,78); 
    //tft.drawString(value2,60,78);  //value2 is same as data2
    tft.setFreeFont(FF18);
    tft.drawString(stamp,60,110);             

    Serial.print("decode payload from topic ");
    Serial.println(topic);
    deserializeJson(doc, (const byte*)payload, length);   //parse MQTT message
    data1 = doc["data1"];    // data1 is humidity
    //data2 = data4;           // data4 is O3 FastAQI
    //data2 = doc["data2"];    // data2 is temp
    //data3 = doc["data3"];    // data3 is empty
    timestamp = doc["timestamp"];    //mqtt message timestamp
    //  stamp = timestamp;
    strcpy (stamp,timestamp);
    stamp[19] = 0;   // terminate string after seconds
    Serial.println(stamp);     //debug print

    tft.setTextColor(TFT_MAGENTA);
    tft.setFreeFont(FF19);     
    itoa(data1,value,10);  //convert data integer value to character
    tft.drawString(value,200,78);//prints data at (x,y)
    //itoa(data2,value2,10);  
    //tft.setTextColor(TFT_BLUE);
    //tft.drawString(value2,60,78);  
    tft.setTextColor(TFT_BLACK);
    tft.setFreeFont(FF18);
    tft.drawString(stamp,60,110); //print timestamp
  }
  //********************************//
  //if message from topic subtopic2
  //********************************//
  if (strcmp(topic, subtopic2)== 0) {   
    // "clear" old data by rewriting old info in white
    tft.setTextColor(TFT_WHITE);   //"clear" old data by rewriting old info in white
    tft.setFreeFont(FF19);
    tft.drawString(value2,50,78);  //"clear" old value2 (is same as data2)  
    tft.setFreeFont(FF18);
    tft.drawString(stamp,60,110);   //"clear" old timestamp      
    
    Serial.print("decode payload from topic ");
    Serial.println(topic);
    deserializeJson(doc, (const byte*)payload, length);   //parse MQTT message
//    data1 = doc["data1"];    
    data2 = doc["FastAQI"];    // 
    itoa(data2,value2,10); 
    tft.setFreeFont(FF19); 
    tft.setTextColor(TFT_BLUE);
    tft.drawString(value2,50,78);  //print new value2 on LCD
    Serial.print("data2 is O3 FastAQI = ");
    Serial.println(data2);
    
    timestamp = doc["timestamp"];    //mqtt message timestamp
    //  stamp = timestamp;
    strcpy (stamp,timestamp);
    stamp[19] = 0;   // terminate string after seconds     
    tft.setTextColor(TFT_BLUE);
    tft.setFreeFont(FF18);
    tft.drawString(stamp,60,110); //print new timestamp on LCD   
    
    //print out color of "AQI" font based on value of AQI
    // <50=green,51to100=yellow,101to150=orange,>150=red
    if (data2 < 51) {
      tft.setTextColor(TFT_GREEN);
    }
    else if (data2 < 101){
      tft.setTextColor(TFT_YELLOW);
    }
    else if (data2 < 151){
      tft.setTextColor(TFT_ORANGE);
    }
    else {
      tft.setTextColor(TFT_RED);  
    }
    tft.setFreeFont(FF18);
    tft.drawString("AQI",100,78);
     
  }
    
  //********************************//
  //if message from topic subtopic3
  //********************************//
  if (strcmp(topic, subtopic3)== 0) {   
    // "clear" old data by rewriting old info in white
    tft.setTextColor(TFT_WHITE);   //"clear" old data by rewriting old info in white
    tft.setFreeFont(FF19);
    tft.drawString(value,200,78);  //"clear" old value   
    tft.setFreeFont(FF18);
    tft.drawString(stamp,60,110);   //"clear" old timestamp   

    Serial.print("decode payload from topic ");
    Serial.println(topic);
    deserializeJson(doc, (const byte*)payload, length);   //parse MQTT message
    data3 = doc["aqiPM2_5"];
    data4 = doc["aqiPM10"];
    Serial.print("data3 = aqiPM2_5 = ");
    Serial.println(data3);
    Serial.print("data4 = aqiPM10 = ");
    Serial.println(data4);
    // determine which PM2.5 or PM10 is larger to use when displaying PM AQI
    if (data3 >= data4) {
      data1 = data3;
    }
    else {
      data1 = data4;
    }
    Serial.print("Particle AQI = ");
    Serial.println(data1);
    tft.setTextColor(TFT_MAGENTA);
    tft.setFreeFont(FF19);     
    itoa(data1,value,10);  //convert data integer to character "value"
    tft.drawString(value,200,78);//prints "value" at (x,y)

    timestamp = doc["timestamp"];    //mqtt message timestamp
    strcpy (stamp,timestamp);
    stamp[19] = 0;   // terminate string after seconds     
    tft.setTextColor(TFT_MAGENTA);
    tft.setFreeFont(FF18);
    tft.drawString(stamp,60,110); //print new timestamp on LCD 

    //print out color of "AQI" font based on value of AQI
    // <50=green,51to100=yellow,101to150=orange,>150=red
    if (data1 < 51) {
      tft.setTextColor(TFT_GREEN);
    }
    else if (data1 < 101){
      tft.setTextColor(TFT_YELLOW);
    }
    else if (data1 < 151){
      tft.setTextColor(TFT_ORANGE);
    }
    else {
      tft.setTextColor(TFT_RED);  
    }
    tft.setFreeFont(FF18);
    tft.drawString("AQI",250,78);
  }
}

//connect to mqtt broker on JEDI One and subscribe to topics
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a unique client ID using the Wio Terminal MAC address
    String MACadd = WiFi.macAddress();
    MACadd = "WioTerminal" + MACadd;  
    String clientID = MACadd;

    // Attempt to connect
    if (client.connect(clientID.c_str())) {
      Serial.println("connected");
      // set up MQTT topic subscription     note: topic needs to be "datacache/" + Device on JEDI One 
      Serial.println("subscribing to topics:");
      //Serial.println(subtopic1);        //comment out subtopic1 for this revision of code
      //client.subscribe(subtopic1);        
      Serial.println(subtopic2);
      client.subscribe(subtopic2); 
      Serial.println(subtopic3);
      client.subscribe(subtopic3); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // setup lcd display
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_WHITE); //Black background
  spr.createSprite(320,110); //try 320x110
  tft.setFreeFont(FF19); //select font
  tft.setTextColor(TFT_BLACK  );
  tft.drawString("Outdoor Air ",5,5);
  tft.drawString("Quality",195,5);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(FF18);
  tft.drawString("  Ozone AQI:    Particle AQI:",10,50);
  tft.drawString(stamp,60,110);
  tft.setFreeFont(FF18);
  tft.setTextColor(TFT_BLACK);
  tft.drawString("AQI",250,78);
  tft.drawString("AQI",100,78);  
  tft.fillRect(0, 40, 320, 2, TFT_BLACK);
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, 1883);  //set mqtt server
  client.setCallback(callback);
}

void loop() {
  // check if connected to mqtt
  if (!client.connected()) {
    reconnect();
  }
  if (msg == 1) {                         // check if new callback message
    
    msg = 0;                             // reset message flag
    Serial.print("decoded timestamp = ");
    Serial.println(stamp);
    Serial.print("decoded data1 = ");
    Serial.println(data1);
    Serial.print("decoded data2 = ");
    Serial.println(data2);

    // update line chart code
    // chart consists of two lines tracking values data1 and data2
    Serial.print("data[].size = ");
    Serial.println(data[0].size());  //debug - print data[] size for line chart
    if (data[0].size() == max_size) {
      for (uint8_t i = 0; i<2; i++){
        data[i].pop();              //this is used to remove the first read variable when max_size reached
      }
    }
    data[0].push(1.0 * data1);  //read variables and store in line chart data array
    data[1].push(1.0 * data2); 
//    Serial.println(data[0]);   //debug    

    spr.fillSprite(TFT_WHITE);  //clear line chart area
    
    //Settings for the line graph title
    auto header =  text( 0, 100)
                .value("test")
                .align(center)
                .valign(vcenter)
                .width(tft.width())
                .thickness(3);
 
    header.height(header.font_height() );
   // header.draw(); //Header is not used so disabled
    
    auto content = line_chart(20, header.height()-10); //(x,y) where the line graph begins
         content
                .height(tft.height() - 140) //header.height() * 1.5) //actual height of the line chart
                .width(tft.width() - 20) //actual width of the line chart
                .based_on(0.0) //Starting point of y-axis, must be a float
                .show_circle(false) //drawing a cirle at each point, default is on so set to false.
                .value({data[0], data[1]}) //passing through the data to line graph
                .color(TFT_PURPLE, TFT_BLUE) //Setting the color for each line
                .draw();
    
    spr.pushSprite(0, 130);  //display line chart
  }
  Serial.println("debug - in main loop");


  delay(2000);

  client.loop();

}
