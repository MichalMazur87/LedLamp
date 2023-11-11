#define PIR 2
#define IR_DIODE 3
#define BUTTON_PIN 4
#define MOSFET 5
#include <IRremote.h>
#include <EEPROM.h>
int solarPanel = A7;
bool solarCharging = false;
int mode = 2;
int memory = 0;
int solarValue = 0;
unsigned long savedTime = 0;
int movement = 0;
int irCode = 0;
bool occupancy = false;
int counter = 0;
bool lowLightMode = false;
int lowLight = 10; //0 to 255 analog low light level

void setup() {
  Serial.begin(9600);
  Serial.println("---- Solar LED lamp ----"); 
  mode = EEPROM.read(memory);
  Serial.println("Mode set to:"); 
  Serial.println(mode); 
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(MOSFET, OUTPUT);
  pinMode(PIR, INPUT);
  IrReceiver.begin(IR_DIODE);
  delay(2000);
}

void loop() {
  checkIrCode();
  checkButton();
  if (irCode == 0 && digitalRead(BUTTON_PIN)!=0 && occupancy==false ) {
    
    switch (mode) {
    case 1:
      // human gody induction
      checkSolarCharging();
      if(solarCharging==false){
        checkPir();
        if (movement==HIGH){
          light(true, mode);
        }
        else {
          light(false, mode);
        }
      }
      else if(solarCharging==true){
        digitalWrite(MOSFET, LOW);
      }
    break;

    case 2:
      // human gody induction +  low light mode
      checkSolarCharging();
      if(solarCharging==false){
        if (lowLightMode == false){
          analogWrite(MOSFET, lowLight);
          lowLightMode = true;
        }
        checkPir();
        if (movement==HIGH){
          light(true, mode);
        }
        else {
          light(false, mode);
        }
      }
      else if(solarCharging==true){
        lowLightMode = false;
        digitalWrite(MOSFET, LOW);
      }
    break;

    case 3:
    //low light mode
      checkSolarCharging();
      if(solarCharging==false){
        //digitalWrite(MOSFET, HIGH);
        analogWrite(MOSFET, lowLight);
      }
      else if(solarCharging==true) {
        digitalWrite(MOSFET, LOW);
      }
    break;

    default:
      Serial.println("Wrong mode!"); 
      break;
    }
  }
  if (occupancy==true){
    if (millis()-savedTime >= 120000UL) //120sec
    {
      occupancy=false;
    }

    counter++;
    if (counter>=30000){
      Serial.println("Waiting 2 minutes:");
      Serial.println((millis()-savedTime)/1000); 
      counter=0;
    }
  }
}

void checkSolarCharging(){
  solarValue = analogRead(solarPanel);
  if (solarValue >=300){
    if(solarCharging==false){
      Serial.println("Solar charging changed to true"); 
    }
    solarCharging = true;
  }
  else if(solarValue <=50){
    if(solarCharging==true){
      Serial.println("Solar charging changed to false"); 
    }
    solarCharging = false;
  }
  if (solarCharging){
    counter++;
    if (counter>=5000){
      Serial.println("Solar value:"); 
      Serial.println(solarValue); 
      counter=0;
    }
  }
}

void checkIrCode(){
  if (IrReceiver.decode()) {
    irCode = IrReceiver.decodedIRData.command;
    if (irCode != 0) {
      irDecoder(irCode);
    }
    delay(2000);
    IrReceiver.resume();
  }
  else {
    irCode = 0;
  }
}

void irDecoder(int irCode) {
  switch (irCode) {
    case 69:
      Serial.println("IR Power on");
      digitalWrite(MOSFET, HIGH);
      savedTime=millis();
      occupancy = true;
    break;

    case 71:
      Serial.println("IR Power off");
      digitalWrite(MOSFET, LOW);
      occupancy = false;
      delay(2000);
    break;

    case 7:
      Serial.println("IR mode 1");
      mode=1;
      EEPROM.write(memory, mode);
      Serial.println("Mode changed to: ");
      Serial.println(mode);
      digitalWrite(MOSFET, LOW);
      delay(200);
      for(int i = 1; i <= mode; i++){
        digitalWrite(MOSFET,   HIGH);
        delay(100);
        digitalWrite(MOSFET, LOW);
        delay(100);
      }
    break;

    case 21:
      Serial.println("IR mode 2");
      mode=2;
      Serial.println("Mode changed to: ");
      Serial.println(mode);
      EEPROM.write(memory, mode);
      digitalWrite(MOSFET, LOW);
      delay(200);
      for(int i = 1; i <= mode; i++){
        digitalWrite(MOSFET,   HIGH);
        delay(100);
        digitalWrite(MOSFET, LOW);
        delay(100);
      }
    break;

    case 9:
      Serial.println("IR mode 3");
      mode=3;
      Serial.println("Mode changed to: ");
      Serial.println(mode);
      EEPROM.write(memory, mode);
      digitalWrite(MOSFET, LOW);
      delay(200);
      for(int i = 1; i <= mode; i++){
        digitalWrite(MOSFET,   HIGH);
        delay(100);
        digitalWrite(MOSFET, LOW);
        delay(100);
      }
    break;

    default:
      Serial.println("IR Unknown");
    break;
  }
}

void checkButton(){
  if(digitalRead(BUTTON_PIN)==0){
    if(mode==3){
      mode = 1;
    }
    else{
      mode+=1;
    }
    Serial.println("Mode changed to:");
    Serial.println(mode);
    EEPROM.write(memory, mode);
    digitalWrite(MOSFET, LOW);
    delay(200);
    for(int i = 1; i <= mode; i++){
      digitalWrite(MOSFET,   HIGH);
      delay(100);
      digitalWrite(MOSFET, LOW);
      delay(100);
    }
  }
}

void checkPir(){
  movement = digitalRead(PIR);
  counter++;
  if(movement != HIGH && counter >=10000) {
    Serial.println("No movement");
    counter=0;
  }
  else if (movement == HIGH && counter >=10000) {
    Serial.println("Movement detected!");
  }
}

void light(bool state, int mode){
  if(state == true){
    digitalWrite(MOSFET,   HIGH);
    savedTime=millis();
  }
  else if (state == false && mode != 2){
    if (millis()-savedTime >= 15000UL){ //15sec
      digitalWrite(MOSFET, LOW);
    }
  }
  else if(state == false && mode == 2){
    //for low light mode
    if (millis()-savedTime >= 15000UL){ //15sec
      analogWrite(MOSFET, lowLight);
    }
  }
}
