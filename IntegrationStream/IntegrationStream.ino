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
#define LED_2_PIN       12              //Led 2 pin 12
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

#define MATERIAL_POS_1  0x01            //Mask position of material 1
#define MATERIAL_POS_2  0x02            //Mask position of material 2
#define MATERIAL_POS_3  0x04            //Mask position of material 3
#define MATERIAL_POS_4  0x08            //Mask position of material 4
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
  pinMode(LED_2_PIN, OUTPUT);       //Set LED_2_PIN as output. Pin 9 manda a reset el micro. NO USAR!!!!
  pinMode(LED_3_PIN, OUTPUT);       //Set LED_3_PIN as output
  pinMode(LED_4_PIN, OUTPUT);       //Set LED_4_PIN as output
  pinMode(LED_ERROR_PIN, OUTPUT);   //Set LED_ERROR_PIN as output

  digitalWrite(LED_1_PIN, LOW);     //Set LED_1_PIN value 0
  digitalWrite(LED_2_PIN, LOW);     //Set LED_2_PIN value 0 Pin 9 manda a reset el micro. NO USAR!!!!
  digitalWrite(LED_3_PIN, LOW);     //Set LED_3_PIN value 0
  digitalWrite(LED_4_PIN, LOW);     //Set LED_4_PIN value 0
  digitalWrite(LED_ERROR_PIN, LOW); //Set LED_ERROR_PIN value 0
  */
  Serial.println("Sensor pins initialized");
}
/*
---------------------------------------
Turn on LEDs depending on data received
---------------------------------------
*/
void SENSOR_changeLedState(int leds, int state)
{
   if(material_to_take & MATERIAL_POS_1)
  {
    digitalWrite(LED_1_PIN, state);     
  }
  if(material_to_take & MATERIAL_POS_2)
  {
    digitalWrite(LED_2_PIN, state);
  }
  if(material_to_take & MATERIAL_POS_3)
  {
    digitalWrite(LED_3_PIN, state);
  }
  if(material_to_take & MATERIAL_POS_4)
  {
    digitalWrite(LED_4_PIN, state);
  }
}
/*
----------------------------------------------------
Receive cloud data and specify wich material to take
----------------------------------------------------
*/
void SENSOR_showMaterial(int material_to_take)
{
  SENSOR_changeLedState(, HIGH);
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
--------------------------------------------
Receive info through Wi-Fi, HTTP request GET
--------------------------------------------
*/

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
    card_id = RFID_readCard();                    //Read new card UID
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
}
/***************************************************
                    CHECK-OUT LOOP
****************************************************/
void loop_checkout(long rfidCard)
{
  int material_to_use = 0;
  //enviar rfidCard a nube y recibir data
  SENSOR_showMterial(material_to_use)
  
}
/***************************************************
                    CHECK-IN LOOP
****************************************************/
void loop_checkin(long rfidCard)
{
  
}






