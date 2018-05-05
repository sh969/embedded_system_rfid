/*
 * Author: sh969 (https://github.com/sh969)
 *
 * Reads 4 byte UID from MIFARE 1K or 4K using the RC522 RFID module
 * Saves UID and timestamp in a list
 * Gives an audio feedback depending on whether UID has already been registered 
 * Any serial input triggers printing of the list
 * Can be used to monitor students lecture attendance as it reads University cards
 *
 * Uses MFRC522 - Library
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above

int serialData;

byte studentList[40];   // One UID is 4 bytes, meaning we can save 10 students
int studentNumber = 0;  // Number of students that already checked in
long studentTime[10];   // Time in ms when each student registered

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

void setup() {
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                           // Init MFRC522 card
  Serial.println(F("Read student data from a MIFARE PICC:"));    //shows in serial that it is ready to read
}

boolean isRegistered() {
  for (int j=0; j<studentNumber; j++) {
    int i = 0;
    while (studentList[j*4+i] == mfrc522.uid.uidByte[i]) {
      i++;
      if (i==4) return true;
    }
  }
  return false;
}

void printList() {
  Serial.println(F("---------------------------"));
  for (int j=0; j<studentNumber; j++) {
    Serial.print(F("Student "));
    for (int i=0; i<4; i++) Serial.print(studentList[j*4+i], HEX);
    Serial.print(F(" registered at "));
    Serial.print(studentTime[j]);
    Serial.println(F(" ms."));
  }
  Serial.println(F("---------------------------"));
}

void loop() {
  if (Serial.available() > 0)
  {
    if (Serial.read()) printList();
//    serialData = Serial.read();
//    Serial.println(serialData);
  } 
//  MFRC522::MIFARE_Key key;
//  key = {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5};

  // leave loop when no card is detected
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // if one is detected we move on
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

//  Serial.println(F("**Card Detected:**"));
  if (!isRegistered()) {
    Serial.print(F("New student ID registered: "));
    tone(7, 4978, 100);
    for (int i=0; i<4; i++) { // mfrc522.uid.size equals 4 for all tested Mifare cards
      studentList[studentNumber*4+i] = mfrc522.uid.uidByte[i];
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    studentTime[studentNumber] = millis();
    studentNumber++;
  }
  else {
    Serial.println(F("Student already registered."));
    tone(7, 4978, 400);
    delay(400);
    tone(7, 4978, 400);
  }
//  Serial.println(F("**Read finished.**"));

//  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card
//  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex
//  mfrc522.PICC_DumpMifareClassicToSerial(&(mfrc522.uid), mfrc522.PICC_TYPE_MIFARE_4K, &key);
  
delay(1000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
