// below code is based on Seeed LoRaE5 example code and modified to NOT use the display on the Seeeduino Xiao expansion board (just uses a Xiao connected to a LoRaE5 
// Grove expansion board. Seeed example code at https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/
//
//note: all Seeed LoRaE5 Grove boards have example code App key of "2B7E151628AED2A6ABF7158809CF4F3C" so needs to be changed
// this version changes out the DHT11 sensor for Sparkfun weather meter kit for wind speed and direction

// Anemometer measuring code based on below:
//More Information at: https://www.aeq-web.com/
//Version 2.0 | 11-NOV-2020
//
//SBR modifications
//SensorPin is connected to RC pulldown circuit on anemometer switch
//Wind vane pin is connected to voltage divider circuit for vane resistors


#include <Arduino.h>
#include <CayenneLPP.h>


// anemometer and direction parameters
const int RecordTime = 3; //Define Measuring Time (Seconds)
const int SensorPin = 2;     // the number of the sensorpin
const int VanePin = 1;      // pin# connected to wind vane
const int ledPin =  13;      // the number of the LED pin
int InterruptCounter;
float WindSpeed;
float DirWind;   //wind direction voltage
float angleWind; //wind vane angle
  

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

    // anemeometer and wind vane setup
    pinMode(SensorPin, INPUT); //use input to count interrupts of anemometer
    pinMode(VanePin, INPUT); //use input to read analog voltage of wind vane output
 
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
     
    // measure windspeed
    measure();
    // measure wind vane voltage
    DirWind = analogRead(VanePin)*3.33;
    // calculate wind direction in degrees
    vaneAngle();
    
    
    Serial.print("Windspeed: ");
    Serial.print(WindSpeed);
    Serial.print(" mph ");
    Serial.print("Wind Direction: ");
    Serial.print(angleWind);
    Serial.println(" degrees");


//***************** start of Cayenne LPP code **************************
    // due to character byte limitatations in LoRa payload buffer, need to alternate between 
    // the two wind parameters when sending out parameter payload
    
    Serial.print("Loop3 = ");  //debug code
    Serial.println(Loop3);    //debug code

    if (Loop3 == 0)
    {
      lpp.reset();
      lpp.addAnalogOutput(1, WindSpeed);  //channel 1, windspeed value
      Loop3 = 1;
    }
    else
    {
      lpp.reset();
      lpp.addAnalogOutput(2, angleWind);  //channel 2, wind direction value
      Loop3 = 0;
    }
   

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
    Serial.println(" in main loop checking LoRa then wait 5 minutes");   
    delay(300000); //main delay 300 seconds 
    }
    else
    {
        delay(1000);
    }
}


//count anemometer pulses and calculate windspeed
void measure() {
  InterruptCounter = 0;
  attachInterrupt(digitalPinToInterrupt(SensorPin), countup, RISING);
  delay(1000 * RecordTime);
  detachInterrupt(digitalPinToInterrupt(SensorPin));
  WindSpeed = (float)InterruptCounter / (float)RecordTime * 2.4;
}

void countup() {
  InterruptCounter++;
}

//determine wind direction angle based on measured wind vane voltage
void vaneAngle() {
  angleWind = 99.9;
  if (DirWind < 270)
    {
      angleWind = 112.5;
    }
  else if ((DirWind >= 270) and (DirWind <= 310))
    {
      angleWind = 67.5  ;
    } 
   else if ((DirWind > 310) and (DirWind <= 400))
    {
      angleWind = 90.0  ;
    }    
  else if ((DirWind >= 400) and (DirWind < 600))
    {
      angleWind = 157.5  ;
    }    
  else if ((DirWind >= 600) and (DirWind < 750))
    {
      angleWind = 135.0  ;
    }    
  else if ((DirWind >= 750) and (DirWind < 850))
    {
      angleWind = 202.5  ;
    }    
  else if ((DirWind >= 850) and (DirWind < 1150))
    {
      angleWind = 180.0  ;
    }    
   else if ((DirWind >= 1150) and (DirWind < 1450))
    {
      angleWind = 22.5  ;
    }    
   else if ((DirWind >= 1450) and (DirWind < 1900))
    {
      angleWind = 45.0  ;
    }    
  else if ((DirWind >= 1900) and (DirWind < 2050))
    {
      angleWind = 247.5  ;
    }    
   else if ((DirWind >= 2050) and (DirWind < 2250))
    {
      angleWind = 225.0  ;
    }    
  else if ((DirWind >= 2250) and (DirWind < 2500))
    {
      angleWind = 337.5;
    }
  else if ((DirWind >= 2500) and (DirWind < 2700))
    {
      angleWind = 0.0;
    }
  else if ((DirWind >= 2700) and (DirWind < 2850))
    {
      angleWind = 292.5;
    }
  else if ((DirWind >= 2850) and (DirWind < 3000))
    {
      angleWind = 315.0;
    }
  else if (DirWind >= 3000)
    {
      angleWind = 270.0;
    }
}  
