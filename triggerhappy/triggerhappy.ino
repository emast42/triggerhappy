
//triggerhappy by Karl Gruenewald (studiokpg@gmail.com)
/*Trigger generator for modular synthesizers.
 Uses internal or external clock
 Has 6 outputs (A-F) which can each be set to one of 6 kinds of output:
 Clicktable: read through a static rhythm pattern, offset and length can be changed
 Logic: choose from 7 logic modes based on any two other outputs
 Euclidean: generate an Euclidean rhythm pattern given total length and number of groups
 LFSR: ...LFSR, adjust length, tap1 and tap2
 Random: random [1 ... N+1] > threshold N
 Division: clock division: none, 2, 3, 4 ... 16
 Save and load settings 0-13
 (Can play, but can't store tempos slower than 59 BPM)
 Swing, +/- 99%

 IMPORTANT: When putting this software on a new Arduino, you need to comment out the loadSettings()
 call in setup(). Otherwise, you will have garbage settings and things may not work. Save the default
 settings to slot 0, then you can re-enable the loadSettings() line.


 makrospex TODO:
 1. (*) swing on exernal clock
 4. (*) saveButton = 4. function Reset (like Sync, set all parameters to defaults on all channels)
 5. (*) repair menusystem (eeprom slot select right before saving/loading, without showing bpm before selection of eeprom slot) (impossible without tmpdata change?
 6. (*) int/ext switch bug (random trigger lengths after switching, based on timing - maybe lastClock?)

 (*) = testing stage
 (p) = work in progress
 */

#include <SoftwareSerial.h>
// old:
// #include <EEPROM.h>
// new
#include <avr/eeprom.h>

#define EEMEM   __attribute__((section(".eeprom")))
// #define eeprom_is_ready()
// #define eeprom_busy_wait() do {} while (!eeprom_is_ready())
// end new
// Using arrays as "clicktables" to store which clicks get a trigger (1=trigger, 0=no trigger)
// Put what ever you want in here!
// 1  2  3  4| 5  6  7  8| 9  10 11 12|13 14 15 16

boolean clickTables[16][16] = {
  {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }
  ,//1
  {
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }
  ,//2
  {
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }
  ,//3
  {
    1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1
  }
  ,//4
  {
    1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0
  }
  ,//5
  {
    1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0
  }
  ,//6
  {
    1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1
  }
  ,//7
  {
    1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1
  }
  ,//8
  {
    1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1
  }
  ,//9
  {
    1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1
  }
  ,//10
  {
    1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0
  }
  ,//11
  {
    1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0
  }
  ,//12
  {
    1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1
  }
  ,//13
  {
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0
  }
  ,//14
  {
    1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0
  }
  ,//15
  {
    1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0
  } //16
};



/* struct for linear feedback shift register, 2 taps: */

struct LFSR {
  uint16_t data;  // or uint32_t data;
  uint8_t length; // max 16/32
  uint8_t bit_0;
  uint8_t tap1;
  uint8_t tap2;
};

/*  array of shift registers */
struct LFSR shift_regs[6];

/*   parameters stuff   */
const int8_t _parameters = 16;
byte outParam[6][_parameters];
// int para_count = 2 + 6*_parameters;
/* parameters for each of the 6 outputs: */
/*
 0: Mode (0 clicktable; 1 logic; 2 Euclidean; 3 LSFR ; 4 Random; 5 Clock division)
 1: Offset (128 = no offset)
 2: Trigger length or Gate > 99
 3: Clicktable number
 4: Length (if clicktable)
 5: Logic mode (1-7) 1= NOT; 2= AND; 3= OR; 4= NAND; 5= NOR; 6= XOR; 7= XNOR
 6: 1st Output to base logic on (Outputs must be lower number than the output doing the logic!)
 7: 2nd Output to base logic on
 8: Euclidean length (beats)
 9: Euclidean pulses (groups)
 10: tap1
 11: tap2
 12: LFSR length
 13: Clock divisor
 14: N / random threshold
 15: division offset
 */

#define OUT_PORT PORTB //pins 8-13 as outputs
#define PORT_DIRECTION DDRB

byte paramA = 0; //variable for selecting output to work on
int clickLength = 16; // was int
// const int offsetRange = 16; //min and max offset amount
boolean logicStore[6][16]; //store logic values for delay
int activeChannel = 0; // was int
String tempStringX = "";
byte nextGate = 0; //stores binary gates to write to OUT_PORT
unsigned long nextClock = 0; //when is next pulse
unsigned long nextOffClock[6] = {0, 0, 0, 0, 0, 0}; //when to turn triggers off
const int trigLen = 20; //default length of triggers
boolean noLow[6] = {false, false, false, false, false, false}; //only turn gates off once per clock loop
// byte offGate = 0; //for turning off triggers selectively

signed int div_count[6] = {0, 0, 0, 0, 0, 0}; // counters for clock division
int old_div_offset[6] = {0, 0, 0, 0, 0, 0}; // division offset, 0 =< x < divisor

const int clockPin = A0; //input for external clock
int clockValue = 0; //variable to store value from clock input
boolean clockInternal = false; //select internal or external clock
boolean clockPrevious = false;
boolean clock = false; // clock or not?
boolean clockFirst = false; // first Clock? 
unsigned long lastClock = 0; //for external clock calculation
const int clockThreshold = 800;
boolean seqStop = false;
boolean extClock = false;
unsigned long nowTime = 0; // makrospex: variable for holding millis();
unsigned long lastBPMupdate = nowTime;
boolean fromIntClock = false;
const int buttonThreshold = 400;

const int buttonPin = 7; //encoder button for menu system
boolean button1;
boolean b1Pushed = false;
boolean b1Previous = false;
unsigned long lastButtonTime = 0;
// const uint8_t debounceTime = 200; // now local

const int saveButtonPin = A5; //enter save/read mode
// const int buttonThreshold = 400;// now local
int saveRead = 255;
boolean saveButton = false;
int saveSlot = 0; // was int
// const int saveDebounce = 500;
unsigned long lastSave = 0;

const int syncButtonPin = A1; //synchronize/reset patterns
boolean syncButton = false;
int syncRead = 0;
// const int syncDebounce = 500; //Keep sync from happening too often
unsigned long lastSync = 0;

const int syncIntExtPin = A3; //select internal/external sync
int sourceRead = 0; //temp variable for reading int/ext switch
unsigned long switchPrevious = nowTime;
const int switchThreshold = 400;
//Defaults
int rateMain = 250; //internal clock interval in ms
String BPM = "60"; // makrospex // set according to rateMain.
int iBPM = 60; // makrospex // set exactly like BPM (was int)
int swing = 0; //range -99 to 99 (percent)
boolean swingToggle = 0; //swing this beat or not
float swingMod = 0; //amount to add or subtract from the beat for swing
boolean swingExtClockEnable = false; // enable swing on external Clock mode (experimental)
boolean isSwingClock = 0; // track if current clock is swinged (pushed ahead by swingMod (ms)
int clickStep[6] = {
  0, 0, 0, 0, 0, 0
}; //array to track which step each pattern is on
boolean outTemp[6] = {
  false, false, false, false, false, false
}; //gather output values
boolean outPrev[6] = {
  false, false, false, false, false, false
}; //keep track of current output status
boolean rhythm[32]; //array for Euclidean pattern
const uint8_t SR_length = 16; // max length shift registers

extern int encoderMode; //for menu system (was int)

volatile int tmpdata = 0; //variable to pass encoder value

const int txPin = 5;  // serial output pin
const int rxPin = 4;  // unused, but SoftwareSerial needs it...


SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

void setup() {
  delay(100);
  Serial.begin(9600);
  randomSeed(analogRead(A2));
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
  // init with 60 bpm internal clocking:
  rateMain = 250;
  iBPM = 60;
  BPM = "60";
  nowTime = millis();
  readSwitch(); // Read int/ext switch setting at startup
}

/*
main loop
*/

void loop() {
  nowTime = millis();
  readInputs();
  /*
  // handling of save/load without encoder action (alpha state): 
  if (tmpdata == 0 && encoderMode == 15) { 
    doUpdates(); 
  }
  else if (tmpdata == 0 && encoderMode == 16) { 
    doUpdates(); 
  }
  // end handling of save/load
  else */ if (tmpdata) {
    doUpdates();
    tmpdata = 0;
  }
  
  if (clockInternal) {
    if (!fromIntClock) fromIntClock = true;
    if (seqStop || extClock) {
      nextClock = nowTime + rateMain;
      seqStop = false;
      extClock = false;
    }
    if (nowTime >= nextClock) {
      clockAction();
      //nextOffClock = nextClock + trigLen;
      for (int x = 0; x < 6; x++) {
        if (outParam[x][2] < 100) nextOffClock[x] = nextClock + outParam[x][2]; //time to make triggers go low
        else nextOffClock[x] = nextClock + rateMain - 10; // NEW GATE MODE
        // old:
        // nextOffClock[x] = nextClock + outParam[x][2]; //time to make triggers go low
      }
      swingMod = ((float)rateMain * ((float)swing / 100));
      if (swingToggle) nextClock = nextClock + rateMain + swingMod;
      else nextClock = nextClock + rateMain - swingMod;
      lastClock = nowTime;
      if (rateMain < 30) rateMain = 30;
      if (encoderMode == 1) {
        // makrospex;
        if (nowTime >= (lastBPMupdate + 250) && encoderMode == 1) {
          if (iBPM > 0) {
            rateMain = (int)(((60.0 / (float)iBPM) * 250.0) + 0.5);
            BPM = String(iBPM);
          }
          else {
            iBPM = (15000 / rateMain);
            BPM = String(iBPM);
          }
          tempStringX = "";
          tempStringX += BPM;
          displayLED(tempStringX);
          lastBPMupdate = nowTime;
        }
      }
      swingToggle = !swingToggle;
    }
  }
  else { //external clock
    extClock = true;
    // handling of int/ext switch bug:
    if (fromIntClock) {
      // nowTime=millis();
      fromIntClock = false;
    }
    // end handling 
    if (clock && lastClock) { // old: if (clock) { 
      clockAction();
      
      for (int x = 0; x < 6; x++) {
        if (outParam[x][2] < 100) nextOffClock[x] = nextClock + outParam[x][2]; //time to make triggers go low
        else nextOffClock[x] = nextClock + rateMain - 10; // NEW GATE MODE
        // old:
        // nextOffClock[x] = nextClock + outParam[x][2]; //time to make triggers go low
      }
      /* experimental swing (makrospex)
      if (swingExtClockEnable) {
        swingMod = ((float)rateMain * ((float)swing / 100));
        if (swingToggle) {
          nextClock = nextClock + rateMain + swingMod;
        }
        else {
          nextClock = nextClock + rateMain - swingMod;
        }
        swingToggle = !swingToggle;
      }
      else {
        nextClock = nextClock + rateMain;
      }
      */
      // lastClock = nowTime;
      if (!nextClock) nextClock = nowTime + rateMain;
      else nextClock = nextClock + rateMain;
      
      if (rateMain < 30) rateMain = 30;
      
      
      if (nowTime >= (lastBPMupdate + 250) && encoderMode == 1) {
        iBPM = (int)((15000.0 / float(rateMain)) + 0.5);
        BPM = String(iBPM);
        tempStringX = "";
        tempStringX += BPM;
        displayLED(tempStringX);
        lastBPMupdate = nowTime;
      }
    }
    if (seqStop) { // updated while seq is stopped
      for (int x = 0; x < 6; x++) {
        if (outParam[x][2] < 100) nextOffClock[x] = nextClock + outParam[x][2]; //time to make triggers go low
        else nextOffClock[x] = nextClock + rateMain - 10; // NEW GATE MODE
        // old:
        // nextOffClock[x] = nextClock + outParam[x][2]; //time to make triggers go low
      }
    }
  }


  if (b1Pushed) {
    doButton();
    b1Pushed = false;
  }

  // take pins low?
  if ((nowTime >= nextOffClock[0]) && (noLow[0] == false)) gatesLow(0);
  if ((nowTime >= nextOffClock[1]) && (noLow[1] == false)) gatesLow(1);
  if ((nowTime >= nextOffClock[2]) && (noLow[2] == false)) gatesLow(2);
  if ((nowTime >= nextOffClock[3]) && (noLow[3] == false)) gatesLow(3);
  if ((nowTime >= nextOffClock[4]) && (noLow[4] == false)) gatesLow(4);
  if ((nowTime >= nextOffClock[5]) && (noLow[5] == false)) gatesLow(5);

  if (saveButton) {
    lastSave = nowTime;
    encoderMode = 14;
    displayLED(F("FILE"));
  }

  if (syncButton) {
    lastSync = nowTime;
    synchronize();
  }
}


void readInputs() {
  const int debounceTime = 50;
  //read encoder button
  button1 = digitalRead(buttonPin);
  if (button1) {
    if ((b1Previous == false) && (nowTime - lastButtonTime > debounceTime)) {
      b1Pushed = true;
      lastButtonTime = nowTime;
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
  if (saveButton && ((nowTime - lastSave) > 500)) saveButton = true;
  else saveButton = false;

  //read sync button
  syncRead = analogRead(syncButtonPin);
  if (syncRead < buttonThreshold) syncButton = false;
  else syncButton = true;
  if (syncButton && ((nowTime - lastSync) > 500)) syncButton = true;
  else syncButton = false;

  //read int/ext switch
  if (nowTime - switchPrevious > debounceTime) {
    sourceRead = analogRead(syncIntExtPin);
    if (sourceRead > switchThreshold) clockInternal = true;
    else if (sourceRead <= buttonThreshold) clockInternal = false;
    switchPrevious = nowTime;
  }
  //read clock pin
  if (!clockInternal) {
    clockValue = analogRead(clockPin);
    if (clockValue >= clockThreshold) {
      if (!clockPrevious) {
        // makrospex:
        if (!lastClock) {
          clock=false;
          lastClock=nowTime;
        }
        else {
          clock = true;
        }
        // end makrospex
        clock = true;
        clockPrevious = true;
      }
      else clock = false;
    }
    else {
      clock = false;
      clockPrevious = false;
      if (nowTime - lastClock > 2000) seqStop = true; //stop if no clock for 2 sec.
    }
    if (clock) {
      rateMain = nowTime - lastClock;
      lastClock = nowTime;
      seqStop = false; 
    }
  }
}

void readSwitch() {
  sourceRead = analogRead(syncIntExtPin);
  if (sourceRead > switchThreshold) {
    clockInternal = true;
  }
  else if (sourceRead <= buttonThreshold) {
    clockInternal = false;
    seqStop = true;
  }  
  switchPrevious = millis();
}


void clockAction() {
  for (int i = 0; i < 6; i++) { //go through each output A-F
    noLow[i] = false;

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

    else if (outParam[i][0] == 3) {  //output is lfsr

      shift_regs[i].tap1 = outParam[i][10];
      shift_regs[i].tap2 = outParam[i][11];
      shift_regs[i].length = outParam[i][12];
      update_lfsr(&shift_regs[i]);
      outTemp[i] = shift_regs[i].bit_0;
    }

    else if (outParam[i][0] == 4) {  //output is random

      int Rthreshold = outParam[i][14];
      if (random(Rthreshold + 2) > Rthreshold) outTemp[i] = 1;
      else outTemp[i] = 0;

    }

    else if (outParam[i][0] == 5) {  //output is division
      if (old_div_offset[i] == outParam[i][15]) {
        div_count[i]++;
      }
      else {
        div_count[i]++;
        div_count[i] += outParam[i][15] - old_div_offset[i];
        old_div_offset[i] = outParam[i][15];
      }
      // div_count[i]++;
      if (div_count[i] >= outParam[i][13]) {
        outTemp[i] = 1;
        div_count[i] = 0;
      }
      else  outTemp[i] = 0;
    }


    else if (outParam[i][0] == 1) { //output is logic;
      //NOTE: user must make sure logic outputs come after the ouputs they refer to, or logic will be based on previous beat
      byte baseA = outParam[i][6];//which output is base
      byte baseB = outParam[i][7];
      baseA = outTemp[baseA];//change base to value of click at that output
      baseB = outTemp[baseB];

      for (int j = 16; j > 0; j--) { //Slide all the values down for delay table
        logicStore[i][j] = logicStore[i][j - 1];
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
      int _offset = outParam[i][1] - 128;
      if (_offset < 1) _offset = 1;
      if (_offset > 16) _offset = 16;
      outTemp[i] = logicStore[i][_offset - 1];
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
      else skipXTime = floor((pulses - remainder) / noSkip);

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



  for (int k = 5; k > -1; k--) { //do outputs
    if (outParam[k][2] > 99) {
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

/*   take pin x low  */

void gatesLow(int _outputx) {
  OUT_PORT  &= ~(1 << _outputx);
  //OUT_PORT = offGate;
  //offGate = 0;
  noLow[_outputx] = true;
}

///////////////

void synchronize() { //resets all counters
  // todo: add code for generating random parameters on currently active channel (use (int) activeChannel(=0) (Chan. A))
  displayLED(F("SYNC"));
  for (int i = 0; i < 6; i++) {
    clickStep[i] = 0;
  }
  // swingToggle = 0;
  /*
  for (int i = 0; i < 6; i++) { // reset division offsets
     outParam[i][15] = 0;
   }

  for (int k = 0; k < 6; k++){
     outParam[k][0] = 5 ; //mode
   }
   */
  doUpdates();
}

void initParams() { //sets some default values for outParam
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < _parameters; j++) {
      if (j == 1) outParam[i][j] = 128;
      else if (j == 2) outParam[i][j] = trigLen;
      else if (j == 0) outParam[i][j] = 5;
      else if (j == 12) outParam[i][j] = SR_length;
      else if (j == 4) outParam[i][j] = clickLength;
      else outParam[i][j] = 0;
    }
  }
  for (int k = 0; k < 6; k++) {
    shift_regs[k].length =  SR_length;
    shift_regs[k].tap1 =  0;
    shift_regs[k].tap2 =  0;
    shift_regs[k].data =  random(SR_length);
    shift_regs[k].bit_0 = shift_regs[k].data & 1u;
  }
}

///////////////////////////

void saveSettings() { //will overwrite settings in EEPROM without warning!
  int slotByte = (saveSlot * 98); // paramaters to save, slotByte is first address per set
  int slotCounter = 0;
  cli(); // disable interrupts
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < _parameters; j++) {
      if (eeprom_read_byte((uint8_t*)(slotByte + slotCounter)) != outParam[i][j]) {
        eeprom_write_byte((uint8_t*)(slotByte + slotCounter), outParam[i][j]);
      }
      slotCounter ++;
    }
  }
  if (eeprom_read_byte((uint8_t*)(slotByte + slotCounter)) != swing) {
    eeprom_write_byte((uint8_t*)(slotByte + slotCounter), swing);
  }
  slotCounter ++;
  int tempRate = (rateMain > 255) ? 255 : rateMain; //can't store more than 1 byte per EEPROM slot
  if (eeprom_read_byte((uint8_t*)(slotByte + slotCounter)) != tempRate) {
    eeprom_write_byte((uint8_t*)(slotByte + slotCounter), tempRate);
  }
  sei(); // re-enable interrupts
}


void loadSettings() {
  int slotByte = (saveSlot * 98); // paramaters to save, slotByte is first address per set
  int slotCounter = 0;
  int rateMainOld = 0;
  cli(); // disable interrupts
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < _parameters; j++) {
      outParam[i][j] = eeprom_read_byte((uint8_t*)(slotByte + slotCounter));
      slotCounter ++;
    }
  }
  swing = eeprom_read_byte((uint8_t*)(slotByte + slotCounter));
  slotCounter ++;
  rateMainOld = rateMain;
  rateMain = eeprom_read_byte((uint8_t*)(slotByte + slotCounter));
  sei(); // enable interrupts
  if (rateMain != rateMainOld) {
    iBPM = (int)(15000 / rateMain);
    BPM = String(iBPM);
  }
}

