#include "Arduino.h"
#include "AnalogRamp.h"

/////////////////
// Module Info //
/////////////////
#define FirmwareVersion "0001" // This doesnt mean anything here I would say, just copied from TouchShaker
#define moduleName "OG_LED_Module" // Name of module for manual override UI and state machine assembler

////////////////////////////////////////////////////
// LED /////////////////////////////////////////////
////////////////////////////////////////////////////
#define GOT_BYTE 14
#define DID_ABORT 15
#define LED_INTENSITY 126

#define SWITCH_CW_ON_OG0 127  // Not triggerable anymore. Line is constantly on at the set intensity.
#define SWITCH_CW_OFF_OG0 128  // Switches CW mode off again. To normal state.
#define SWITCH_CW_ON_OG1 129  // Not triggerable anymore. Line is constantly on at the set intensity.
#define SWITCH_CW_OFF_OG1 130  // Switches CW mode off again. To normal state.
#define MODULE_INFO 255  // returns module information

// Pins for stepper - spouts
#define PIN_og0_LED_IN 25  // Pulldown Line for reset after every trial
#define PIN_og1_LED_IN 24  // Pulldown Line for reset after every trial

#define PIN_og0_LED_OUT A21  // Pulldown Line for reset after every trial
#define PIN_og1_LED_OUT A22  // Pulldown Line for reset after every trial
#define PIN_og0_GATE 38   // Digital line that is HIGH as long as the Analoge Line is != 0
#define PIN_og1_GATE 39  // Digital line that is HIGH as long as the Analoge Line is != 0

// define ramp objects
AnalogRamp og0(PIN_og0_LED_IN, PIN_og0_LED_OUT, PIN_og0_GATE);
AnalogRamp og1(PIN_og1_LED_IN, PIN_og1_LED_OUT, PIN_og1_GATE);

// Serial COM variables
unsigned long serialClocker = millis();
int FSMheader = 0;
bool midRead = 0;
bool read_msg_length = 0;
float temp[2]; // temporary variable for general purposes

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
}

void loop() {
  ReadSerialCommunication();

  og0.update();
  og1.update();
} // end of void loop

void ReadSerialCommunication() {
  // Serial Communication over USB
  // Main purpose is to start a new trial
  if (Serial.available() > 0) {
    if (!midRead) {
      FSMheader = Serial.read();
      midRead = 1; // flag for current reading of serial information
      serialClocker = millis(); // counter to make sure that all serial information arrives within a reasonable time frame (currently 100ms)
      read_msg_length = false;
    }
    
    if (midRead) {
      if (FSMheader == LED_INTENSITY) { // set new intensity value
        if (!read_msg_length) {
          if (Serial.available() >= 2) {
            for (int i = 0; i < 2; i++) { // get number of characters for each variable (6 in total)
              temp[i] = Serial.read(); // number of characters for current variable
            }
            read_msg_length = true;
          }
        }

        if (read_msg_length) {
          if (Serial.available() >= (temp[0]+temp[1])) { // if enough bytes are sent for all variables to be read
            // read all variables for current trial
            og0.intensity = readSerialChar(temp[0]); // og1_intensity
            og1.intensity = readSerialChar(temp[1]); // og2_intensity
            
            Serial.write(GOT_BYTE); midRead = 0;
  
            //flush the Serial to be ready for new data
            while (Serial.available() > 0) {
              Serial.read();
            }
          }
          
          else if ((millis() - serialClocker) >= 100) {
            Serial.write(DID_ABORT); midRead = 0; 
            while (Serial.available() > 0) {
              Serial.read();
            }
          }
        }
      }
      else if (FSMheader == SWITCH_CW_ON_OG0) { // set new intensity value
        og0.switchCWMode(1);
        Serial.write(GOT_BYTE); midRead = 0;
      }
      else if (FSMheader == SWITCH_CW_OFF_OG0) { // set new intensity value
        og0.switchCWMode(0);
        Serial.write(GOT_BYTE); midRead = 0;
      }
      else if (FSMheader == SWITCH_CW_ON_OG1) { // set new intensity value
        og1.switchCWMode(1);
        Serial.write(GOT_BYTE); midRead = 0;
      }
      else if (FSMheader == SWITCH_CW_OFF_OG1) { // set new intensity value
        og1.switchCWMode(0);
        Serial.write(GOT_BYTE); midRead = 0;
      }
      else if (FSMheader == MODULE_INFO) { // return module information to bpod
        returnModuleInfo();
        midRead = 0;
      }
      
      else {
        //flush the Serial to be ready for new data
        while (Serial.available() > 0) {
          Serial.read();
        }
  
        //send abort to request resend
        Serial.write(DID_ABORT); midRead = 0;
      }
    }
  }

  if (midRead && ((millis() - serialClocker) >= 100)) {
      //flush the Serial to be ready for new data
      while (Serial.available() > 0) {
        Serial.read();
      }

      //send abort to request resend
      Serial.write(DID_ABORT); midRead = 0;
  }
}

float readSerialChar(byte currentRead){
  float currentVar = 0;
  byte cBytes[currentRead-1]; // current byte
  int preDot = currentRead; // indicates how many characters there are before a dot
  int cnt = 0; // character counter

  if (currentRead == 1){
    currentVar = Serial.read() -'0'; 
  }

  else {
    for (int i = 0; i < currentRead; i++) {
      cBytes[i] = Serial.read(); // go through all characters and check for dot or non-numeric characters
      if (cBytes[i] == '.') {cBytes[i] = '0'; preDot = i;}
      if (cBytes[i] < '0') {cBytes[i] = '0';}
      if (cBytes[i] > '9') {cBytes[i] = '9';}
    }
  
    // go through all characters to create new number
    if (currentRead > 1) {
      for (int i = preDot-1; i >= 1; i--) {
        currentVar = currentVar + ((cBytes[cnt]-'0') * pow(10,i));
        cnt++;
      }
    }
    currentVar = currentVar + (cBytes[cnt] -'0'); 
    cnt++;
  
    // add numbers after the dot
    if (preDot != currentRead){
      for (int i = 0; i < (currentRead-preDot); i++) {
        currentVar = currentVar + ((cBytes[cnt]-'0') / pow(10,i));
        cnt++;
      }
    }
  }
  return currentVar;
}


void returnModuleInfo() {
  Serial.write(65); // Acknowledge
  Serial.write(FirmwareVersion); // 4-byte firmware version
  Serial.write(sizeof(moduleName)-1); // Length of module name
  Serial.print(moduleName); // Module name
  Serial.write(0); // 1 if more info follows, 0 if not
}
