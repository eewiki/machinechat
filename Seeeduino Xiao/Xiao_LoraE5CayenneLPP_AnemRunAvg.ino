// below code is based on Seeed LoRaE5 example code and modified to NOT use the display on the Seeeduino Xiao expansion board (just uses a Xiao connected to a LoRaE5 
// Grove expansion board. Seeed example code at https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/
//
//note: all Seeed LoRaE5 Grove boards have example code App key of "2B7E151628AED2A6ABF7158809CF4F3C" so needs to be changed
// this version changes out the DHT11 sensor for Sparkfun weather meter kit for wind speed and direction

// Anemometer measuring code based on below example code:
//More Information at: https://www.aeq-web.com/
//Version 2.0 | 11-NOV-2020
//
//SBR modifications 2-NOV-2022
//SensorPin is connected to RC pulldown circuit on anemometer switch
//Wind vane functionality removed from this version
//Anemometer used is industry #40R with 4 magnets per rev, windspeed formula is: (Hz * 1.38) + 1.17
//add in Running Average code that uses RunningAverage library
//update sampling times for average wind speed and max wind speed gust

#include <Arduino.h>
#include <CayenneLPP.h>

// this version adds in RunningAverage code
#include "RunningAverage.h"
RunningAverage myRA(10);
int samples = 0;

// anemometer and direction parameters
const int RecordTime = 3; //Define Measuring Time (Seconds)
const int SensorPin = 2;     // the number of the sensorpin
//const int VanePin = 1;      // pin# connected to wind vane
const int ledPin =  13;      // the number of the LED pin
int InterruptCounter;
float WindSpeed;
float WindSpeedAvg;   //average wind speed
float WindSpeedGust; //max wind speed gust
  

CayenneLPP lpp(51);   //setup Cayenne LPP (low power payload) buffer - per documentation 51 bytes is safe to send
 
static char recv_buf[512];
static bool is_exist = false;
static bool is_join = false;
static int led = 0;

int buf_size;  //Cayenne LPP buffer payload size
int Pointer;   //pointer used in Cayenne LPP buffer
int Offset = 12;    //offset to where Cayenne LPP payload data starts
int Loop1;       //loop counter in LoRa payload builder
int Loop2;       //loop counter in LoRa payload builder
int Loop3 = 0;       //loop counter in LoRa parameter send

 
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

 // LoRa message receive message buffer RSSI and SNR
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
        led = !!data;
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
         Serial.println(rssi);
    }
    p_start = strstr(p_msg, "SNR");
    if (p_start && (1 == sscanf(p_start, "SNR %d", &snr)))
    {
        Serial.println(snr);
    }
}

 
void setup(void)
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    myRA.clear(); // explicitly start clean with RunningAverage by clearing buffer

    // anemeometer and wind vane setup
    pinMode(SensorPin, INPUT); //use input to count interrupts of anemometer
    //pinMode(VanePin, INPUT); //use input to read analog voltage of wind vane output
 
    Serial1.begin(9600);                  //UART serial1 connection to LoRaE5
    Serial.print("E5 LORAWAN TEST\r\n");
 
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
        is_join = true;
    }
    else
    {
        is_exist = false;
        Serial.print("No E5 module found.\r\n");
    }
 
}
 
void loop(void)
//void loop()
{
     
    // measure windspeed and add to Running Average data table myRA
    measure();
    myRA.addValue(WindSpeed);
    samples++;
    //debug code to print out sample# and min, average, max windspeed 
    Serial.print("samples = ");
    Serial.print(samples);
    Serial.print("\t");
    Serial.print("MinAvgMax  ");
    Serial.print(myRA.getMin(), 3);
    Serial.print("\t");
    Serial.print(myRA.getAverage(), 3);
    WindSpeedAvg = myRA.getAverage();
    Serial.print("\t");
    Serial.println(myRA.getMax(), 3);
    WindSpeedGust = myRA.getMax();

    
    
    Serial.print("Windspeed: ");
    Serial.print(WindSpeed);
    Serial.print(" mph ");


    // due to character byte limitatations in LoRa payload buffer, need to alternate between 
    // the two wind parameters when sending out parameter payload so use loop3
    
    Serial.print("Loop3 = ");  //debug code
    Serial.println(Loop3);    //debug code

    if ((Loop3 == 0) and (samples == 40))
    //when samples = 40, add WindSpeedAvg to LoRa payload
    {
      lpp.reset();
      lpp.addAnalogOutput(3, WindSpeedAvg);  //channel 3, Wind Speed Avg
      Loop3 = 1;
      BuildPayload();
    }
    else if((Loop3 == 1) and (samples == 50))
    //when samples = 50, add WindSpeedGust to LoRa payload
    {
      lpp.reset();
      lpp.addAnalogOutput(4, WindSpeedGust);  //channel 4, Wind Speed Max
      Loop3 = 0;
      BuildPayload();
    }
    else
    {
      Serial.println("sample limits not reached");
    }



  // clear running average data after samples = 51   
  if (samples == 51)
  {
    samples = 0;
    myRA.clear();
    Serial.println("RunningAverage data cleared");
  }
    
}


//count anemometer pulses and calculate windspeed
void measure() {
  InterruptCounter = 0;
  attachInterrupt(digitalPinToInterrupt(SensorPin), countup, RISING);
  delay(1000 * RecordTime);
  detachInterrupt(digitalPinToInterrupt(SensorPin));
  // windspeed calculation for #40R anemometer: mph = (Hz * 0.857) + 0.724
  WindSpeed = (float)InterruptCounter / ((float)RecordTime * 0.857) + 0.724;
  if (InterruptCounter == 0)
  {
    WindSpeed = 0;
  }
}

void countup() {
  InterruptCounter++;
}

//LoRa payload builder and send function
void BuildPayload() {
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
            sprintf(cmd, "AT+CMSGHEX=\"%02X\"\r\n", (char)payload[0]);  //first part of confirmed message
            Serial.println("debug - print cmd at start of loop");
            Serial.println(cmd);
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
            Serial.println("debug - print cmd at end of loop");
            Serial.println(cmd);
            //******** end of CayenneLPP encodeding *********
            ret = at_send_check_response("Done", 12000, cmd);        //sends cmd command over LoRa - increase time from 5000 to 12000
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
    Serial.println(" in main loop checking LoRa then delay for 10 seconds");   
    delay(10000); // 10 seconds
    }
    else
    {
        delay(1000);
    }


}
