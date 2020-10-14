
//triggerhappy by Karl Gruenewald (studiokpg@gmail.com)
/*Trigger generator for modular synthesizers.
 Uses internal or external clock
 Has 6 outputs (A-F) which can each be set to one of 3 kinds of output:
 Clicktable: read through a static rhythm pattern, offset and length can be changed
 Logic: choose from 7 logic modes based on any two other outputs
 Euclidean: generate an Euclidean rhythm pattern given total length and number of groups
 Save and load settings 0-9
 (Can play, but can't store tempos slower than 59 BPM)
 Swing, +/- 99%
 
 IMPORTANT: When putting this software on a new Arduino, you need to comment out the loadSettings()
 call in setup(). Otherwise, you will have garbage settings and things may not work. Save the default
 settings to slot 0, then you can re-enable the loadSettings() line.
 
 */

#include <SoftwareSerial.h>
#include <EEPROM.h>

//Using arrays as "clicktables" to store which clicks get a trigger (1=trigger, 0=no trigger)
//Put what ever you want in here!
// 1  2  3  4| 5  6  7  8| 9  10 11 12|13 14 15 16

boolean clickTables[16][16] = {
  {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0        }
  ,//1
  {
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0        }
  ,//2
  {
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0        }
  ,//3
  {
    1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1        }
  ,//4
  {
    1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0        }
  ,//5
  {
    1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0        }
  ,//6
  {
    1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1        }
  ,//7
  {
    1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0        }
  ,//8
  {
    1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1        }
  ,//9
  {
    1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1        }
  ,//10
  {
    1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0        }
  ,//11
  {
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0        }
  ,//12
  {
    1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1        }
  ,//13
  {
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0        }
  ,//14
  {
    1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0        }
  ,//15
  {
    1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0        } //16
};

byte outParam[6][10]; //10 parameters for each of the 6 outputs:
/*
 0: Mode (0, clicktable, 1, logic or 2, Euclidean)
 1: Offset (128 = no offset)
 2: 0, Trigger or 1, Gate
 3: Clicktable number
 4: Length (if clicktable)
 5: Logic mode (1-7) 1= NOT; 2= AND; 3= OR; 4= NAND; 5= NOR; 6= XOR; 7= XNOR 
 6: 1st Output to base logic on (Outputs must be lower number than the output doing the logic!)
 7: 2nd Output to base logic on
 8: Euclidean length (beats)
 9: Euclidean pulses (groups)
 */
byte paramA = 0; //variable for selecting output to work on

const int offsetRange = 16; //min and max offset amount
boolean logicStore[6][offsetRange]; //store logic values for delay

#define OUT_PORT PORTB //pins 8-13 as outputs
#define PORT_DIRECTION DDRB

byte nextGate = 0; //stores binary gates to write to OUT_PORT
unsigned long nextClock = 0; //when is next pulse
unsigned long nextOffClock = 0; //when to turn triggers off 
const int trigLen = 20; //length of triggers
boolean noLow = false; //only turn gates off once per clock loop
byte offGate = 0; //for turning off triggers selectively

const int clockPin = A0; //input for external clock
int clockValue = 0; //variable to store value from clock input
boolean clockInternal = true; //select internal or external clock
boolean clockPrevious = false;
boolean clock = false; //it is a clock or not?
unsigned long lastClock = 0; //for external clock calculation
const int clockThreshold = 800;
boolean seqStop = false;
boolean extClock = false;

const int buttonPin = 7; //encoder button for menu system
boolean button1;
boolean b1Pushed = false;
boolean b1Previous = false;
unsigned long lastButtonTime = 0;
const int debounceTime = 20;

const int saveButtonPin = A5; //enter save/read mode
const int buttonThreshold = 400;
int saveRead = 255;
boolean saveButton = false;
int saveSlot = 0;
const int saveDebounce = 500;
unsigned long lastSave = 0;

const int syncButtonPin = A1; //synchronize/reset patterns
boolean syncButton = false;
int syncRead = 0;
const int syncDebounce = 500; //Keep sync from happening too often
unsigned long lastSync = 0;

const int syncIntExtPin = A3; //select internal/external sync
int sourceRead = 0; //temp variable for reading int/ext switch
unsigned long switchPrevious = 0;
const int switchThreshold = 400;

//Defaults
int rateMain = 250; //internal clock interval in ms
int swing = 0; //range -99 to 99 (percent)
boolean swingToggle = 0; //swing this beat or not
float swingMod = 0; //amount to add or subtract from the beat for swing
String BPM;
int clickStep[6] = {
  0,0,0,0,0,0}; //array to track which step each pattern is on
boolean outTemp[6] = {
  false, false, false, false, false, false}; //gather output values
boolean outPrev[6] = {
  false, false, false, false, false, false}; //keep track of current output status
const int BjorkLen = 32; //max length of Euclidean pattern
boolean rhythm[BjorkLen]; //array for Euclidean pattern

extern int encoderMode; //for menu system

volatile int8_t tmpdata = 0; //variable to pass encoder value

const int txPin = 5;  // serial output pin
const int rxPin = 4;  // unused, but SoftwareSerial needs it...

SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);


void setup() {
  Serial.begin(9600);
  setupEncoder();
  setupDisplay();
  PORT_DIRECTION = B111111;
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  initParams();// initializes outParams array
  //IMPORTANT: comment out the following line first time you run this sketch!
//  loadSettings(); //loads from save location 0 on startup (will load garbage until you do a save to slot 0!)
  pinMode(clockPin, INPUT);
  pinMode(saveButtonPin, INPUT);
  digitalWrite(saveButtonPin, HIGH);
  pinMode(syncButtonPin, INPUT);
  digitalWrite(syncButtonPin, HIGH);
  pinMode(syncIntExtPin, INPUT);
  digitalWrite(syncIntExtPin, HIGH);
  lastClock = millis();
}

void loop() {
  readInputs();
  if (tmpdata) {
    doUpdates();
    tmpdata = 0;
  }
  if (clockInternal){
    if (seqStop || extClock) {
      nextClock = millis() + rateMain;
      seqStop = false;
      extClock = false;
    }
    if ((millis() >= nextClock) ) {
      clockAction();
      nextOffClock = nextClock + trigLen; //time to make triggers go low
      swingMod = ((float)rateMain * ((float)swing / 100));
      if (swingToggle) nextClock = nextClock + rateMain + swingMod;
      else nextClock = nextClock + rateMain - swingMod;
      lastClock = millis();
      if (rateMain < 30) rateMain = 30;
      if (encoderMode == 1) {
        BPM = String(15000/rateMain);
        displayLED(BPM);
      }
      swingToggle = !swingToggle;
    }
  }

  else { //external clock
    extClock = true;
    if (clock) {
      clockAction();
      nextOffClock = nextClock + trigLen; //no swing with external clock
      nextClock = nextClock + rateMain;
      if (rateMain < 30) rateMain = 30;
      if (encoderMode == 1) {
        BPM = String(15000/rateMain);
        displayLED(BPM);
      }
    }
    if (seqStop) { //this value need to keep getting updated while seq is stopped
      nextOffClock = nextClock + trigLen;
    }
  }

  if ((millis() >= nextOffClock) && (noLow == false)) gatesLow();

  if (b1Pushed) {
    doButton();
    b1Pushed = false;
  }
  if (saveButton) {
    lastSave = millis();
    encoderMode = 14;
    displayLED("FILE");
  }
  if (syncButton) {
    lastSync = millis();
    synchronize();
  }
}


void readInputs() {

  //read encoder button
  button1 = digitalRead(buttonPin);
  if (button1) {
    if((b1Previous == false) && (millis() - lastButtonTime > debounceTime)) {
      b1Pushed = true;
      lastButtonTime = millis();
    }
    else b1Pushed = false;
    b1Previous = true;
  }
  else {
    b1Pushed = false;
    b1Previous = false;
  }

  //read file button
  saveRead = analogRead(saveButtonPin);
  if (saveRead < buttonThreshold) saveButton = false;
  else saveButton = true;
  if (saveButton && ((millis() - lastSave) > saveDebounce)) saveButton = true;
  else saveButton = false;

  //read sync button
  syncRead = analogRead(syncButtonPin);
  if (syncRead < buttonThreshold) syncButton = false;
  else syncButton = true;
  if (syncButton && ((millis() - lastSync) > syncDebounce)) syncButton = true;
  else syncButton = false;

  //read int/ext switch
  sourceRead = analogRead(syncIntExtPin);
  if (sourceRead > switchThreshold) clockInternal = true;
  else if (sourceRead <= buttonThreshold) clockInternal = false;

  //read clock pin
  if (!clockInternal) {
    clockValue = analogRead(clockPin);
    if (clockValue >= clockThreshold) {
      if (!clockPrevious) {
        clock = true;
        clockPrevious = true;
      }
      else clock = false;
    }
    else {
      clock = false;
      clockPrevious = false;
      if (millis() - lastClock > 2000) seqStop = true; //stop if no clock for 2 sec.
    }
    if (clock) { 
      rateMain = millis() - lastClock;
      lastClock = millis();
      seqStop = false;
    }
  } 
}


void clockAction() {
  noLow = false;
  for (int i = 0; i < 6; i++) { //go through each output A-F

    if (outParam[i][0] == 0) { //output is clicktable
      int length = outParam[i][4];
      int offset = outParam[i][1] - 128;
      if (abs(offset) > length) offset = offset % length;
      int counter = clickStep[i] + offset;
      if (counter > length - 1) counter = counter - length;
      if (counter < 0) counter = counter + length;
      int thisArray = outParam[i][3];
      outTemp[i] = clickTables[thisArray][counter];
      clickStep[i]++;
      if (clickStep[i] > length - 1) clickStep[i] = 0;
    }

    else if (outParam[i][0] == 1) { //output is logic; 
      //NOTE: user must make sure logic outputs come after the ouputs they refer to, or logic will be based on previous beat
      byte baseA = outParam[i][6];//which output is base
      byte baseB = outParam[i][7];
      baseA = outTemp[baseA];//change base to value of click at that output
      baseB = outTemp[baseB];

      for (int j = offsetRange; j > 0; j--) { //Slide all the values down for delay table
        logicStore[i][j] = logicStore[i][j-1];
      }

      switch (outParam[i][5]) { //logic calculations

      case 0: //NOT
        logicStore[i][0] = !baseA; 
        break;

      case 1: //AND 
        logicStore[i][0] = baseA && baseB; 
        break;

      case 2: //OR 
        logicStore[i][0] = baseA || baseB;
        break;

      case 3: //NAND 
        logicStore[i][0] = !(baseA && baseB); 
        break;

      case 4: //NOR 
        logicStore[i][0] = !(baseA || baseB); 
        break;

      case 5: //XOR 
        logicStore[i][0] = baseA ^ baseB;
        break;

      case 6: //XNOR 
        logicStore[i][0] = !(baseA ^ baseB); 
        break;
      }
      int offset = outParam[i][1] - 128;
      if (offset < 1) offset = 1;
      if (offset > offsetRange) offset = offsetRange;
      outTemp[i] = logicStore[i][offset - 1];
    }

    else {  //output is Euclidean
      // based on http://kreese.net/blog/2010/03/27/generating-musical-rhythms/
      int steps = outParam[i][8]; //beats
      int pulses = outParam[i][9]; //groups
      int pauses = steps - pulses;
      boolean switcher = false;
      if (pulses > pauses) {
        switcher = true;
        pauses ^= pulses;
        pulses ^= pauses;
        pauses ^= pulses;
      }
      int perPulse = floor(pauses / pulses);
      int remainder = pauses % pulses;
      int noSkip;
      if (remainder == 0) noSkip = 0;
      else noSkip = floor(pulses / remainder);
      int skipXTime;
      if (noSkip == 0) skipXTime = 0;
      else skipXTime = floor((pulses - remainder)/noSkip);

      int counter = 0;
      int skipper = 0;
      int pos = 0;
      for (int h = 1; h <= steps; h++) {
        if (counter == 0) {
          rhythm[pos] = !switcher;
          pos ++;
          counter = perPulse;

          if (remainder > 0 && skipper == 0) {
            counter++;
            remainder--;
            if (skipXTime > 0) skipper = noSkip;
            else skipper = 0;
            skipXTime--;
          }
          else {
            skipper --;
          }
        }
        else {
          rhythm[pos] = switcher;
          pos++;
          counter--;
        }
      }

      int c = clickStep[i] + outParam[i][1] - 128;
      c = abs(c % steps);
      outTemp[i] = rhythm[c];
      clickStep[i]++;
      if (clickStep[i] >= steps) clickStep[i] = 0;
    } 
  }



  for (int k = 5; k > -1; k--) { //doing the output
    if (outParam[k][2]) {
      if (outTemp[k]) {
        outTemp[k] = !outPrev[k];
      }
      else outTemp[k] = outPrev[k];
    }
    nextGate = nextGate << 1;
    nextGate = nextGate | outTemp[k];
  }

  OUT_PORT = nextGate; //write gate outputs
  nextGate = 0;
  for (int k = 0; k < 6; k++) {
    outPrev[k] = outTemp[k]; 
  }   
}

void gatesLow() {
  for (int k = 5; k > -1; k--) {
    offGate = offGate << 1;
    if (outParam[k][2] && outPrev[k]) {
      offGate = offGate | 1;
    }
  }
  OUT_PORT = offGate;
  offGate = 0;
  noLow = true;
}

void synchronize() { //resets all counters
  displayLED("SYNC");
  for (int i = 0; i < 6; i++) {
    clickStep[i] = 0;
  }
  swingToggle = 0;
  doUpdates();
}

void initParams() { //sets some default values for outParam
  for (int i = 0; i < 6; i++){
    for (int j = 0; j < 10; j++){
      outParam[i][j] = 0;
    }
  }
  for (int k = 0; k < 6; k++){
    outParam[k][1] = 128; //go back and set offsets to null value
  }
}

/*
 0: Mode (0, clicktable, 1, logic or 2, Euclidean)
 1: Offset (128 = no offset)
 2: 0, Trigger or 1, Gate
 3: Clicktable number
 4: Length (if clicktable)
 5: Logic mode (1-7) 1= NOT; 2= AND; 3= OR; 4= NAND; 5= NOR; 6= XOR; 7= XNOR 
 6: 1st Output to base logic on
 7: 2nd Output to base logic on
 8: Euclidean length (beats)
 9: Euclidean pulses (groups)
 */

void saveSettings() { //will overwrite settings in EEPROM without warning!
  int slotByte = (saveSlot * 62); //62 paramaters to save, slotByte is first address per set
  int slotCounter = 0;
  for (int i = 0; i < 6; i++){
    for (int j = 0; j < 10; j++){
      EEPROM.write(slotByte + slotCounter, outParam[i][j]);
      slotCounter ++;
    }
  }
  EEPROM.write(slotByte + slotCounter, swing);
  slotCounter ++;
  int tempRate = (rateMain > 255) ? 255 : rateMain; //can't store more than 1 byte per EEPROM slot
  EEPROM.write(slotByte + slotCounter, tempRate); //rateMain 256 ~= 59 bpm
}


void loadSettings() {
  int slotByte = (saveSlot * 62); //62 paramaters to save, slotByte is first address per set
  int slotCounter = 0;
  for (int i = 0; i < 6; i++){
    for (int j = 0; j < 10; j++){
      outParam[i][j] = EEPROM.read(slotByte + slotCounter);
      slotCounter ++;
    }
  }
  swing = EEPROM.read(slotByte + slotCounter);
  slotCounter ++;
  rateMain = EEPROM.read(slotByte + slotCounter);
}




