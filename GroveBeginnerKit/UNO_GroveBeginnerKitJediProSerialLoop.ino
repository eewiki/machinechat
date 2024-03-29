//
// Seeed Grove Beginner Kit for Arduino - default code
// Modified to send CSV information over serial (USB) port
// For use as input to Machinechat JEDI software
// v0.1 DRM 07-04-2022
//
// CSV is as follows: "Light, Sound, Temoperature, Humidity, Air Pressure,
// Acceleration X, Acceleration Y, Acceleration Z"
//
// Only one parameter is sent at a time based on selection displayed on OLED
// Display
// SBR modification to remove selection capability and just loop thru the sensors send & display data on OLED
//
#include <U8g2lib.h>
#include <Wire.h>
//#include <MsTimer2.h>
#include "DHT.h" 
#include "Seeed_BMP280.h"
#include "LIS3DHTR.h"
#include "millisDelay.h" //part of SafeString library for non-blocking timer use
#define WIRE Wire
millisDelay loopDelay;
//20*20
const unsigned char sound_bmp[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3e, 0x00, 0x80, 0x3f, 0x0c, 0xc0, 0x3b, 0x0c, 0xfe, 0xb8, 0x0d, 0x7f, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0xff, 0xb8, 0x0d, 0xc0, 0x39, 0x0c, 0x80, 0x3f, 0x0c, 0x00, 0x3e, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00};
//20*30
const unsigned char temp_bmp[] U8X8_PROGMEM = {0x00, 0x06, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x80, 0x19, 0x00, 0x80, 0x19, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x3f, 0x00, 0x80, 0x1f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x06, 0x00};
//20*20
const unsigned char hum_bmp[] U8X8_PROGMEM= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x00,0x80,0x10,0x00,0x00,0x00,0x00,0x40,0x20,0x00,0x20,0x40,0x00,0x20,0x40,0x00,0x10,0x80,0x00,0x10,0x80,0x00,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x10,0x80,0x00,0x00,0x00,0x00,0x40,0x20,0x00,0x80,0x10,0x00};
//20*20
const unsigned char pressure_bmp1[] U8X8_PROGMEM= {0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x06,0x00,0x00,0x06,0x00,0x00,0x06,0x00,0x80,0x96,0x01,0x80,0x1f,0x03,0x00,0x0f,0x06,0x18,0x06,0x04,0x1c,0x04,0x04,0x06,0x00,0x06,0xc6,0xff,0x03,0xc6,0xff,0x01,0x04,0x00,0x00,0xfc,0xff,0x03,0xf8,0xff,0x03,0x00,0x00,0x00,0xfe,0xff,0x07,0xfe,0xff,0x07,0x00,0x00,0x00};
//30*30
const unsigned char pressure_bmp[] U8X8_PROGMEM= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc8,0xc4,0x01,0x00,0xf8,0x87,0x07,0x00,0xf8,0x87,0x07,0x60,0xf0,0x03,0x04,0x60,0xf0,0x03,0x04,0xf0,0xc0,0x00,0x04,0xf0,0xc0,0x00,0x04,0x18,0x00,0x00,0x06,0x18,0x00,0x00,0x06,0x08,0xfc,0xff,0x03,0x08,0xfc,0xff,0x03,0x18,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0xf0,0xff,0xff,0x03,0xf0,0xff,0xff,0x03,0xe0,0xff,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xff,0xff,0x1f,0xfe,0xff,0xff,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//30*30
const unsigned char light_tmp[] U8X8_PROGMEM = {0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x80, 0xc1, 0x60, 0x00, 0x80, 0xc3, 0x70, 0x00, 0x00, 0xc3, 0x30, 0x00, 0x00, 0xc7, 0x38, 0x00, 0x00, 0x06, 0x18, 0x00, 0x0c, 0xf0, 0x03, 0x0c, 0x3c, 0xfc, 0x0f, 0x0f, 0x78, 0x1e, 0x9e, 0x07, 0x60, 0x07, 0xb8, 0x01, 0x00, 0x03, 0x30, 0x00, 0x80, 0x03, 0x70, 0x00, 0x80, 0x01, 0x60, 0x00, 0xbf, 0x01, 0x60, 0x3f, 0xbf, 0x01, 0x60, 0x3f, 0x80, 0x01, 0x60, 0x00, 0x80, 0x03, 0x70, 0x00, 0x00, 0x03, 0x30, 0x00, 0x60, 0x07, 0xb8, 0x01, 0x78, 0x1e, 0x9e, 0x07, 0x3c, 0xfc, 0x0f, 0x0f, 0x0c, 0xf0, 0x03, 0x0c, 0x00, 0x06, 0x18, 0x00, 0x00, 0xc7, 0x38, 0x00, 0x00, 0xc3, 0x30, 0x00, 0x80, 0xc3, 0x70, 0x00, 0x80, 0xc1, 0x60, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00};
//30*30
const unsigned char sound_bmp1[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x60, 0x00, 0x00, 0x70, 0xc0, 0x00, 0x00, 0x48, 0x80, 0x01, 0x00, 0x4c, 0x18, 0x03, 0x00, 0x46, 0x30, 0x02, 0x00, 0x43, 0x60, 0x04, 0x80, 0x41, 0x43, 0x04, 0xfc, 0x40, 0x84, 0x08, 0x46, 0x40, 0x8c, 0x08, 0x46, 0x40, 0x88, 0x08, 0x46, 0x40, 0x10, 0x09, 0x06, 0x40, 0x10, 0x09, 0x06, 0x40, 0x10, 0x19, 0x06, 0x40, 0x10, 0x09, 0x06, 0x40, 0x10, 0x09, 0x46, 0x40, 0x18, 0x09, 0x46, 0x40, 0x88, 0x08, 0x46, 0x40, 0x8c, 0x08, 0xfc, 0x40, 0x86, 0x0c, 0x80, 0x41, 0x43, 0x04, 0x00, 0x43, 0x20, 0x04, 0x00, 0x42, 0x30, 0x02, 0x00, 0x44, 0x0c, 0x03, 0x00, 0x48, 0x80, 0x01, 0x00, 0x50, 0xc0, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//30*30
const unsigned char acel_bmp[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x07, 0x00, 0x00, 0x07, 0x1c, 0x00, 0x80, 0x01, 0x60, 0x00, 0x60, 0x00, 0x80, 0x00, 0x30, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x04, 0x0c, 0x00, 0x00, 0x04, 0x04, 0x20, 0x01, 0x08, 0x04, 0x10, 0x02, 0x08, 0x02, 0xf8, 0x03, 0x08, 0x02, 0x0c, 0x0e, 0x10, 0x02, 0x0a, 0x10, 0x10, 0x02, 0x0a, 0x10, 0x10, 0x02, 0x02, 0x0a, 0x10, 0x02, 0x18, 0x06, 0x18, 0x06, 0x10, 0x02, 0x08, 0x04, 0x20, 0x01, 0x08, 0x04, 0xc0, 0x00, 0x0c, 0x08, 0x00, 0x00, 0x04, 0x18, 0x00, 0x00, 0x02, 0x30, 0x00, 0x00, 0x03, 0x60, 0x00, 0x80, 0x01, 0xc0, 0x00, 0xc0, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//30*30
const unsigned char temp_bmp1[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xfc, 0x01, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x1f, 0x8f, 0xff, 0xff, 0x3f, 0x0f, 0x00, 0x00, 0x3e, 0x0f, 0x00, 0x00, 0x3e, 0x8f, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x1f, 0xfe, 0xff, 0xff, 0x07, 0xfc, 0x01, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define BoardVersion 2

#if BoardVersion == 1
char led = 5;
char buzzer = 6;
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2, /* reset=*/U8X8_PIN_NONE);
char button = 2;
char rotary = A1;
char light = A2;
char sound = A0;
DHT dht(4,DHT11);
BMP280 bmp280; 
// LIS3DHTR<TwoWire> accelemeter(I2C_MODE);
LIS3DHTR<TwoWire> accelemeter; //IIC

#elif BoardVersion == 2
char led = 4;
char buzzer = 5;
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2, /* reset=*/U8X8_PIN_NONE);
char button = 6;
char rotary = A0;
char light = A6;
char sound = A2;
DHT dht(3,DHT11);
BMP280 bmp280; 
//LIS3DHTR<TwoWire> accelemeter(I2C_MODE);
LIS3DHTR<TwoWire> accelemeter; //IIC
#endif

#define CLICKS 100
int BuzzerFrequency = 300;
char MODE = 1;
char LongPress = false;
int x = 50, y = 13;
char PressCounter = 0;
char BlinkEnable = true;
#define BLINK 10
#define SQueueLEN 10
int SQueue[SQueueLEN] = {0};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!bmp280.init()){
    Serial.println("bmp280 init error!");
  }
  //accelemeter.begin(Wire);
  accelemeter.begin(WIRE, LIS3DHTR_ADDRESS_UPDATED); //IIC init
  delay(100);
  //accelemeter.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
  accelemeter.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);

  if (!accelemeter) {
      Serial.println("LIS3DHTR didn't connect.");
  }
  for (int Index = 0 ; Index < SQueueLEN ; Index ++)
  {
    SQueue[Index] = analogRead(sound);
  }
  u8g2.begin();

  //SBR mod
  MODE = 1;
  loopDelay.start(10000);  // start a 10sec delay
}

void loop() {
  if (loopDelay.justFinished()) {
    MODE = MODE + 1; // increment mode
    loopDelay.start(10000);  // start a 10sec delay
  }

  //reset mode loop
  if (MODE == 6) {
    MODE = 1;
  }

   if (MODE == 1) {
    Light_show();
  } else if (MODE == 2) {
    Sound_show();
  } else if (MODE == 3) {
    Temp_show();
  } else if (MODE == 4) {
    Pressure_show();
  } else if (MODE == 5) {
    Acele_show();
  }
}


void Light_show()
{
  int l = analogRead(light);
  Serial.println(String(l) + ",,,,,,,");
  Serial.flush();
  delay(500);
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.drawCircle(8, 8, 8, U8G2_DRAW_ALL);
    if (l >= 50 && l < 100) {
      u8g2.drawDisc(8, 8, 8, U8G2_DRAW_UPPER_LEFT);
    }
    if (l >= 100 & l < 200) {
      u8g2.drawDisc(8, 8, 8,  U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    }
    if (l >= 200 && l < 350) {
      u8g2.drawDisc(8, 8, 8,  U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    }
    if (l >= 350) {
      u8g2.drawDisc(8, 8, 8,  U8G2_DRAW_ALL);
    }
    u8g2.setCursor(26, 32);
    u8g2.print(F("Light:"));
    u8g2.setCursor(80, 32);
    u8g2.print(l);
     //SBR mod
    u8g2.setCursor(0, 64);
    u8g2.print(F("JEDI Serial data"));

    
  } while (u8g2.nextPage());
}

int filter1(int NEW_DATA,int QUEUE[],char n)
{
    int max;
    int min;
    int sum;
    char i;
    QUEUE[0]=NEW_DATA;
    if (QUEUE[0] < 0)
    QUEUE[0] = 0;
    max=QUEUE[0];
    min=QUEUE[0];
    sum=QUEUE[0];
    for(i=n-1;i!=0;i--){
        if(QUEUE[i]>max)max=QUEUE[i];                  
        else if (QUEUE[i]<min)min=QUEUE[i];             
        sum=sum+QUEUE[i];                              
        QUEUE[i]=QUEUE[i-1];                            
    }

    i=n-2;
    sum=sum-max-min+i/2;
    sum=sum/i;                                     
                                                    
    return ((int)sum);
}       
void Sound_show()
{
  int s = analogRead(sound);
  s = filter1(s,SQueue,SQueueLEN);
  delay(500);
  Serial.println("," + String(s) + ",,,,,,");
  Serial.flush();
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.setCursor(28, 32);
    u8g2.print(F("Sound:"));
    u8g2.setCursor(76, 32);
    u8g2.drawXBMP(0, 0, 20, 20, sound_bmp);
    u8g2.print(s);
    //SBR mods
    u8g2.setCursor(0, 64);
    u8g2.print(F("JEDI Serial data"));
    
  } while (u8g2.nextPage());
}

void Temp_show()
{
  int tempC = 0, humid = 0;
  float tempF = 0.0;
  do{
  humid = dht.readHumidity();
  tempC = dht.readTemperature();
  tempF = (float(tempC) * 1.8) + 32.0F;
  } while ((humid == 0) && (tempC == 0)); 
  delay(500);
  Serial.println(",," + String(int(tempF)) + ",,,,,");
  Serial.flush();
  Serial.println(",,," + String(humid) + ",,,,");
  Serial.flush();
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.setCursor(32, 16);
    u8g2.print(F("Temp:"));
    u8g2.setCursor(72, 16);
    u8g2.print(int(tempF));
    u8g2.setCursor(88, 16);
    u8g2.print("F");
    u8g2.setCursor(32, 40);
    u8g2.print(F("Humid:"));
    u8g2.setCursor(80, 40);
    u8g2.print(humid);
    u8g2.drawXBMP(0, 0, 20, 30, temp_bmp);
    u8g2.setCursor(0, 64);
    u8g2.print(F("JEDI Serial data")); 
  } while (u8g2.nextPage());
}

void Pressure_show()
{
  float pressure = (bmp280.getPressure() * 0.000295300586F);
  delay(500);
  Serial.println(",,,," + String(pressure, 2) + ",,,");
  Serial.flush();
  u8g2.setFont(u8g2_font_t0_16b_mr);
  u8g2.firstPage();
  do {
      u8g2.setCursor(76, 32);
      u8g2.drawXBMP(0, 0, 20, 20, pressure_bmp1);
      u8g2.setCursor(30, 25);
      u8g2.print("Pressure:"); 
      u8g2.setCursor(30, 40);
      u8g2.print(pressure);
      u8g2.print("inHg");
      u8g2.setCursor(0, 64);
      u8g2.print(F("JEDI Serial data"));       
    } while (u8g2.nextPage());
}

void Acele_show()
{
  float ax, ay, az;
  ax = accelemeter.getAccelerationX();
  ay = -accelemeter.getAccelerationY();
  az = accelemeter.getAccelerationZ();
  delay(500);
  Serial.println(",,,,," + String(ax, 2) + ",,");
  Serial.flush();
  Serial.println(",,,,,," + String(ay, 2) + ",");
  Serial.flush();
  Serial.println(",,,,,,," + String(az, 2));
  Serial.flush();
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.setCursor(0, 16);
    u8g2.print(F("Accel:")); 
    u8g2.setFont(u8g2_font_t0_12b_mr);   
    u8g2.setCursor(55, 16);
    u8g2.print(F("X:"));
    u8g2.setCursor(71, 16);
    u8g2.print(ax);
    u8g2.setCursor(55, 32);
    u8g2.print(F("Y:"));
    u8g2.setCursor(71, 32);
    u8g2.print(ay);
    u8g2.setCursor(55, 48);
    u8g2.print(F("Z:"));
    u8g2.setCursor(71, 48);
    u8g2.print(az);
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.setCursor(0, 64);
    u8g2.print(F("JEDI Serial data"));        
  } while (u8g2.nextPage());
}
