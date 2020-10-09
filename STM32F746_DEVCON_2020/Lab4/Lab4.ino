#include <LwIP.h>
#include <STM32Ethernet.h>

#include <STTS751Sensor.h>

STTS751Sensor *STTS751_Temp;

void setup() {
  // initialize digital pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize I2C bus.
  Wire.begin();

  // Initialize STTS751 Sensor 
  STTS751_Temp = new STTS751Sensor (&Wire);
  STTS751_Temp->Enable();

  // give the ethernet module time to boot up:
  delay(1000);

  // start the Ethernet connection:
  Ethernet.begin();

  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  //Read STTS751 Temperature
  float STTS751_tempC = 0;
  STTS751_Temp->GetTemperature(&STTS751_tempC);
  float STTS751_tempF = (STTS751_tempC * 1.8) + 32.0F;

  Serial.print(" | Temp[F]: ");
  Serial.print(STTS751_tempF, 2);
  Serial.println(" |");

  if (Ethernet.linkStatus() == LinkON) {
    Serial.println("Link status: On");

    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Link status: Off");

    digitalWrite(LED_BUILTIN, LOW);
  }  
  delay(500);
}