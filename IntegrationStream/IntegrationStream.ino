/***************************************************
                      INCLUDES
****************************************************/
#include <SPI.h>                  //SPI data
#include <MFRC522.h>              //RFID module
#include <ESP8266WiFi.h>          //Wi-Fi connectio
#include <HX711_ADC.h>            //HX711 load
#include <Wire.h>                 //HX711 load
#include <ESP8266HTTPClient.h>    //HTTP comm
#include <ArduinoJson.h>          //Json
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
#define LED_4_PIN       13              //Led 4 pin 13
#define LED_ERROR_PIN   16              //Error led pin 16 (In mcu)
#define LOAD_A_DATA     1               //Load A pin 1 - Data
#define LOAD_A_SCK      3               //Load A pin 3 - Clock
#define LOAD_B_DATA     0               //Load B pin 1 - Data
#define LOAD_B_SCK      2               //Load B pin 3 - Clock

#define ID_LIST_NUM     5               //ID list max number
#define LOAD_A_FACTOR   999             //Calibration factor of load A
#define LOAD_B_FACTOR   999             //Calibration factor of load B
/***************************************************
               VARIABLES & CONSTANTS
****************************************************/
const char* ssid = "BCG-PlazaP";                //Wi-Fi network name
const char* password = "Caramelatte12";         //Wi-Fi network password

long id_list[ID_LIST_NUM] = {0};                //List of ID's

MFRC522 mfrc522(SS_PIN, RST_PIN);               //RC522 object
HX711_ADC LoadACell(LOAD_A_DATA, LOAD_A_SCK);   //Load A cell object (data pin, sck pin)
HX711_ADC LoadBCell(LOAD_B_DATA, LOAD_B_SCK);   //Load B cell object (data pin, sck pin)
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
  /*
  LoadACell.begin();                        //Start connection to HX711
  LoadACell.start(2000);                    //Load cells gets 2000ms of time to stabilize
  LoadACell.setCalFactor(LOAD_A_FACTOR);    //Calibration factor for load cell
  LoadBCell.begin();                        //Start connection to HX711
  LoadBCell.start(2000);                    //Load cells gets 2000ms of time to stabilize
  LoadBCell.setCalFactor(LOAD_B_FACTOR);    //Calibration factor for load cell
  */
  pinMode(LED_1_PIN, OUTPUT);       //Set LED_1_PIN as output.
  digitalWrite(LED_1_PIN, LOW);     //Set LED_1_PIN value 0
  //pinMode(LED_2_PIN, OUTPUT);       //Set LED_2_PIN as output. Pin 9 manda a reset el micro. NO USAR!!!!
 /* pinMode(LED_3_PIN, OUTPUT);       //Set LED_3_PIN as output
  pinMode(LED_4_PIN, OUTPUT);       //Set LED_4_PIN as output
  pinMode(LED_ERROR_PIN, OUTPUT);   //Set LED_ERROR_PIN as output
  pinMode(PB_1_PIN, INPUT_PULLUP);  //Set PB_1_PIN as input with pull up resistor
  pinMode(PB_2_PIN, INPUT_PULLUP);  //Set PB_2_PIN as input with pull up resistor

  digitalWrite(LED_1_PIN, LOW);     //Set LED_1_PIN value 0
  //digitalWrite(LED_2_PIN, LOW);     //Set LED_2_PIN value 0
  digitalWrite(LED_3_PIN, LOW);     //Set LED_3_PIN value 0
  digitalWrite(LED_4_PIN, LOW);     //Set LED_4_PIN value 0
  digitalWrite(LED_ERROR_PIN, LOW); //Set LED_ERROR_PIN value 0
  */
  Serial.println("Sensor pins initialized");
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
  SPI.begin();          //SPI Bus Initialization
  mfrc522.PCD_Init();   //MFRC522 Initialization
  Serial.println("RFID module initialized");
}
/*
------------------
Read UID from RFID
return: long of card id 
------------------
*/
long RFID_readCard()
{
  long id_card = 0; 
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
bool RFID_check_id_list(long id_to_look)
{
  bool id_added = true;
  byte i;
  for(i = 0 ; i<ID_LIST_NUM ; i++)
  {
    if(id_to_look == id_list[i])
    {
      id_added = false;
      id_list[i] = 0;
      break;
    }
  }
  if(true == id_added)
  {
    for(i = 0 ; i<ID_LIST_NUM ; i++)
    {
      if(0 == id_list[i])
      {
        id_list[i] = id_to_look;
        break;
      } 
    }
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
      if(200 == httpResponseCode)                           //Positive response code 
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
  Serial.begin(9600);         //Serial comm init
  Serial.println("Initialization");
  SENSOR_Init();              //Push buttons, LEDS and LOAD sensors initialization
  RFID_Init();                //RFID initialization
  //WiFi_Init();                //Wi-Fi initialization
  Serial.println("Lectura del UID");
}
/***************************************************
                    MAIN LOOP
****************************************************/
void loop() 
{
  long card_id;
  bool card_added;
  if ( mfrc522.PICC_IsNewCardPresent())           //Check if there is a new card
  {  
    Serial.println("Start");
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
/***************************************************
                    CHECK-OUT LOOP
****************************************************/
void loop_checkout()
{
  Serial.println("checkout");
    digitalWrite(LED_1_PIN, HIGH);     //Set LED_1_PIN value 0    
}
/***************************************************
                    CHECK-IN LOOP
****************************************************/
void loop_checkin()
{
  Serial.println("checkin");
    digitalWrite(LED_1_PIN, LOW);     //Set LED_2_PIN value 0
}






