/***************************************************
                      INCLUDES
****************************************************/
#include <SPI.h>                  //SPI data
#include <MFRC522.h>              //RFID module
#include <ESP8266WiFi.h>          //Wi-Fi connection
#include <ESP8266HTTPClient.h>    //HTTP comm
//#include <ArduinoJson.h>          //Json
/***************************************************
                   DEFINES
****************************************************/
#define RST_PIN         5               //RFID Pin 5 RC522 Reset
#define SS_PIN          15              //RFID Pin 15 RC522 SS (SDA) 
#define PB_1_PIN        12              //Push Button 1 pin 12
#define PB_2_PIN        14              //Push Button 2 pin 14
#define LED_1_PIN       4               //Led 1 pin 4
#define LED_2_PIN       9               //Led 2 pin 9
#define LED_3_PIN       10              //Led 3 pin 10
#define LED_4_PIN      13              //Led 4 pin 13
#define LED_ERROR_PIN   16              //Error led pin 16 (In mcu)
//TODO: Falta inicializar los pines de sensores de peso. Pines 1 y 3 pensados para eso. Estos pines pueden ser serial. RXD0 y TXD0

#define HTTP_OK       200               //HTTP Response OK
#define ID_LIST_NUM   10                //ID list max number
/***************************************************
               VARIABLES & CONSTANTS
****************************************************/
const char* ssid = "BCG-PlazaP";        //Wi-Fi network name
const char* password = "Caramelatte12"; //Wi-Fi network password

int id_list[ID_LIST_NUM] = {0};         //List of ID's
byte actual_id_num = 0;                 //Number of id's on ID's list

MFRC522 mfrc522(SS_PIN, RST_PIN);       //RC522 object
/***************************************************
                    FUNCTIONS
****************************************************/
/*
-------------------------------------
Initialize Sensors and Actuators pins
-------------------------------------
*/
void SENSOR_Init()
{
  pinMode(LED_1_PIN, OUTPUT);       //Set LED_1_PIN as output
  pinMode(LED_2_PIN, OUTPUT);       //Set LED_2_PIN as output
  pinMode(LED_3_PIN, OUTPUT);       //Set LED_3_PIN as output
  pinMode(LED_4_PIN, OUTPUT);       //Set LED_4_PIN as output
  pinMode(LED_ERROR_PIN, OUTPUT);   //Set LED_ERROR_PIN as output
  pinMode(PB_1_PIN, INPUT_PULLUP);  //Set PB_1_PIN as input with pull up resistor
  pinMode(PB_2_PIN, INPUT_PULLUP);  //Set PB_2_PIN as input with pull up resistor

  digitalWrite(LED_1_PIN, LOW);     //Set LED_1_PIN value 0
  digitalWrite(LED_2_PIN, LOW);     //Set LED_2_PIN value 0
  digitalWrite(LED_3_PIN, LOW);     //Set LED_3_PIN value 0
  digitalWrite(LED_4_PIN, LOW);     //Set LED_4_PIN value 0
  digitalWrite(LED_ERROR_PIN, LOW); //Set LED_ERROR_PIN value 0

  //TODO: Falta inicializar los sensores de peso
}
/*
------------------------
Read push buttons values
------------------------
*/
void SENSOR_read_pushButtons()
{ 
  //TODO: Agregar dato de retorno 
  byte push_buttons_pressed = 0;
  digitalRead(PB_1_PIN);
  digitalRead(PB_2_PIN);
}
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
------------------
Read UID from RFID
return: int of card id 
------------------
*/
int RFID_readCard()
{
  int id_card = 0; 
  if ( mfrc522.PICC_ReadCardSerial()) 
    {
      Serial.print("Card UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++)                 //Print card information
      {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);   
        id_card = ((id_card << (8)) | mfrc522.uid.uidByte[i]);
      } 
      Serial.println(id_card);
      mfrc522.PICC_HaltA();                                       //End card reading
    }   
    return id_card;
}
/*
--------------------------------------------------
Check list of ID's that already asked for material 
return: true:   new card id added (chekout)
    false:  card id already on list (checkin)
--------------------------------------------------
*/
bool RFID_check_id_list(int id_to_look)
{
  bool id_added = true;
  byte i;
  for(i = 0 ; i<actual_id_num ; i++)
  {
    if(id_to_look == id_list[i])
    {
      id_added = false;
      break;
    }
  }
  if(true == id_added)
  {
    id_list[actual_id_num] = id_to_look;
    actual_id_num++;
  }
  return id_added; 
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
  //delay(10000);
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
  //delay(10000);
}
/***************************************************
                      SETUP
****************************************************/
void setup() 
{
  Serial.begin(115200);       //Serial comm init
  SENSOR_Init();              //Push buttons, LEDS and weight sensors initialization
  RFID_Init();                //RFID initialization
  WiFi_Init();                //Wi-Fi initialization
  SENSOR_Init();             //Sensors initialization
  Serial.println("Lectura del UID");
}
/***************************************************
                    CHECK-OUT LOOP
****************************************************/
void loop_checkout()
{

}
/***************************************************
                    CHECK-IN LOOP
****************************************************/
void loop_checkin()
{
  
}
/***************************************************
                    MAIN LOOP
****************************************************/
void loop() 
{
  int card_id;
  bool card_added;

  if ( mfrc522.PICC_IsNewCardPresent())           //Check if there is a new card
  {  
    card_id = RFID_readCard();                    //Read new card UID
    card_added = RFID_check_id_list(card_id);     //Verify ID's list
    if(true == card_added)                        //CHECKOUT
    {
      loop_checkout();
    }
    else                                          //CHECKIN
    {
      loop_checkin();
    }
  } 
}






