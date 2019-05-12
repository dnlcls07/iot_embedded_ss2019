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
#define LED_1_PIN       1               //Led 1 pin 1
#define LED_2_PIN       3               //Led 2 pin 3
#define LED_ERROR_PIN   16              //Error led pin 16 (In mcu)-
#define LOAD_A_DATA     4               //Load A pin 4 - Data
#define LOAD_A_SCK      16              //Load A pin 16 - Clock
#define LOAD_B_DATA     0               //Load B pin 1 - Data
#define LOAD_B_SCK      2               //Load B pin 3 - Clock

#define ID_LIST_NUM     5               //ID list max number
#define LOAD_A_FACTOR   -1913.27         //Calibration factor of load A
#define LOAD_B_FACTOR   2157.6          //Calibration factor of load B

#define MATERIAL_POS_1  0x01            //Mask position of material 1
#define MATERIAL_POS_2  0x02            //Mask position of material 2

/***************************************************
               VARIABLES & CONSTANTS
****************************************************/
const char* ssid = "INFINITUM3587";   //itesoIoT
const char* password = "123456789";  //1t3s0IoT18

uint32_t id_list[ID_LIST_NUM] = {0};                //List of ID's
StaticJsonDocument<200> doc_send;
StaticJsonDocument<200> doc_receive;

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
  LoadACell.begin();                        //Start connection to HX711
  LoadACell.start(2000);                    //Load cells gets 2000ms of time to stabilize
  if(LoadACell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadACell.setCalFactor(LOAD_A_FACTOR); // set calibration value (float)
    Serial.println("Startup + tare is complete");
  }
  
  Serial.println("Load A initialized");
  LoadBCell.begin();                        //Start connection to HX711
  LoadBCell.start(2000);                    //Load cells gets 2000ms of time to stabilize
  if(LoadACell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadBCell.setCalFactor(LOAD_B_FACTOR); // set calibration value (float)
    Serial.println("Startup + tare is complete");
  }
  Serial.println("Load B initialized");
  
  //pinMode(LED_1_PIN, OUTPUT);       //Set LED_1_PIN as output.
  //pinMode(LED_2_PIN, OUTPUT);       //Set LED_2_PIN as output. Pin 9 manda a reset el micro. NO USAR!!!!

  //digitalWrite(LED_1_PIN, LOW);     //Set LED_1_PIN value 0
  //digitalWrite(LED_2_PIN, LOW);     //Set LED_2_PIN value 0 Pin 9 manda a reset el micro. NO USAR!!!!  
  Serial.println("Sensor pins initialized");
}
/*
---------------------------------------
Turn on LEDs depending on data received
---------------------------------------
*/
void SENSOR_changeLedState(int material_to_take, int state)
{
   if(material_to_take & MATERIAL_POS_1)
  {
    digitalWrite(LED_1_PIN, state); 
    // TODO: remove   
    Serial.println("Take first material"); 
  }
  if(material_to_take & MATERIAL_POS_2)
  {
    digitalWrite(LED_2_PIN, state);
    // TODO: remove
    Serial.println("Take second material"); 
  }
}
/*
----------------------------------------------------
Receive cloud data and specify wich material to take
----------------------------------------------------
*/
void SENSOR_showMaterial(int material_to_take)
{
  SENSOR_changeLedState(material_to_take, HIGH);
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
return: uint32_t of card id 
------------------
*/
uint32_t RFID_readCard()
{
  uint32_t id_card = 0; 
  if ( mfrc522.PICC_ReadCardSerial()) 
    {
      for (byte i = 0; i < mfrc522.uid.size; i++)                 //Print card information
      {   
        id_card = ((id_card << (8)) | mfrc522.uid.uidByte[i]);
      } 
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
bool RFID_check_id_list(uint32_t id_to_look)
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
uint32_t WiFi_Send(uint32_t id, float weight_0, float weight_1, String transaction)
{
   String string_to_send = "";
   uint32_t parsed_data;
   
   if(WiFi.status()== WL_CONNECTED){
   HTTPClient http;   
   http.begin("http://2806b32d.ngrok.io/weight");
   http.addHeader("Content-Type", "application/json");            
   doc_send["id"] = id;
   doc_send["peso0"] = weight_0;
   doc_send["peso1"] = weight_1;
   doc_send["transaction"] = transaction; 
   serializeJson(doc_send,string_to_send);

   Serial.println(string_to_send);
   int httpResponseCode = http.PUT(string_to_send);   
   string_to_send = "";
 
   if(httpResponseCode > 0){
 
    String response = http.getString();
    DeserializationError err = deserializeJson(doc_receive,response);
    parsed_data = doc_receive["data"];
 
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
 return parsed_data;
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
}
/***************************************************
                    MAIN LOOP
****************************************************/
void loop() 
{
  uint32_t card_id;
  bool card_added;
  if ( mfrc522.PICC_IsNewCardPresent())           //Check if there is a new card
  {  
    card_id = RFID_readCard();                    //Read new card UID
    Serial.print("Card ID:");
    Serial.println(card_id);
    card_added = RFID_check_id_list(card_id);     //Verify ID's list
    
    if(true == card_added)                        //CHECKOUT
    {
      loop_checkout(card_id);
    }
    else                                          //CHECKIN
    {
      loop_checkin(card_id);
    }
  } 
  LoadACell.update();
  LoadBCell.update();
}
/***************************************************
                    CHECK-OUT LOOP
****************************************************/
void loop_checkout(uint32_t rfidCard)
{
  String material_received = "";
  float weight_0 = LoadACell.getData();
  float weight_1 = LoadBCell.getData();
  Serial.print("A: ");
  Serial.println(weight_0);
  Serial.print("B: ");
  Serial.println(weight_1);
  material_received = WiFi_Send(rfidCard, weigth_0, weigth_1, "out");
  SENSOR_showMaterial(material_received.toInt());
}
/***************************************************
                    CHECK-IN LOOP
****************************************************/
void loop_checkin(uint32_t rfidCard)
{
  String response = "";
  float weight_0 = LoadACell.getData();
  float weight_1 = LoadBCell.getData();
  Serial.print("A: ");
  Serial.println(weight_0);
  Serial.print("B: ");
  Serial.println(weight_1);
  material_received = WiFi_Send(rfidCard, weigth_0, weigth_1, "in");
}
