/***************************************************
                      INCLUDES
****************************************************/
#include <SPI.h>                  //SPI data
#include <MFRC522.h>              //RFID module
#include <ESP8266WiFi.h>          //Wi-Fi connection
#include <ESP8266HTTPClient.h>    //HTTP comm
#include <ArduinoJson.h>          //Json
/***************************************************
              VARIABLES & CONSTANTS
****************************************************/
#define RST_PIN  5                      //Pin 5 RC522 Reset
#define SS_PIN   15                     //Pin 15 RC522 SS (SDA) 

#define HTTP_OK  200                    //HTTP Response OK
const char* ssid = "BCG-PlazaP";        //Wi-Fi network name
const char* password = "Caramelatte12"; //Wi-Fi network password

MFRC522 mfrc522(SS_PIN, RST_PIN);       //RC522 object
/***************************************************
                    FUNCTIONS
****************************************************/
/*
----------------------
Initialize RFID module
----------------------
*/
void RFID_Init()
{
  Serial.println("RFID module init");
  SPI.begin();          //SPI Bus Initialization
  mfrc522.PCD_Init();   //MFRC522 Initialization
  Serial.println("RFID module initialized");
}
/*
---------------------------
Initialize Wi-Fi connection
---------------------------
*/
void WiFi_Init()
{
  Serial.println("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
}
/*
------------------
Read UID from RFID
------------------
*/
void RFID_readCard()
{
  if ( mfrc522.PICC_ReadCardSerial()) 
    {
      Serial.print("Card UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++)                 //Print card information
      {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);   
      } 
      Serial.println();
      mfrc522.PICC_HaltA();                                       //End card reading
    }   
}
/*
-----------------------------------------
Send info through Wi-Fi, HTTP request PUT
-----------------------------------------
*/
/* 
 bool WiFi_sendHttpRequest(RFID info, Temperature info, Weight info)
 bool WiFi_sendHttpRequest(byte *uid, float temp, float weight)
 DEFINIR ARGUMENTOS DEPENDIENDO DE LOS SENSORES A USAR.
*/
bool WiFi_sendHttpRequest()
{
  bool requestSent = false;
  if(WiFi.status()== WL_CONNECTED)
  {
    HTTPClient http;                                            //Create HTTP object
   
    http.begin("http://jsonplaceholder.typicode.com/posts/1");  
    http.addHeader("Content-Type", "text/plain");               
   
    int httpResponseCode = http.PUT("body");                    //PUT Request
   
    if(httpResponseCode > 0)                                    //Get response code
    {
      String response = http.getString();   
      Serial.println(response);     
      if(HTTP_OK == httpResponseCode)                           //Positive response code 
      {
        requestSent = true;
      }
    }
    else
    {
      Serial.print("Error on sending PUT Request: ");
    }
    Serial.println(httpResponseCode);
    http.end();
   }
   else
   {
      Serial.println("Error in WiFi connection");
   }
  delay(10000);
}
/*
--------------------------------------------
Receive info through Wi-Fi, HTTP request GET
--------------------------------------------
*/
bool WiFi_receiveHttpRequest()
{
  bool requestSent = false;
  if(WiFi.status()== WL_CONNECTED)
  {
    HTTPClient http;                                            //Create HTTP object
   
    http.begin("http://jsonplaceholder.typicode.com/posts/1");               
   
    int httpCode = http.GET();                                  //GET Request payload
   
    if(httpCode > 0)                                            //Get returning code
    {
      String payload = http.getString();   
      Serial.println(payload);     
    }
    http.end();
   }
   else
   {
      Serial.println("Error in WiFi connection");
   }
  delay(10000);
}
/***************************************************
                      SETUP
****************************************************/
void setup() {
  Serial.begin(115200);       //Serial comm init
  RFID_Init();                //RFID initialization
  WiFi_Init();                //Wi-Fi initialization
  Serial.println("Lectura del UID");
}
/***************************************************
                    MAIN LOOP
****************************************************/
void loop() {
  
  if ( mfrc522.PICC_IsNewCardPresent()) //Check if there is a new card
  {  
    RFID_readCard();                    //Read new card UID
    //WiFi_sendHttpRequest();             //Send info to server
  } 
}










