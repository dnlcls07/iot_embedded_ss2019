/*
#include <HX711_ADC.h> // https://github.com/olkal/HX711_ADC
#include <Wire.h>

#define LOAD_A_FACTOR 50
HX711_ADC LoadACell(4, 16); // parameters: dt pin, sck pin<span data-mce-type="bookmark" style="display: inline-block; width: 0px; overflow: hidden; line-height: 0;" class="mce_SELRES_start"></span>
HX711_ADC LoadBCell(0, 2); // parameters: dt pin, sck pin<span data-mce-type="bookmark" style="display: inline-block; width: 0px; overflow: hidden; line-height: 0;" class="mce_SELRES_start"></span>

void setup() {
  Serial.begin(9600);
  Serial.print("A:");
  LoadACell.begin(); // start connection to HX711
  LoadACell.start(2000); // load cells gets 2000ms of time to stabilize
  LoadACell.setCalFactor(LOAD_A_FACTOR); // calibration factor for load cell => strongly dependent on your individual setup
  Serial.print("B:");
  LoadBCell.begin(); // start connection to HX711
  LoadBCell.start(2000); // load cells gets 2000ms of time to stabilize
  LoadBCell.setCalFactor(LOAD_A_FACTOR); 
}

void loop() {

  delay(1000);
  Serial.print("A:");
  LoadACell.update(); // retrieves data from the load cell
  float iA = LoadACell.getData(); // get output value
  Serial.println(iA);

  Serial.print("B:");
  LoadBCell.update(); // retrieves data from the load cell
  float iB = LoadBCell.getData(); // get output value
  Serial.println(iB);
  
}

*/

//-------------------------------------------------------------------------------------
// HX711_ADC.h
// Arduino master library for HX711 24-Bit Analog-to-Digital Converter for Weigh Scales
// Olav Kallhovd sept2017
// Tested with      : HX711 asian module on channel A and YZC-133 3kg load cell
// Tested with MCU  : Arduino Nano, ESP8266
//-------------------------------------------------------------------------------------
// This is an example sketch on how to use this library
// Settling time (number of samples) and data filtering can be adjusted in the config.h file

#include <HX711_ADC.h>
#include <EEPROM.h>

//HX711 constructor (dout pin, sck pin):
HX711_ADC LoadCell(4,16);

const int eepromAdress = 0;

long t;

void setup() {
  
  float calValue; // calibration value
  calValue = 696.0; // uncomment this if you want to set this value in the sketch 
  #if defined(ESP8266) 
  //EEPROM.begin(512); // uncomment this if you use ESP8266 and want to fetch the value from eeprom
  #endif
  //EEPROM.get(eepromAdress, calValue); // uncomment this if you want to fetch the value from eeprom
  
  Serial.begin(9600); delay(10);
  Serial.println();
  Serial.println("Starting...");
  LoadCell.begin();
  long stabilisingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  if(LoadCell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calValue); // set calibration value (float)
    Serial.println("Startup + tare is complete");
  }
}

void loop() { 
  //update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  //use of delay in sketch will reduce effective sample rate (be carefull with use of delay() in the loop)
  LoadCell.update();

  //get smoothed value from data set
  if (millis() > t + 250) {
    float i = LoadCell.getData();
    Serial.print("Load_cell output val: ");
    Serial.println(i);
    t = millis();
  }

  //receive from serial terminal
  if (Serial.available() > 0) {
    float i;
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  //check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

}
