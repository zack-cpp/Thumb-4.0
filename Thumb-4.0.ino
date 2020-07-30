#include <Keyboard.h>
#include <EEPROM.h>
#include <Adafruit_Fingerprint.h>

#if defined(__AVR__) || defined(ESP8266)
SoftwareSerial mySerial(2,3);
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

struct Data{
  String option = "0";
  
  int addr = 0;
  byte baca[25]; // length + 1
  byte initState = 0;
  
  String eepromVal;
  String serialVal;

  bool state = false;
  bool readState = false;

  void writeEEPROM(int addr, const String &kata);
  String readPasswordFromEEPROM(int addr);
  void readIsi();
  void init();
  void enroll();
  void readOption();
  void readFinger();
  void changeFinger();
  void changePassword();

  void readSerial();
  void (*resetFunc)(void) = 0;
}data;

void setup() {
  Serial.begin(9600);
  finger.begin(57600);
//  Keyboard.begin();
  if(finger.verifyPassword()){

  }else{

    while(1){delay(1);}
  }
  
  finger.getTemplateCount();
  if(finger.templateCount == 0){

  }else{

  }
}

void loop() {
  data.readIsi();
  data.readOption();
  if(data.state){
    if(data.option != "0"){
      data.init();
      data.changePassword();
      data.changeFinger();
    }else{
      data.readFinger();
      if(data.readState){
        data.readPasswordFromEEPROM(0);
//        Keyboard.write(data.eepromVal.c_str());
      }
      data.readState = false;
    }
  }else{
    data.init();
  }
  delay(10);
}

void Data::readIsi(){
  if(EEPROM.read(0) == 0){
    data.state = false;
  }else{
    data.state = true;
  }
}

void Data::writeEEPROM(int addr, const String &kata){
  byte len = kata.length() + 1;
  EEPROM.write(addr, len);
  for(int i = 0; i < len; i++){
    EEPROM.write(addr + 1 + i, kata[i]);
  }
}

String Data::readPasswordFromEEPROM(int addr){
  data.eepromVal = "";
  for(int i = addr + 1; i < EEPROM.read(addr); i++){
    data.baca[i-1] = EEPROM.read(i);
  }
  data.eepromVal += String((char *)baca);
  return data.eepromVal;
}

void Data::readOption(){
  while(Serial.available()){
    data.option = Serial.readStringUntil('\n');
  }
}

void Data::enroll(){
  Serial.write("FG_PLACE_NEW_FINGER");
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
//      Serial.println("Imaging error");
      break;
    default:
//      Serial.println("Unknown error");
      break;
    }
  }
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
//      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
//      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
//      Serial.println("Could not find fingerprint features");
      return p;
    default:
//      Serial.println("Unknown error");
      return p;
  }
//  Serial.println("remove finger");
  Serial.write("FG_REMOVE_FINGER");
  delay(2000);
  p = 0;
  while(p != FINGERPRINT_NOFINGER){
    p = finger.getImage();
  }
  p = -1;
//  Serial.println("place same finger again");
  Serial.write("FG_PLACE_SAME_FINGER");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
//      Serial.println("Imaging error");
      break;
    default:
//      Serial.println("Unknown error");
      break;
    }
  }
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
//      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
//      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
//      Serial.println("Could not find fingerprint features");
      return p;
    default:
//      Serial.println("Unknown error");
      return p;
  }
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
//    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
//    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
//    Serial.println("Fingerprints did not match");
    return p;
  } else {
//    Serial.println("Unknown error");
    return p;
  }
  p = finger.storeModel(1);
  if (p == FINGERPRINT_OK) {
//    Serial.println("Stored!");
    Serial.write("FG_SUCCESS");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
//    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
//    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
//    Serial.println("Error writing to flash");
    return p;
  } else {
//    Serial.println("Unknown error");
    return p;
  }
}

void Data::readFinger(){
  uint8_t p = finger.getImage();
  switch(p){
    case FINGERPRINT_OK:
      break;
    default:
      return p;
  }
  p = finger.image2Tz();
  switch(p){
    case FINGERPRINT_OK:
      break;
    default:
      return p;
  }
  p = finger.fingerSearch();
  if(p == FINGERPRINT_OK){
    data.readState = true;
  }else{
    return p;
  }
}

void Data::init(){
  if(EEPROM.read(50) == 0){
    if(data.option == "1"){
      data.enroll();
      Serial.write("INIT_NEW_PASS");
      data.readSerial();
      for(int i = 0; i < EEPROM.length(); i++){
        EEPROM.write(i,0);
      }
      data.writeEEPROM(0,data.serialVal.c_str());
      EEPROM.write(50,1);
      Serial.write("INIT_SUCCESS");
    }
  }else{
    Serial.write("INIT_REJECTED");
  }
}

void Data::changePassword(){
  if(data.option == "2"){
    //send PLACE_FINGER
    Serial.write("PW_PLACE_FINGER");
    while(data.readState == false){
      data.readFinger();
    }
    Serial.write(data.readPasswordFromEEPROM(0).c_str());
    delay(2000);
    
    Serial.write("PW_NEW_PASS");
    data.readSerial();
    for (int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.write(50,1);
    data.writeEEPROM(0,data.serialVal.c_str());
    data.option = "0";
    Serial.write("PW_SUCCESS");
  }
//  data.resetFunc();
}

void Data::changeFinger(){
  int p = 0;
  if(data.option == "3"){
    Serial.write("FG_PLACE_OLD_FINGER");
    while(data.readState == false){
      data.readFinger();
    }
    finger.emptyDatabase();
    Serial.write("FG_REMOVE_FINGER");
    delay(2000);
    while(p != FINGERPRINT_NOFINGER){
      p = finger.getImage();
    }
    data.enroll();
  }
  data.option = "0";
  data.readState = false;
//  data.resetFunc();
}

void Data::readSerial(){
  data.serialVal = "";
  while(!Serial.available()){
      delay(10);
    }
    while(Serial.available()){
      data.serialVal = Serial.readStringUntil('\n');
    }
}
