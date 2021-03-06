#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN  2    //Pin 5 para el reset del RC522
#define SS_PIN   0   //Pin 15 para el SS (SDA) del RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); //Creamos el objeto para el RC522

void setup() {
  Serial.begin(9600); //Iniciamos la comunicación  serial
  SPI.pins(6,7,8,0);
  SPI.begin();        //Iniciamos el Bus SPI
  mfrc522.PCD_Init(); // Iniciamos  el MFRC522
  Serial.println("Lectura del UID");
}

void loop() {
  // Revisamos si hay nuevas tarjetas  presentes
  long print_id = 0;
  if ( mfrc522.PICC_IsNewCardPresent()) 
        {  
      //Seleccionamos una tarjeta
            if ( mfrc522.PICC_ReadCardSerial()) 
            {
                  // Enviamos serialemente su UID
                  Serial.println();
                  Serial.print("Card UID: ");
                  for (byte i = 0; i < mfrc522.uid.size; i++) {
                      print_id = (print_id << 8) | mfrc522.uid.uidByte[i];
                      if(mfrc522.uid.uidByte[i] == 0)
                      {
                        Serial.print("00000000");
                      }
                      else
                      {
                        Serial.print(mfrc522.uid.uidByte[i], BIN);
                      }
                      //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                      //Serial.print(mfrc522.uid.uidByte[i], HEX);   
                  } 
                  Serial.println();
                  Serial.print("Full ID: ");
                  Serial.print(print_id, BIN);
                  // Terminamos la lectura de la tarjeta  actual
                  mfrc522.PICC_HaltA();         
            }      
  } 
}
