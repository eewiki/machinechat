/*
 Wio Terminal MQTT data display example 
 This sketch demonstrates using the Wio Terminal to mqtt subscribe to temp and humidity sensor data on the 
 machinechat JEDI One IoT data platform. It uses WiFi to connect to the JEDI One MQTT broker.
 
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


char value[7] = "      "; //initial value
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

int data1;  //data1 of MQTT json message
int data2;  //data2 of MQTT json message
int data3;  //data3 of MQTT json message
int msg = 0;
const char* timestamp = "dummy data";  //the is the MQTT message timestamp (this works)
String recv_payload;

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

  tft.setTextColor(TFT_WHITE);   //"clear" old data by rewriting old info in white
  tft.setFreeFont(FF19);
  tft.drawString(value,200,78); 
  tft.drawString(value2,60,78);  
  tft.setFreeFont(FF18);
  tft.drawString(stamp,60,110);  
    
  Serial.print("Subscribe topic: ");
  Serial.println(topic);
  Serial.print("Subscribe JSON payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);     // print mqtt payload
  }
  Serial.println();

  msg = 1;  //message flag = 1 when new subscribe message received

  deserializeJson(doc, (const byte*)payload, length);   //parse MQTT message
  data1 = doc["data1"];    // data1 is humidity
  data2 = doc["data2"];    // data2 is temp
  data3 = doc["data3"];    // data3 is empty
  timestamp = doc["timestamp"];    //mqtt message timestamp
//  stamp = timestamp;
  strcpy (stamp,timestamp);
  stamp[19] = 0;   // terminate string after seconds
  Serial.println(stamp);     //debug print

  tft.setTextColor(TFT_MAGENTA);
  tft.setFreeFont(FF19);     
  itoa(data1,value,10);  //convert data integer value to character
  tft.drawString(value,200,78);//prints data at (x,y)
  itoa(data2,value2,10);  
  tft.setTextColor(TFT_BLUE);
  tft.drawString(value2,60,78);  
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(FF18);
  tft.drawString(stamp,60,110); //print timestamp
  
  recv_payload = String(( char *) payload);  // put payload in string for future use
  //Serial.println("print recv_payload string");   //debug info
  //Serial.println(recv_payload);                  //debug info
}

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
      // set up MQTT topic subscription
      client.subscribe("datacache/T960981B2D");  // topic needs to be "datacache/" + Device on JEDI One 
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
  tft.setTextColor(TFT_RED  );
  tft.drawString("Pump House Data",5,5);//prints string at (10,10)
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(FF18);
  tft.drawString("  Temperature:  Humidity:",10,50);
  tft.drawString(stamp,60,110);
  tft.setFreeFont(FF19);
  tft.setTextColor(TFT_MAGENTA);
  tft.drawString(" %",237,78);
  tft.setTextColor(TFT_BLUE);
  tft.drawString(" F",100,78);  
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
    //Serial.println("message flag = 1");  //debug print
    msg = 0;                             // reset message flag
    Serial.print("decoded timestamp = ");
    Serial.println(timestamp);
    Serial.print("decoded data1 = ");
    Serial.println(data1);
    Serial.print("decoded data2 = ");
    Serial.println(data2);

    // line chart code
    Serial.print("data[].size = ");
    Serial.println(data[0].size());  //debug - print data[] size for line chart
    if (data[0].size() == max_size) {
      for (uint8_t i = 0; i<2; i++){
        data[i].pop();//this is used to remove the first read variable
      }
    }
    data[0].push(1.0 * data1);  //read variables and store in data
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
                .show_circle(false) //drawing a cirle at each point, default is on.
                .value({data[0], data[1]}) //passing through the data to line graph
                .color(TFT_PURPLE, TFT_BLUE) //Setting the color for the line
                .draw();
    
    spr.pushSprite(0, 130);  //display chart
  }
  Serial.println("debug - in main loop");


  delay(2000);

  client.loop();

}
