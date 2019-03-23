#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
 
const char* ssid = "BCG-PlazaP";
const char* password = "Caramelatte12";
 
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
 
 if(WiFi.status()== WL_CONNECTED){
 
   HTTPClient http;   
 
   http.begin("http://jsonplaceholder.typicode.com/posts/1");
   http.addHeader("Content-Type", "text/plain");            
 
   int httpResponseCode = http.PUT("body");   
 
   if(httpResponseCode > 0){
 
    String response = http.getString();   
 
    Serial.println(httpResponseCode);
    Serial.println(response);          
 
   }else{
 
    Serial.print("Error on sending PUT Request: ");
    Serial.println(httpResponseCode);
 
   }
 
   http.end();
 
 }else{
    Serial.println("Error in WiFi connection");
 }
 
  delay(10000);
}
 
