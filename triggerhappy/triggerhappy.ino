//triggerhappy by Karl Gruenewald (studiokpg@gmail.com)
/* 
 *  Trigger generator for modular synthesizers.
 *  Uses internal or external clock
 *  Has 6 outputs (A-F) which can each be set to one of 6 kinds of output:
 *  Clicktable: read through a static rhythm pattern, offset and length can be changed
 *  Logic: choose from 7 logic modes based on any two other outputs
 *  Euclidean: generate an Euclidean rhythm pattern given total length and number of groups
 *  LFSR: ...LFSR, adjust length, tap1 and tap2
 *  Random: random [1 ... N+1] > threshold N
 *  Division: clock division: none, 2, 3, 4 ... 16
 *  Save and load settings 0-13
 *  (Can play, but can't store tempos slower than 59 BPM)
 *  Swing, +/- 99%

 *  IMPORTANT: When putting this software on a new Arduino, you need to comment out the loadSettings()
 *  call in setup(). Otherwise, you will have garbage settings and things may not work. Save the default
 *  settings to slot 0, then you can re-enable the loadSettings() line.


 *  makrospex TODO:
 *  1. (*) swing on exernal clock
 *  4. (*) saveButton = 4. function Reset (like Sync, set all parameters to defaults on all channels)
 *  5. (*) repair menusystem (eeprom slot select right before saving/loading, without showing bpm before selection of eeprom slot) (impossible without tmpdata change?
 *  6. (*) int/ext switch bug (random trigger lengths after switching, based on timing - maybe lastClock?)

 *  (*) = testing stage
 *  (p) = work in progress
 */

#include <SoftwareSerial.h>
#include <avr/eeprom.h>

#define EEMEM   __attribute__((section(".eeprom")))

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
    //1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0
    0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0 //tom ITAT
  }
  ,//14
  {
    //1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 //open ITAT
    1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0
  }
  ,//15
  {
  //  1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0
    0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 //bassdrum ITAT 
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
/* parameters for each of the 6 outputs: 
 * 
 *  0: Mode (0 clicktable; 1 logic; 2 Euclidean; 3 LSFR ; 4 Random; 5 Clock division)
 *  1: Offset (128 = no offset)
 *  2: Trigger length or Gate > 99
 *  3: Clicktable number
 *  4: Length (if clicktable)
 *  5: Logic mode (1-7) 1= NOT; 2= AND; 3= OR; 4= NAND; 5= NOR; 6= XOR; 7= XNOR
 *  6: 1st Output to base logic on (Outputs must be lower number than the output doing the logic!)
 *  7: 2nd Output to base logic on
 *  8: Euclidean length (beats)
 *  9: Euclidean pulses (groups)
 *  10: tap1
 *  11: tap2
 *  12: LFSR length
 *  13: Clock divisor
 *  14: N / random threshold
 *  15: division offset
 */
struct ChannelParams {
    unsigned char mode;
    unsigned char offset; //0 Offset?
    unsigned char triggerLength;
    unsigned char clickTable; // 1 subtype
    unsigned char clickTableLength; //2 Length
    unsigned char logicMode; // 1 subtype
    unsigned char logicOutput1; //3 param1
    unsigned char logicOutput2; //4 param2
    unsigned char euclidianLength; //2 Length
    unsigned char euclidianPulses; //3 param1
    unsigned char lfsrTap1; //3 param1
    unsigned char lfsrTap2; //4 param2
    unsigned char lfsrLength; //2 Length
    unsigned char clockDivisor; // 1 subtype
    unsigned char randomThreshold; //1 subtype?
    unsigned char divisionOffset; //0 Offset?
};

struct PatchSettings {    
    ChannelParams channel[6];   
    unsigned char swing;
    unsigned int rate;
};

PatchSettings patch;
/*
 * Hardware options
 */
#define OUT_PORT PORTB //pins 8-13 as outputs
#define PORT_DIRECTION DDRB

//Inverse buttons: when al buttons en encoders hava a pullup instead of a pulldown resistor
// This enabled eg usage of a switching jack for int ext
#define INVERSE_BUTTONS
/*
 * Consts
 */
const int clickLength = 16; // was int
const int trigLen = 20; //default length of triggers
const int clockThreshold = 800;
const int buttonThreshold = 400;
const int switchThreshold = 400;

const uint8_t SR_length = 16; // max length shift registers

const int slotSize = 100;
const int saveSlotMax = 9; //max slot# that can be saved to

/*
 * Pin connections
 */
const int clockPin = A0; //input for external clock
const int buttonPin = 7; //encoder button for menu system
const int saveButtonPin = A1; //enter save/read mode
const int syncButtonPin = A5; //synchronize/reset patterns
const int syncIntExtPin = A3; //select internal/external sync

/* 
 *  Variables
 */
unsigned int selectedChannel = 0;
boolean logicStore[6][16]; //store logic values for delay
byte nextGate = 0; //stores binary gates to write to OUT_PORT
unsigned long nextClock = 0; //when is next pulse
unsigned long nextOffClock[6] = {0, 0, 0, 0, 0, 0}; //when to turn triggers off
boolean noLow[6] = {false, false, false, false, false, false}; //only turn gates off once per clock loop

signed int div_count[6] = {0, 0, 0, 0, 0, 0}; // counters for clock division
int old_div_offset[6] = {0, 0, 0, 0, 0, 0}; // division offset, 0 =< x < divisor

boolean clockInternal = false; //select internal or external clock
boolean clockPrevious = false;
boolean clock = false; // clock or not?
unsigned long lastClock = 0; //for external clock calculation
boolean seqStop = false;
boolean extClock = false;
unsigned long nowTime = 0; // makrospex: variable for holding millis();
unsigned long lastBPMupdate = nowTime;
boolean fromIntClock = false;

boolean b1Pushed = false;
boolean b1Previous = false;
unsigned long lastButtonTime = 0;

boolean saveButton = false;


int saveSlot = 0; // was int
unsigned long lastSave = 0;

boolean syncButton = false;
unsigned long lastSync = 0;

unsigned long switchPrevious = nowTime;

//Defaults
String BPM = "60"; // makrospex // set according to patch.rate.
int iBPM = 60; // makrospex // set exactly like BPM (was int)
boolean swingToggle = 0; //swing this beat or not
float swingMod = 0; //amount to add or subtract from the beat for swing
boolean swingExtClockEnable = false; // enable swing on external Clock mode (experimental)
boolean isSwingClock = 0; // track if current clock is swinged (pushed ahead by swingMod (ms)
int clickStep[6] = { 0, 0, 0, 0, 0, 0}; //array to track which step each pattern is on
boolean outTemp[6] = { false, false, false, false, false, false}; //gather output values
boolean outPrev[6] = { false, false, false, false, false, false}; //keep track of current output status
boolean rhythm[32]; //array for Euclidean pattern

extern int encoderMode; //for menu system (was int)

volatile int tmpdata = 0; //variable to pass encoder value


void setup() {
  delay(100);
  Serial.begin(9600);
  Serial.println("Triggerhappy 1.08 beta starting");
  randomSeed(analogRead(A2));
  setupEncoder();
  setupDisplay();
  PORT_DIRECTION = B111111;
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  initParams();// initializes outParams array
  pinMode(clockPin, INPUT);
  pinMode(saveButtonPin, INPUT);
  digitalWrite(saveButtonPin, HIGH);
  pinMode(syncButtonPin, INPUT);
  digitalWrite(syncButtonPin, HIGH);
  pinMode(syncIntExtPin, INPUT);
  digitalWrite(syncIntExtPin, HIGH);
  // init with 60 bpm internal clocking:
  patch.rate = 250;
  iBPM = 60;
  BPM = "60";

  
  
  //IMPORTANT: comment out the following line first time you run this sketch!
  loadSettings(); //loads from save location 0 on startup (will load garbage until you do a save to slot 0!)

  //new Arduino
  if (patch.channel[0].triggerLength==0) {
    initSettings();
    saveSettings();
  }
  
  nowTime = millis();
  readSwitch(); // Read int/ext switch setting at startup
}

/*
   main loop
*/

void loop() {
  nowTime = millis();
  readInputs();
 
  if (tmpdata) {
    doUpdates();
    tmpdata = 0;
  }
  
  if (clockInternal) {
    if (!fromIntClock) fromIntClock = true;
    if (seqStop || extClock) {
      nextClock = nowTime + patch.rate;
      seqStop = false;
      extClock = false;
    }
    if (nowTime >= nextClock) {
      clockAction();

      for (int x = 0; x < 6; x++) {
        if (patch.channel[x].triggerLength < 100) nextOffClock[x] = nextClock + patch.channel[x].triggerLength; //time to make triggers go low
        else nextOffClock[x] = nextClock + patch.rate - 10; // NEW GATE MODE
      }
      swingMod = ((float)patch.rate * ((float)patch.swing / 100));
      if (swingToggle) nextClock = nextClock + patch.rate + swingMod;
      else nextClock = nextClock + patch.rate - swingMod;
      lastClock = nowTime;
      if (patch.rate < 30) patch.rate = 30;
      if (encoderMode == 1) {
        // makrospex;
        if (nowTime >= (lastBPMupdate + 250) && encoderMode == 1) {
          if ((iBPM > 1)&&(iBPM<500)) {
            patch.rate = (int)(((60.0 / (float)iBPM) * 250.0) + 0.5);
            BPM = String(iBPM);
          }
          else {
            iBPM = (15000 / patch.rate);
            BPM = String(iBPM);
          }
          String tempStringX = "";
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
        if (patch.channel[x].triggerLength < 100) nextOffClock[x] = nextClock + patch.channel[x].triggerLength; //time to make triggers go low
        else nextOffClock[x] = nextClock + patch.rate - 10; // NEW GATE MODE
      }
      /* experimental swing (makrospex)
      if (swingExtClockEnable) {
        swingMod = ((float)patch.rate * ((float)swing / 100));
        if (swingToggle) {
          nextClock = nextClock + patch.rate + swingMod;
        }
        else {
          nextClock = nextClock + patch.rate - swingMod;
        }
        swingToggle = !swingToggle;
      }
      else {
        nextClock = nextClock + patch.rate;
      }
      */
      if (!nextClock) nextClock = nowTime + patch.rate;
      else nextClock = nextClock + patch.rate;
      
      if (patch.rate < 30) patch.rate = 30;
      
      
      if (nowTime >= (lastBPMupdate + 250) && encoderMode == 1) {
        iBPM = (int)((15000.0 / float(patch.rate)) + 0.5);
        BPM = String(iBPM);
        String tempStringX = "";
        tempStringX += BPM;
        displayLED(tempStringX);
        lastBPMupdate = nowTime;
      }
    }
    if (seqStop) { // updated while seq is stopped
      for (int x = 0; x < 6; x++) {
        if (patch.channel[x].triggerLength < 100) nextOffClock[x] = nextClock + patch.channel[x].triggerLength; //time to make triggers go low
        else nextOffClock[x] = nextClock + patch.rate - 10; // NEW GATE MODE
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
  boolean button1 = digitalRead(buttonPin);

#ifdef INVERSE_BUTTONS
  button1 = !button1;
#endif
  
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
  int saveRead = analogRead(saveButtonPin);
#ifdef INVERSE_BUTTONS
  if (saveRead > buttonThreshold) saveButton = false;
  else saveButton = true;
#else
  if (saveRead < buttonThreshold) saveButton = false;
  else saveButton = true;
#endif
  if (saveButton && ((nowTime - lastSave) > 500)) saveButton = true;
  else saveButton = false;

  //read sync button
  int syncRead = analogRead(syncButtonPin);
#ifdef INVERSE_BUTTONS
  if (syncRead > buttonThreshold) syncButton = false;
  else syncButton = true;
#else
  if (syncRead < buttonThreshold) syncButton = false;
  else syncButton = true;
#endif
  if (syncButton && ((nowTime - lastSync) > 500)) syncButton = true;
  else syncButton = false;

  //read int/ext switch
  if (nowTime - switchPrevious > debounceTime) {
    int sourceRead = analogRead(syncIntExtPin);
#ifdef INVERSE_BUTTONS
    if (sourceRead < switchThreshold) clockInternal = true;
    else if (sourceRead >= switchThreshold) clockInternal = false;
#else
    if (sourceRead > switchThreshold) clockInternal = true;
    else if (sourceRead <= switchThreshold) clockInternal = false;
#endif
    switchPrevious = nowTime;
  }
  //read clock pin
  if (!clockInternal) {
    int clockValue = analogRead(clockPin);
#ifdef INVERSE_BUTTONS
    if (clockValue <= clockThreshold) {
#else
    if (clockValue >= clockThreshold) {
#endif
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
      patch.rate = nowTime - lastClock;
      lastClock = nowTime;
      seqStop = false; 
    }
  }
}

void readSwitch() {
  int sourceRead = analogRead(syncIntExtPin);
#ifdef INVERSE_BUTTONS
  if (sourceRead < switchThreshold) {
    clockInternal = true;
  }
  else if (sourceRead >= switchThreshold) {
    clockInternal = false;
    seqStop = true;
  }
#else
  if (sourceRead > switchThreshold) {
    clockInternal = true;
  }
  else if (sourceRead <= switchThreshold) {
    clockInternal = false;
    seqStop = true;
  }  
#endif
  switchPrevious = millis();
}


void clockAction() {
  //calculate processing order
  unsigned int index[6]= {0, 0, 0, 0, 0, 0};
  int startIndex = -1;
  int endIndex = 6;
  for (int i = 0; i< 6; i++) {
    if (patch.channel[i].mode == 1) {
      endIndex--;
      index[endIndex] = i;
    } else {
      startIndex ++;
      index[startIndex] = i;
    }
  }
  
  for (int ii = 0; ii < 6; ii++) { //go through index[]
    int i = index[ii]; //set original i to index[ii]  
  
    noLow[i] = false;

    if (patch.channel[i].mode == 0) { //output is clicktable
      int length = patch.channel[i].clickTableLength;
      int offset = patch.channel[i].offset - 128;
      if (abs(offset) > length) offset = offset % length;
      int counter = clickStep[i] + offset;
      if (counter > length - 1) counter = counter - length;
      if (counter < 0) counter = counter + length;
      int thisArray = patch.channel[i].clickTable;
      outTemp[i] = clickTables[thisArray][counter];
      clickStep[i]++;
      if (clickStep[i] > length - 1) clickStep[i] = 0;
    }

    else if (patch.channel[i].mode == 3) {  //output is lfsr

      shift_regs[i].tap1 = patch.channel[i].lfsrTap1;
      shift_regs[i].tap2 = patch.channel[i].lfsrTap2;
      shift_regs[i].length = patch.channel[i].lfsrLength;
      update_lfsr(&shift_regs[i]);
      outTemp[i] = shift_regs[i].bit_0;
    }

    else if (patch.channel[i].mode == 4) {  //output is random

      int Rthreshold = patch.channel[i].randomThreshold;
      if (random(Rthreshold + 2) > Rthreshold) outTemp[i] = 1;
      else outTemp[i] = 0;

    }

    else if (patch.channel[i].mode == 5) {  //output is division
      if (old_div_offset[i] == patch.channel[i].divisionOffset) {
        div_count[i]++;
      }
      else {
        div_count[i]++;
        div_count[i] += patch.channel[i].divisionOffset - old_div_offset[i];
        old_div_offset[i] = patch.channel[i].divisionOffset;
      }
      if (div_count[i] >= patch.channel[i].clockDivisor) {
        outTemp[i] = 1;
        div_count[i] = 0;
      }
      else  outTemp[i] = 0;
    }


    else if (patch.channel[i].mode == 1) { //output is logic;
      //NOTE: user must make sure logic outputs come after the ouputs they refer to, or logic will be based on previous beat
      byte baseA = patch.channel[i].logicOutput1;//which output is base
      byte baseB = patch.channel[i].logicOutput2;
      baseA = outTemp[baseA];//change base to value of click at that output
      baseB = outTemp[baseB];

      for (int j = 16; j > 0; j--) { //Slide all the values down for delay table
        logicStore[i][j] = logicStore[i][j - 1];
      }

      switch (patch.channel[i].logicMode) { //logic calculations

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
      int _offset = patch.channel[i].offset - 128;
      if (_offset < 1) _offset = 1;
      if (_offset > 16) _offset = 16;
      outTemp[i] = logicStore[i][_offset - 1];
    }

    else {  //output is Euclidean
      // based on http://kreese.net/blog/2010/03/27/generating-musical-rhythms/
      int steps = patch.channel[i].euclidianLength; //beats
      int pulses = patch.channel[i].euclidianPulses; //groups
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

      int c = clickStep[i] + patch.channel[i].offset - 128;
      c = abs(c % steps);
      outTemp[i] = rhythm[c];
      clickStep[i]++;
      if (clickStep[i] >= steps) clickStep[i] = 0;
    }
  }



  for (int k = 5; k > -1; k--) { //do outputs
    if (patch.channel[k].triggerLength > 99) {
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
  displayLED(F("SYNC"));
  for (int i = 0; i < 6; i++) {
    clickStep[i] = 0;
  }
  // swingToggle = 0;
  doUpdates();
}

void initParams() { //sets some default values for params
  for (int i = 0; i < 6; i++) {
      patch.channel[i].mode = 5;
      patch.channel[i].offset = 128;
      patch.channel[i].triggerLength = trigLen;
      patch.channel[i].clickTable =0;
      patch.channel[i].clickTableLength = clickLength;
      patch.channel[i].logicMode = 0; // 5: Logic mode (1-7) 1= NOT; 2= AND; 3= OR; 4= NAND; 5= NOR; 6= XOR; 7= XNOR
      patch.channel[i].logicOutput1 = 0; // 6: 1st Output to base logic on (Outputs must be lower number than the output doing the logic!)
      patch.channel[i].logicOutput2 = 0; // 7: 2nd Output to base logic on
      patch.channel[i].euclidianLength = 1; // 8: Euclidean length (beats)
      patch.channel[i].euclidianPulses = 1; //  9: Euclidean pulses (groups)
      patch.channel[i].lfsrTap1 = 0; // 10: tap1
      patch.channel[i].lfsrTap2 = 0; // 11: tap2
      patch.channel[i].lfsrLength = SR_length; // 12: LFSR length
      patch.channel[i].clockDivisor = 1; // 13: Clock divisor
      patch.channel[i].randomThreshold = 0; // 14: N / random threshold
      patch.channel[i].divisionOffset = 0; // 15: division offset
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

void checkAndSave(int index, unsigned char value){
  if (eeprom_read_byte((uint8_t*)index) != value) {
        eeprom_write_byte((uint8_t*)index, value);
  }      
}

void saveSettings() { //will overwrite settings in EEPROM without warning!
  //EM: 98-> slotSize
  int slotByte = (saveSlot * slotSize); // paramaters to save, slotByte is first address per set
  int slotCounter = 0;
  cli(); // disable interrupts
  for (int i = 0; i < 6; i++) {
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].mode);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].offset);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].triggerLength);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].clickTable);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].clickTableLength);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].logicMode);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].logicOutput1);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].logicOutput2);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].euclidianLength);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].euclidianPulses);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].lfsrTap1);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].lfsrTap2);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].lfsrLength);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].clockDivisor);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].randomThreshold);
    slotCounter ++;
    checkAndSave(slotByte + slotCounter, patch.channel[selectedChannel].divisionOffset);
    slotCounter ++;

  }
  checkAndSave(slotByte + slotCounter, patch.swing);
  slotCounter ++;
  //EM: int tempRate = (patch.rate > 255) ? 255 : patch.rate; //can't store more than 1 byte per EEPROM slot
  byte tempRate = patch.rate; //new code
  checkAndSave(slotByte + slotCounter, tempRate);
  
  //EM
  slotCounter ++;
  tempRate = patch.rate>>8; //new code
  checkAndSave(slotByte + slotCounter, tempRate);
  //EM
  sei(); // re-enable interrupts
}

void initSettings(){
// init with 60 bpm internal clocking:
  //patch.rate = 250;
  //iBPM = 60;
  //BPM = "60";

  for (int i = 0; i<6; i++) {
    patch.channel[selectedChannel].mode = 0; // 0: Mode (0 clicktable; 1 logic; 2 Euclidean; 3 LSFR ; 4 Random; 5 Clock division)
    patch.channel[selectedChannel].offset = 128; // 1: Offset (128 = no offset)
    patch.channel[selectedChannel].triggerLength = trigLen; // 2: Trigger length or Gate > 99
    patch.channel[selectedChannel].clickTable = 0; // 3: Clicktable number
    patch.channel[selectedChannel].clickTableLength = clickLength; // 4: Length (if clicktable)
    patch.channel[selectedChannel].logicMode = 0; // 5: Logic mode (1-7) 1= NOT; 2= AND; 3= OR; 4= NAND; 5= NOR; 6= XOR; 7= XNOR
    patch.channel[selectedChannel].logicOutput1 = 0; // 6: 1st Output to base logic on (Outputs must be lower number than the output doing the logic!)
    patch.channel[selectedChannel].logicOutput2 = 0; // 7: 2nd Output to base logic on
    patch.channel[selectedChannel].euclidianLength = 1; // 8: Euclidean length (beats)
    patch.channel[selectedChannel].euclidianPulses = 1; //  9: Euclidean pulses (groups)
    patch.channel[selectedChannel].lfsrTap1 = 0; // 10: tap1
    patch.channel[selectedChannel].lfsrTap2 = 0; // 11: tap2
    patch.channel[selectedChannel].lfsrLength = SR_length; // 12: LFSR length
    patch.channel[selectedChannel].clockDivisor = 1; // 13: Clock divisor
    patch.channel[selectedChannel].randomThreshold = 0; // 14: N / random threshold
    patch.channel[selectedChannel].divisionOffset = 0; // 15: division offset
  }
}



void loadSettings() {
  //EM: 98-> slotSize
  int slotByte = (saveSlot * slotSize); // paramaters to save, slotByte is first address per set
  int slotCounter = 0;
  int rateMainOld = 0;
  cli(); // disable interrupts
  for (int i = 0; i < 6; i++) {
      patch.channel[selectedChannel].mode = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 0: Mode (0 clicktable; 1 logic; 2 Euclidean; 3 LSFR ; 4 Random; 5 Clock division)
      slotCounter ++;
      patch.channel[selectedChannel].offset = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 1: Offset (128 = no offset)
      slotCounter ++;
      patch.channel[selectedChannel].triggerLength = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 2: Trigger length or Gate > 99
      slotCounter ++;
      patch.channel[selectedChannel].clickTable = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 3: Clicktable number
      slotCounter ++;
      patch.channel[selectedChannel].clickTableLength = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 4: Length (if clicktable)
      slotCounter ++;
      patch.channel[selectedChannel].logicMode = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 5: Logic mode (1-7) 1= NOT; 2= AND; 3= OR; 4= NAND; 5= NOR; 6= XOR; 7= XNOR
      slotCounter ++;
      patch.channel[selectedChannel].logicOutput1 = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 6: 1st Output to base logic on (Outputs must be lower number than the output doing the logic!)
      slotCounter ++;
      patch.channel[selectedChannel].logicOutput2 = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 7: 2nd Output to base logic on
      slotCounter ++;
      patch.channel[selectedChannel].euclidianLength = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 8: Euclidean length (beats)
      slotCounter ++;
      patch.channel[selectedChannel].euclidianPulses = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); //  9: Euclidean pulses (groups)
      slotCounter ++;
      patch.channel[selectedChannel].lfsrTap1 = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 10: tap1
      slotCounter ++;
      patch.channel[selectedChannel].lfsrTap2 = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 11: tap2
      slotCounter ++;
      patch.channel[selectedChannel].lfsrLength = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 12: LFSR length
      slotCounter ++;
      patch.channel[selectedChannel].clockDivisor = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 13: Clock divisor
      slotCounter ++;
      patch.channel[selectedChannel].randomThreshold = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 14: N / random threshold
      slotCounter ++;
      patch.channel[selectedChannel].divisionOffset = eeprom_read_byte((uint8_t*)(slotByte + slotCounter)); // 15: division offset
      slotCounter ++;
    }
  patch.swing = eeprom_read_byte((uint8_t*)(slotByte + slotCounter));
  slotCounter ++;
  rateMainOld = patch.rate;
  patch.rate = eeprom_read_byte((uint8_t*)(slotByte + slotCounter));
  //EM: extra code
  slotCounter ++;
  patch.rate += (eeprom_read_byte((uint8_t*)(slotByte + slotCounter))<<8);
  //
  sei(); // enable interrupts
  if (patch.rate != rateMainOld) {
    iBPM = (int)(15000 / patch.rate);
    BPM = String(iBPM);
  }
}


