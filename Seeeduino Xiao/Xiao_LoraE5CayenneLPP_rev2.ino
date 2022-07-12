// below code is based on Seeed LoRaE5 example code and modified to NOT use the display on the Seeeduino Xiao expansion board (just uses a Xiao connected to DHT11 sensor and LoRaE5 
// eval board0 Seeed example code at https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/
//
//note: all Seeed LoRaE5 Grove boards have App key of 2B7E151628AED2A6ABF7158809CF4F3C 
// rev 2 just changes off time to 60 seconds and humidity sensor channel to 2 to test chirpstack/JEDI integration

#include <Arduino.h>
//sr #include <U8x8lib.h>
#include "DHT.h"
#include <CayenneLPP.h>


#define DHTPIN 2     // what pin DHT11 sensor is connected to 
//#define DHTPIN 0 // what pin we're connected to
 
// Uncomment whatever type you're using!
#define DHTTYPE DHT11 // DHT 11
// #define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
 
DHT dht(DHTPIN, DHTTYPE);

CayenneLPP lpp(51);   //setup Cayenne LPP (low power payload) buffer
 
//sr U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);
// U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
 
static char recv_buf[512];
static bool is_exist = false;
static bool is_join = false;
static int led = 0;

int buf_size;  //Cayenne LPP buffer payload size
int Pointer;   //pointer used in Cayenne LPP buffer
int Offset = 12;    //offset to where Cayenne LPP payload data starts
int Loop1;
int Loop2;

 
static int at_send_check_response(char *p_ack, int timeout_ms, char *p_cmd, ...)
{
    int ch;
    int num = 0;
    int index = 0;
    int startMillis = 0;
    va_list args;
    memset(recv_buf, 0, sizeof(recv_buf));
    va_start(args, p_cmd);
    Serial1.printf(p_cmd, args);
    Serial.printf(p_cmd, args);
    va_end(args);
    delay(200);
    startMillis = millis();
 
    if (p_ack == NULL)
    {
        return 0;
    }
 
    do
    {
        while (Serial1.available() > 0)
        {
            ch = Serial1.read();
            recv_buf[index++] = ch;
            Serial.print((char)ch);
            delay(2);
        }
 
        if (strstr(recv_buf, p_ack) != NULL)
        {
            return 1;
        }
 
    } while (millis() - startMillis < timeout_ms);
    return 0;
}
 
static void recv_prase(char *p_msg)
{  
    if (p_msg == NULL)
    {
        return;
    }
    char *p_start = NULL;
    int data = 0;
    int rssi = 0;
    int snr = 0;
 
    p_start = strstr(p_msg, "RX");
    if (p_start && (1 == sscanf(p_start, "RX: \"%d\"\r\n", &data)))
    {
        Serial.println(data);
// sr        u8x8.setCursor(2, 4);
// sr        u8x8.print("led :");
        led = !!data;
// sr        u8x8.print(led);
        if (led)
        {
            digitalWrite(LED_BUILTIN, LOW);
        }
        else
        {
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }
 
    p_start = strstr(p_msg, "RSSI");
    if (p_start && (1 == sscanf(p_start, "RSSI %d,", &rssi)))
    {
        /*
        u8x8.setCursor(0, 6);
        u8x8.print("                ");
        u8x8.setCursor(2, 6);
        u8x8.print("rssi:");
        u8x8.print(rssi);
        */
        Serial.println(rssi);
    }
    p_start = strstr(p_msg, "SNR");
    if (p_start && (1 == sscanf(p_start, "SNR %d", &snr)))
    {
        /*
        u8x8.setCursor(0, 7);
        u8x8.print("                ");
        u8x8.setCursor(2, 7);
        u8x8.print("snr :");
        u8x8.print(snr);
        */
        Serial.println(snr);
    }
}
 
void setup(void)
{
//sr    u8x8.begin();
//sr    u8x8.setFlipMode(1);
//sr    u8x8.setFont(u8x8_font_chroma48medium8_r);
 
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
 
    Serial1.begin(9600);                  //UART serial1 connection to LoRaE5
    Serial.print("E5 LORAWAN TEST\r\n");
//sr    u8x8.setCursor(0, 0);
 
    if (at_send_check_response("+AT: OK", 100, "AT\r\n"))
    {
        is_exist = true;
        at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
        at_send_check_response("+MODE: LWOTAA", 1000, "AT+MODE=LWOTAA\r\n");
        //at_send_check_response("+DR: EU868", 1000, "AT+DR=EU868\r\n");  //original
        at_send_check_response("+DR: US915", 1000, "AT+DR=US915\r\n"); 
        //at_send_check_response("+CH: NUM", 1000, "AT+CH=NUM,0-2\r\n");  //original
        at_send_check_response("+CH: NUM", 1000, "AT+CH=NUM,8-15,65\r\n"); // configure channels to match chirpstack
        at_send_check_response("+KEY: APPKEY", 1000, "AT+KEY=APPKEY,\"2B7E151628AED2A6ABF7158809CF4F3C\"\r\n");
        at_send_check_response("+CLASS: C", 1000, "AT+CLASS=A\r\n");
        at_send_check_response("+PORT: 8", 1000, "AT+PORT=8\r\n");
        delay(200);
//sr        u8x8.setCursor(5, 0);
//sr        u8x8.print("LoRaWAN");
        is_join = true;
    }
    else
    {
        is_exist = false;
        Serial.print("No E5 module found.\r\n");
//sr        u8x8.setCursor(0, 1);
//sr        u8x8.print("unfound E5 !");
    }
 
    dht.begin();
}
 
void loop(void)
{
    float temp = 0;
    float humi = 0;
 
    temp = dht.readTemperature();
    humi = dht.readHumidity();
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" *C");


//***************** start of Cayenne LPP code **************************
    lpp.reset();
    lpp.addTemperature(1, temp);
    lpp.addRelativeHumidity(2, humi);
    //lpp.addBarometricPressure(3, 1002.4);  //just use dummy value

    buf_size = lpp.getSize();
    Serial.print("Cayenne LPP buffer size = ");
    Serial.println(buf_size);

    uint8_t *payload = lpp.getBuffer();
    char cmd[128];
    
    // Lmsg is just test code that probably can be deleted?
    char Lmsg[4];
    for (unsigned char i = 0; i < lpp.getSize(); i++)
    {
      sprintf(Lmsg, "%02X", (char)payload[i]);
      Serial.print(Lmsg);
    }

    Serial.println(" payload"); 


//**************end of payload builder code**************************



    // start of LoRa communication code 
    if (is_exist)
    {
        int ret = 0;
        if (is_join)
        {
 
            ret = at_send_check_response("+JOIN: Network joined", 12000, "AT+JOIN\r\n");
            if (ret)
            {
                is_join = false;   //resets join check?
            }
            else
            {
                at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
                Serial.print("JOIN failed!\r\n\r\n");
                delay(5000);
            }
        }
        else
        {
            char cmd[128]; 
            //sprintf(cmd, "AT+CMSGHEX=\"%04X%04X\"\r\n", (int)temp, (int)humi); //original
            //****** add in CayenneLPP ecoding ******
            sprintf(cmd, "AT+CMSGHEX=\"%02X\"\r\n", (char)payload[0]);  //first part of message
            // add data payload to LoRa message by looping thru Cayenne LPP data buffer
            char TestMsg[2];   
            for (Loop1 = 0; Loop1 < buf_size; Loop1++)
              {
                Pointer = (Loop1*2) + Offset;
                sprintf(TestMsg, "%02X", (char)payload[Loop1]);
                // write data buffer character to LoRa message
                for (Loop2 = 0; Loop2 < 2; Loop2++)
                  {
                     cmd[Loop2 + Pointer] = TestMsg[Loop2];
                  }
              }            
            // create end of message characters for LoRa message (need ",return,null characters)
            char EndMsg[20];
            sprintf(EndMsg, "test\"%02X\"\r\n", (char)payload[0]);
            // add ", return and null characters to end of LoRa message
            for (unsigned char i = 2; i < 7; i++)
              {
                cmd[Pointer + i] = EndMsg[5 + i];
              }            
            //******** end of CayenneLPP encodeding *********
            ret = at_send_check_response("Done", 5000, cmd);                     //sends cmd command over LoRa
            if (ret)
            {
                recv_prase(recv_buf);
            }
            else
            {
                Serial.print("Send failed!\r\n\r\n");
            }
            delay(5000); 
        }
    Serial.println(" in main loop checking LoRa then wait 60 seconds");   
    delay(60000); //main delay 60 seconds 
    }
    else
    {
        delay(1000);
    }
}
