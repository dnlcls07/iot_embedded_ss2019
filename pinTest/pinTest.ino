#include <SPI.h>                  //SPI data
#include <MFRC522.h>              //RFID module
#include <HX711_ADC.h>            //HX711 load
#include <Wire.h>                 //HX711 load

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
#define LOAD_A_FACTOR   1               //Calibration factor of load A
#define LOAD_B_FACTOR   1               //Calibration factor of load B

#define MATERIAL_POS_1  0x01            //Mask position of material 1
#define MATERIAL_POS_2  0x02            //Mask position of material 2

MFRC522 mfrc522(SS_PIN, RST_PIN);               //RC522 object
HX711_ADC LoadACell(LOAD_A_DATA, LOAD_A_SCK);   //Load A cell object (data pin, sck pin)
HX711_ADC LoadBCell(LOAD_B_DATA, LOAD_B_SCK);   //Load B cell object (data pin, sck pin)

void setup() {
  
 // Serial.begin(9600);         //Serial comm init
  //Serial.println("Initialization");

  pinMode(LED_1_PIN, OUTPUT);       //Set LED_1_PIN as output.
  pinMode(LED_2_PIN, OUTPUT);       //Set LED_2_PIN as output. Pin 9 manda a reset el micro. NO USAR!!!!

  digitalWrite(LED_1_PIN, LOW);     //Set LED_1_PIN value 0
  digitalWrite(LED_2_PIN, LOW);     //Set LED_2_PIN value 0 Pin 9 manda a reset el micro. NO USAR!!!!  
  Serial.println("Sensor pins initialized");

  Serial.println("Load cells");
  LoadACell.begin();                        //Start connection to HX711
  LoadACell.start(2000);                    //Load cells gets 2000ms of time to stabilize
  LoadACell.setCalFactor(LOAD_A_FACTOR);    //Calibration factor for load cell
  Serial.println("Load A initialized");
  LoadBCell.begin();                        //Start connection to HX711
  LoadBCell.start(2000);                    //Load cells gets 2000ms of time to stabilize
  LoadBCell.setCalFactor(LOAD_B_FACTOR);    //Calibration factor for load cell
  Serial.println("Load B initialized");

  delay(2000);
  Serial.println("Passed 2 seconds");
  
  SPI.begin();          //SPI Bus Initialization
  mfrc522.PCD_Init();   //MFRC522 Initialization
  Serial.println("RFID module initialized");
}

void loop() {
  
  digitalWrite(LED_1_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_1_PIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

  digitalWrite(LED_2_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_2_PIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

}
