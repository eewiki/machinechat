#include <LwIP.h>
#include <STM32Ethernet.h>

#include <STTS751Sensor.h>
#include <HTS221Sensor.h>
#include <LPS22HHSensor.h>

#include <ArduinoJson.h>

STTS751Sensor *STTS751_Temp;
HTS221Sensor *HTS221_HumTemp;
LPS22HHSensor *LPS22HH_PressTemp;

// Create a unique ID for the data from each STM32 running this code
const char* jediID = "STM32F7_IKSO1A3";

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

  // Initialize HTS221 Sensor 
  HTS221_HumTemp = new HTS221Sensor (&Wire);
  HTS221_HumTemp->Enable();

  // Initialize LPS22HH Sensor 
  LPS22HH_PressTemp= new LPS22HHSensor(&Wire);
  LPS22HH_PressTemp->Enable();

  // give the ethernet module time to boot up:
  delay(1000);

  // start the Ethernet connection:
  Ethernet.begin();

  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  String postData;

  //Read STTS751 Temperature
  float STTS751_tempC = 0;
  STTS751_Temp->GetTemperature(&STTS751_tempC);
  float STTS751_tempF = (STTS751_tempC * 1.8) + 32.0F;

  // Read HTS221 Humidity and Temperature
  float HTS221_humidity = 0, HTS221_tempC = 0;
  HTS221_HumTemp->GetHumidity(&HTS221_humidity);
  HTS221_HumTemp->GetTemperature(&HTS221_tempC);
  float HTS221_tempF = (HTS221_tempC * 1.8) + 32.0F;

  // Read LPS22HH Pressure and Temperature.
  float LPS22HH_pressure = 0, LPS22HH_tempC = 0;
  LPS22HH_PressTemp->GetPressure(&LPS22HH_pressure);
  LPS22HH_PressTemp->GetTemperature(&LPS22HH_tempC);
  float LPS22HH_tempF = (LPS22HH_tempC * 1.8) + 32.0F;

  Serial.print(" | Temp[F]: ");
  Serial.print(STTS751_tempF, 2);
  Serial.print(" | Temp[F]: ");
  Serial.print(HTS221_tempF, 2);
  Serial.print(" | Temp[F]: ");
  Serial.print(LPS22HH_tempF , 2);
  Serial.print("| Hum[%]: ");
  Serial.print(HTS221_humidity, 2); 
  Serial.print(" | Pres[hPa]: ");
  Serial.print(LPS22HH_pressure, 2); 
  Serial.println(" |");
  
  StaticJsonDocument <200> doc;
  
  JsonObject context = doc.createNestedObject("context");
  context["target_id"] = String(jediID);

  JsonObject data = doc.createNestedObject("data");
  data["HTS221_humidity"] = HTS221_humidity;
  data["HTS221_tempF"] = HTS221_tempF;
  data["LPS22HH_pressure"] = LPS22HH_pressure;
  data["LPS22HH_tempF"] = LPS22HH_tempF;
  data["STTS751_tempF"] = STTS751_tempF;

  serializeJson(doc, postData);

  //This prints the JSON to the serial monitor screen
  Serial.println(postData);

  if (Ethernet.linkStatus() == LinkON) {
    Serial.println("Link status: On");

    digitalWrite(LED_BUILTIN, HIGH);

    String contentType = "application/json";

    client.post("/v1/data/mc", contentType, postData);

    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
  
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
  }
  else if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Link status: Off");

    digitalWrite(LED_BUILTIN, LOW);
  }  
  delay(500);
}