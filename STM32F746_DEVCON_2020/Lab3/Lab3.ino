#include <LwIP.h>
#include <STM32Ethernet.h>

void setup() {
  // initialize digital pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // give the ethernet module time to boot up:
  delay(1000);

  // start the Ethernet connection:
  Ethernet.begin();

  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
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