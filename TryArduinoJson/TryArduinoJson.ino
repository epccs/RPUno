/*
    by  Ronald Sutherland

    referances:
        ArduinoJson by Beno�t Blanchon
            https://github.com/bblanchon/ArduinoJson
            https://github.com/bblanchon/ArduinoJson/wiki/Using-the-library-with-Arduino
        F() macro and PROGMEM techniques save SRAM
            http://www.gammon.com.au/progmem

*/

#include <ArduinoJson.h>


void setup(void)
{  
  // Start Serial
  Serial.begin(115200);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& array = jsonBuffer.createObject();
  array["sensor"] = "gps";
  array["time"] = 1351824120;
  JsonArray& data = array.createNestedArray("data");
  data.add(48.756080, 6);  // 6 is the number of decimals to print
  data.add(2.302038, 6);   // if not specified, 2 digits are printed

  // Send to Serial
  array.printTo(Serial);
  Serial.println();

  // same data in a local array
  char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";

  // Deserialize the JSON string
  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success())
  {
    Serial.println(F("parseObject() failed"));
    return;
  }

  // check the values
  const char* sensor    = root["sensor"];
  Serial.print(F("sensor: "));
  Serial.println(sensor);
  long        time      = root["time"];
  Serial.print(F("time: "));
  Serial.println(time);
  double      latitude  = root["data"][0];
  Serial.print(F("latitude: "));
  Serial.println(latitude);
  double      longitude = root["data"][1];
  Serial.print(F("longitude: "));
  Serial.println(longitude);
}

void loop() 
{

}
