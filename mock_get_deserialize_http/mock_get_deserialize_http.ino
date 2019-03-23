#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "BCG-PlazaP";
const char* password = "Caramelatte12";
StaticJsonDocument<350> rcv_doc;

void setup() {
  Serial.begin(115200);
  delay(10);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
Serial.println("");
Serial.println("WiFi connected");
}

void loop() {

if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

HTTPClient http;  //Declare an object of class HTTPClient

http.begin("http://jsonplaceholder.typicode.com/posts/1");  //Specify request destination
int httpCode = http.GET();                                                                  //Send the request

if (httpCode > 0) { //Check the returning code -> &gt es > &lt es <

String payload = http.getString();   //Get the request response payload
Serial.println(payload); //Print the response payload

DeserializationError error = deserializeJson(rcv_doc, payload);
if (error) {
   Serial.print(F("deserializeJson() failed: "));
   Serial.println(error.c_str());
   delay(30000);
   return;
}

int userId = rcv_doc["userId"];
int id = rcv_doc["id"];
const char* title = rcv_doc["title"];
const char* body = rcv_doc["body"];

Serial.print("User ID: "); Serial.println(userId);
Serial.print("ID: "); Serial.println(id);
Serial.print("Title: "); Serial.println(title);
Serial.print("Body: "); Serial.println(body);
}

http.end();   //Close connection
}

delay(30000);    //Send a request every 30 seconds

}
