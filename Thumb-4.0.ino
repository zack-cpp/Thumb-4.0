#include <Keyboard.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

SoftwareSerial mySerial(8,9);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

struct Data{
  String option = "0";
  
  int addr = 0;
  byte baca[25];
  byte initState = 0;
  
  String eepromVal;
  String serialVal;

  bool state = false;
  bool readState = false;

  char toSend[25];

  String readPasswordFromEEPROM(int addr);
  void writeEEPROM(int addr, const String &kata);
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
  Serial1.begin(9600);
  finger.begin(57600);
//  Keyboard.begin();
  if(finger.verifyPassword()){

  }else{
    while(true){
      delay(1);
    }
  }  
  finger.getTemplateCount();
  if(finger.templateCount == 0){
    EEPROM.write(50,0);
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
        data.eepromVal.toCharArray(data.toSend, sizeof(data.toSend));
        for(unsigned short int i = 0; i < sizeof(data.toSend); i++){
          Keyboard.write(data.toSend[i]);
        }
	      Keyboard.write(176);
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
  while(Serial1.available()){
    data.option = Serial1.readStringUntil('\n');
  }
}

void Data::enroll(){
  Serial1.write("FG_PLACE_NEW_FINGER");
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      break;
    default:
      break;
    }
  }
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    default:
      return p;
  }
  Serial1.write("FG_REMOVE_FINGER");
  delay(2000);
  p = 0;
  while(p != FINGERPRINT_NOFINGER){
    p = finger.getImage();
  }
  p = -1;
  Serial1.write("FG_PLACE_SAME_FINGER");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      break;
    default:
      break;
    }
  }
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    default:
      return p;
  }
  p = finger.createModel();
  if (p == FINGERPRINT_OK){
  }else{
    return p;
  }
  
  p = finger.storeModel(1);
  if (p == FINGERPRINT_OK) {
    Serial1.write("FG_SUCCESS");
  }else{
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
  if(data.option == "1"){
    if(EEPROM.read(50) == 0){
      data.enroll();
      delay(2000);
      Serial1.write("INIT_NEW_PASS");
      data.readSerial();
      for(int i = 0; i < EEPROM.length(); i++){
        EEPROM.write(i,0);
      }
      data.writeEEPROM(0,data.serialVal.c_str());
      EEPROM.write(50,1);
      data.option = "0";
      Serial1.write("INIT_SUCCESS");
    }else{
      Serial1.write("INIT_REJECTED");
    }
  }
}

void Data::changePassword(){
  if(data.option == "2"){
    //send PLACE_FINGER
    Serial1.write("PW_PLACE_FINGER");
    while(data.readState == false){
      data.readFinger();
    }
    Serial1.write(data.readPasswordFromEEPROM(0).c_str());
    delay(2000);
    
    Serial1.write("PW_NEW_PASS");
    data.readSerial();
    for (int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.write(50,1);
    data.writeEEPROM(0,data.serialVal.c_str());
    data.option = "0";
    Serial1.write("PW_SUCCESS");
  }
//  data.resetFunc();
}

void Data::changeFinger(){
  int p = 0;
  if(data.option == "3"){
    Serial1.write("FG_PLACE_OLD_FINGER");
    while(data.readState == false){
      data.readFinger();
    }
    finger.emptyDatabase();
    Serial1.write("FG_REMOVE_FINGER");
    delay(2000);
    while(p != FINGERPRINT_NOFINGER){
      p = finger.getImage();
    }
    data.enroll();
    p = 0;
    while(p != FINGERPRINT_NOFINGER){
      p = finger.getImage();
    }
  }
  data.option = "0";
  data.readState = false;
//  data.resetFunc();
}

void Data::readSerial(){
  data.serialVal = "";
  while(!Serial1.available()){
      delay(10);
    }
    while(Serial1.available()){
      data.serialVal = Serial1.readStringUntil('\n');
    }
}
