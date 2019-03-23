#include <ArduinoJson.h>

void setup() {
    Serial.begin(9600);
    const char * input = "{\"sensor\":\"gps\",\"time\":13204050,\"data\":[10,30]}";
    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc,input);

    if(err) {
        Serial.print("ERROR :");
        Serial.println(err.c_str());
        return;
    }

    const char* sensor = doc["sensor"];
    long time = doc["time"];
    float lat = doc["data"][0];
    float lon = doc["data"][1];
    Serial.println(sensor);
    Serial.println(time);
    Serial.println("LAT: ");Serial.print(lat);Serial.print(" LON: ");Serial.print(lon);
}

void loop() {
  
}
