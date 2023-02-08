// This program uses a Xiao and SEN0287 AC current sensor to measure current and sends sample measurements of average current over LoRa
//
// below LoRa code is based on Seeed LoRaE5 example code and modified to NOT use the display on the Seeeduino Xiao expansion board (just uses a Xiao connected to a LoRaE5 
// Grove expansion board. Seeed example code at https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/
//
//note: all Seeed LoRaE5 Grove boards have example code App key of "2B7E151628AED2A6ABF7158809CF4F3C" so needs to be changed
// 




//SBR modifications 28-DEC-2022 (file name: Xia_LoRaE5CayenneLPP_DS18B20tempSensorRev2.ino)
// remove anemometer related code and replace with DS18B20 temp sensor code
// SBR 25-JAN-2023 created new file name: Xiao_LoRaE5CayenneLPP_SEN0287avgCurrent.ino
// remove DS18B20 related code with SEN0287 related code

//SBR modifications 28-DEC-2022 (file name: Xiao_LoRaE5CayenneLPP_SEN0287avgCurrentLoadOnTime.ino)
// test to add in code to send out elapsed time (load on) as additional payload

#include <Arduino.h>
#include <CayenneLPP.h>   //library for Cayenne Low Power Payload LPP encoding for use in LoRa payload

// this version uses RunningAverage code
#include "RunningAverage.h"
RunningAverage myRA(100);
int samples = 0;

// general parameters
const int RecordTime = 3; //Define Measuring Time (Seconds)
const int ledPin =  13;      // the number of the LED pin

float currAvg;   //average current
float currMax; //max current
float currMin; //max current

// analog current sensor parameter
#define ACPin 1
#define VREF 3.3
#define ACTectionRange 5;    //set Non-invasive AC Current Sensor tection range (5A,10A,20A)
  


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

int transition = 0;  //transition defines state 2 when load ON=>OFF, 1 when load OFF=>ON, 0 when otherwise
unsigned long int timeOn;
unsigned long int timeOff;
unsigned long int elapsedTime = 0;
float onTime = 0;


 
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
    // Start up the library:
    //sensors.begin();
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    myRA.clear(); // explicitly start clean with RunningAverage by clearing buffer
 
    Serial1.begin(9600);                  //UART serial1 connection to LoRaE5
    Serial.print("E5 LORAWAN TEST\r\n");
 
    if (at_send_check_response("+AT: OK", 100, "AT\r\n"))
    {
        is_exist = true;
        //at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
        at_send_check_response("+MODE: LWOTAA", 1000, "AT+MODE=LWOTAA\r\n");
        //at_send_check_response("+DR: EU868", 1000, "AT+DR=EU868\r\n");  //original
        at_send_check_response("+DR: US915", 1000, "AT+DR=US915\r\n"); 
        //at_send_check_response("+CH: NUM", 1000, "AT+CH=NUM,0-2\r\n");  //original
        at_send_check_response("+CH: NUM", 1000, "AT+CH=NUM,8-15,65\r\n"); // configure channels to match chirpstack
        //at_send_check_response("+KEY: APPKEY", 1000, "AT+KEY=APPKEY,\"2B7E151628AED2A6ABF7158809CF4F3C\"\r\n");
        at_send_check_response("+CLASS: C", 1000, "AT+CLASS=A\r\n");
        at_send_check_response("+PORT: 8", 1000, "AT+PORT=8\r\n");
        at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
        at_send_check_response("+KEY: APPKEY", 1000, "AT+KEY=APPKEY,\"2B7E151628AED2A6ABF7158809CF4F3C\"\r\n");
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
     
    // measure SEN0287 current
    float AC_current = readACCurrentValue();
    Serial.print("transition state = "); 
    Serial.println(transition);
    delay(100);


    
    //measure();
    myRA.addValue(AC_current);
    samples++;
    delay(100 * RecordTime);
    //debug code to print out sample# and min, average, max current 
    Serial.print("samples = ");
    Serial.print(samples);
    Serial.print("\t");
    Serial.print("MinAvgMax  ");
    Serial.print(myRA.getMin(), 3);
    currMin = myRA.getMin();
    Serial.print("\t");
    Serial.print(myRA.getAverage(), 3);
    currAvg = myRA.getAverage();
    Serial.print("\t");
    Serial.println(myRA.getMax(), 3);
    currMax = myRA.getMax();
    Serial.println(AC_current, 3);
    

    //check if load turned off
    if ((transition == 2) && (AC_current < 0.9*currAvg))
      {       
       transition = 1;
       Serial.println("load turned off");      
      }

    //check if load turned on
    if ((transition == 0) && (AC_current > 1.1*currAvg))
      {
        timeOn= millis();
        transition = 2;
        samples = 0;
        Serial.println("load turned on");
      }

    //print out load on time if load turned off
    if (transition == 1)
      {
        timeOff = millis();
        elapsedTime = (timeOff - timeOn)/1000; 
        onTime = (elapsedTime/60 + 0.5);
        transition = 0;
        Serial.print(onTime);
        Serial.println("******* load on time in minutes ********");
        lpp.reset();
        lpp.addAnalogOutput(6, onTime);
        BuildPayload();
        samples = 0;  // reset samples to 0
        myRA.clear(); // clear myRA data
        // note: this starts sampling new current data again at the main loop beginning
      }

    
    // due to character byte limitatations in LoRa payload buffer, need to alternate between 

    
    Serial.print("Loop3 = ");  //debug code
    Serial.println(Loop3);    //debug code

    //if ((Loop3 == 0) and (samples == 30))
    if (samples == 30)
    //when samples = 30, add average current to LoRa payload 
    {
      lpp.reset();
      lpp.addAnalogOutput(5, currAvg);  //channel 5, average current
      Loop3 = 1;
      BuildPayload();
    }
    else if((Loop3 == 1) and (samples == 150))
    //when samples = 150, add average current to LoRa payload
    {
      lpp.reset();
      lpp.addAnalogOutput(5, currAvg);  //channel 5, average current
      Loop3 = 0;
      BuildPayload();
    }
    else
    {
      Serial.println("sample limits not reached");
    }

  // clear running average data after samples = 151   
  if (samples == 151)
  {
    samples = 0;
    myRA.clear();
    Serial.println("RunningAverage data cleared");
  }
    
}

// routine for reading current from DFRobot code for SEN0287
float readACCurrentValue()
{
  float ACCurrentValue = 0;
  float peakVoltage = 0;  
  float voltageVirtualValue = 0;  //Vrms
  for (int i = 0; i < 10; i++)
  {
    peakVoltage += analogRead(ACPin);   //read peak voltage
    delay(1);
  }
  peakVoltage = peakVoltage / 10;   //average out peak voltage
  peakVoltage = peakVoltage - 7.5; //calibrate out 0 current ADC reading (about 5mV)
  if (peakVoltage < 0.03) peakVoltage = 0; // zero out 0 current measurement
 
  voltageVirtualValue = peakVoltage * 0.707;    //change the peak voltage to the Virtual Value of voltage
  /*The circuit is amplified by 2 times, so it is divided by 2.*/
  voltageVirtualValue = (voltageVirtualValue / 1024 * VREF ) / 2;  
  ACCurrentValue = voltageVirtualValue * ACTectionRange;

  if (ACCurrentValue < 0.09) ACCurrentValue = 0; //zero out low current reading

  return ACCurrentValue;
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
    Serial.println(" in main loop checking LoRa then delay for 5 seconds");   
    delay(5000); // 5 seconds
    }
    else
    {
        delay(1000);
    }


}
