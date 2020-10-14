int mainSelector = 1; //top level (was int)
int encoderMode = 1; //menu location pointer (was int)
int mode = 1; //variable for output mode (was int)
int arrayNumber = 0; //variable for array number assignment (was int)
int arrayLength = 32; //variable to set array length
int patternOffset = 128; //variable to set pattern offset
int relativeOffset = 0;
int trigGate = 20; //variable for trigger or gate selection
int logicMode = 0; //variable for logic mode selection
int logic1 = 0; //first logic input
int logic2 = 0; //second logic input
int EuclidBeats = 8; //Number of beats for Euclidean pattern
int EuclidGroups = 2; //Number of groups within Euclidean pattern
int saveFunction = 1; //Save or Load files
int tap_1 = 0;
int tap_2 = 0;
int LFSR_length = 16;
int divisor = 1;
int rand_n = 0;
int div_off = 0;

void displayMode(int locMode){
  switch (locMode) {
    case 0: displayLED(F("ARR ")); break;
    case 1: displayLED(F("LoG ")); break;
    case 2: displayLED(F("EuC ")); break;
    case 3: displayLED(F("LFSR")); break;
    case 4: displayLED(F("RAnd")); break;
    case 5: displayLED(F("d--n")); break;  
  }
}

void displayLogicMode(int locLogicMode){
  switch (locLogicMode) {
    case 0: displayLED(F("Not ")); break;
    case 1: displayLED(F("AND ")); break;
    case 2: displayLED(F(" oR ")); break;
    case 3: displayLED(F("NAND")); break;
    case 4: displayLED(F("nor ")); break;
    case 5: displayLED(F("Hor ")); break;  
    case 6: displayLED(F("Hnor")); break;  
  }
}

void displayParam(String locName, int value){
  String tempString = "";
  if (value > 9) tempString = locName;
  else tempString = locName+ " ";
  tempString += (value);
  displayLED(tempString);  
}

void displayOffset(String locName, int value){
  String tempString ="";
      if (value > 9) tempString = locName+" ";
      else if (value < -9) tempString =locName;
      else if (value < 0) tempString =locName+" ";
      else tempString = locName+"  ";
      tempString.concat(value);
      displayLED(tempString);
}

void doUpdates() //handles encoder input and display for menu system
{ 
  String tempString = "";
        
  switch (encoderMode) {
    
    case 1: //Adjust Rate Main
      if (clockInternal) {
        if ((iBPM > 20)&&(iBPM<500)) {
          iBPM = iBPM + tmpdata;
        }
        else {
          if (patch.rate > 0) {
            iBPM = (int)((15000/patch.rate) + tmpdata);
          }
        }
        patch.rate = (int)(((60.0/(float)iBPM) * 250.0) + 0.5); //quarter note(s)
        BPM = String(iBPM);
        tempString += BPM;
        displayLED(tempString);
      }
    break;
    
    case 2: //Output selector/Main Menu
      mainSelector = mainSelector + tmpdata;
      if (mainSelector > 8) mainSelector = 1;
      if (mainSelector < 1) mainSelector = 8;
      if (mainSelector == 1) displayLED(F("outA"));
      else if (mainSelector == 2) displayLED(F("outB")); 
      else if (mainSelector == 3) displayLED(F("outC")); 
      else if (mainSelector == 4) displayLED(F("outD")); 
      else if (mainSelector == 5) displayLED(F("outE")); 
      else if (mainSelector == 6) displayLED(F("outF"));
      else if (mainSelector == 7) displayLED(F("ShUF")); //swing ;)
      else if (mainSelector == 8) displayLED(F("RAtE"));
    break;
    
    case 3: //Select output mode
      mode = mode + tmpdata;
      if (mode > 5) mode = 0;
      if (mode < 0) mode = 5;
      displayMode(mode);
    break;
    
    case 4: //Select array number
      arrayNumber = arrayNumber + tmpdata;
      if (arrayNumber < 0) arrayNumber = 15;
      if (arrayNumber > 15) arrayNumber = 0;
      displayParam("AR", arrayNumber + 1);

      patch.channel[selectedChannel].clickTable = arrayNumber;
    break;
    
    case 5: //Select array length
      arrayLength = arrayLength + tmpdata;
      if (arrayLength < 1) arrayLength = 1;
      if (arrayLength > 16) arrayLength = 16;
      displayParam("AL", arrayLength);
      patch.channel[selectedChannel].clickTableLength = arrayLength;
    break;
    
    case 6: //Select pattern offset
      patternOffset = patternOffset + tmpdata;
      relativeOffset = patternOffset - 128;
      if (patternOffset < (128 - 16)) patternOffset = (128 - 16);
      if (patternOffset > (128 + 16)) patternOffset = (128 + 16);
      if (relativeOffset > 9) tempString = "O ";
      else if (relativeOffset < -9) tempString ="O";
      else if (relativeOffset < 0) tempString ="O ";
      else tempString = "O  ";
      tempString.concat(relativeOffset);
      displayLED(tempString);
      patch.channel[selectedChannel].offset = patternOffset;
    break;
    
    case 7: // adjust trigger length
      trigGate = trigGate + tmpdata;
      if (trigGate > 100) trigGate = 100;
      else if (trigGate < 5) trigGate = 5;
      if (trigGate == 100) displayLED(F("GAtE"));
      else displayParam("tr", trigGate);
      patch.channel[selectedChannel].triggerLength = trigGate;
    break;
    
    case 8: //select logic mode
      logicMode = logicMode + tmpdata;
      if (logicMode < 0) logicMode = 6;
      if (logicMode > 6) logicMode = 0;
      displayLogicMode(logicMode);
      patch.channel[selectedChannel].logicMode = logicMode;
    break;
    
    case 9: //select logic input 1
      logic1 = logic1 + tmpdata;
      if (logic1 < 0) logic1 = 5;
      if (logic1 > 5) logic1 = 0;
      if (logic1 == 0) displayLED(F("L1-A"));
      else if (logic1 == 1) displayLED(F("L1-B"));
      else if (logic1 == 2) displayLED(F("L1-C"));
      else if (logic1 == 3) displayLED(F("L1-D"));
      else if (logic1 == 4) displayLED(F("L1-E"));
      else if (logic1 == 5) displayLED(F("L1-F"));
      patch.channel[selectedChannel].logicOutput1 = logic1;
    break;
    
    case 10: //select logic input 2
      logic2 = logic2 + tmpdata;
      if (logic2 < 0) logic2 = 5;
      if (logic2 > 5) logic2 = 0;
      if (logic2 == 0) displayLED(F("L2-A"));
      else if (logic2 == 1) displayLED(F("L2-B"));
      else if (logic2 == 2) displayLED(F("L2-C"));
      else if (logic2 == 3) displayLED(F("L2-D"));
      else if (logic2 == 4) displayLED(F("L2-E"));
      else if (logic2 == 5) displayLED(F("L2-F"));
      patch.channel[selectedChannel].logicOutput2 = logic2;
    break;
    
    case 11: // + offset (delay)
      patternOffset = patternOffset + tmpdata;
      relativeOffset = patternOffset - 128;
      if (patternOffset < (128 - 16)) patternOffset = (128 - 16);
      if (patternOffset > (128 + 16)) patternOffset = (128 + 16);
      displayOffset("O", relativeOffset);
      patch.channel[selectedChannel].offset = patternOffset;
    break;
    
    case 12: //Euclidean #beats
      EuclidBeats = EuclidBeats + tmpdata;
      if (EuclidBeats < 1) EuclidBeats = 1;
      if (EuclidBeats > 32) EuclidBeats = 32;
      displayParam("b ", EuclidBeats);
      patch.channel[selectedChannel].euclidianLength = EuclidBeats;
    break;
    
    case 13: //Euclidean #groups
      EuclidGroups = EuclidGroups + tmpdata;
      if (EuclidGroups < 1) EuclidGroups = 1;
      if (EuclidGroups > EuclidBeats) EuclidGroups = EuclidBeats;
      displayParam("G ", EuclidGroups);
      patch.channel[selectedChannel].euclidianPulses = EuclidGroups;
    break;
    
    case 14: //Save & Read parameters from EEPROM
      saveFunction = saveFunction + tmpdata;
      if (saveFunction < 1) saveFunction = 4;
      if (saveFunction > 4) saveFunction = 1;
      if (saveFunction == 1) displayLED(F("SAUE"));
      else if (saveFunction == 2) displayLED(F("LOAd"));
      else if (saveFunction == 3) displayLED(F("REST"));
      else displayLED(F("BACH")); //"back" ;)
    break;
    
    case 15: //Save settings
      saveSlot = saveSlot + tmpdata;
      if (saveSlot < 0) saveSlot = saveSlotMax;
      if (saveSlot > saveSlotMax) saveSlot = 0;
      if (saveSlot > 9) tempString = "S-";
      else tempString = "S--";
      tempString += saveSlot;
      displayLED(tempString);
    break;
    
    case 16: //Load settings
      saveSlot = saveSlot + tmpdata;
      if (saveSlot < 0) saveSlot = saveSlotMax;
      if (saveSlot > saveSlotMax) saveSlot = 0;
      if (saveSlot > 9) tempString = "L-";
      else tempString = "L--";
      tempString += saveSlot;
      displayLED(tempString);
    break;
    
    case 17: //Adjust swing
      patch.swing = patch.swing + tmpdata;
      if (patch.swing < -99) patch.swing = -99;
      if (patch.swing > 99) patch.swing = 99;
      if (patch.swing < 0) tempString = "-";
      else tempString = "";
      tempString += abs(patch.swing);
      displayLED(tempString);
    break;
    
    case 18: // adjust tap1
      tap_1 = tap_1 + tmpdata;
      if (tap_1 < 1) tap_1 = 0;
      if (tap_1 > patch.channel[selectedChannel].lfsrLength) tap_1 = patch.channel[selectedChannel].lfsrLength;
      displayParam("T1", tap_1);
      patch.channel[selectedChannel].lfsrTap1 = tap_1;
    break; 
    
   case 19: // adjust tap2
      tap_2 = tap_2 + tmpdata;
      if (tap_2 < 1) tap_2 = 0;
      displayParam("T2", tap_2);
      patch.channel[selectedChannel].lfsrTap2 = tap_2;
     
    break;  
    
    case 20: // adjust LFSR length
      LFSR_length = LFSR_length + tmpdata;
      if (LFSR_length < 4) LFSR_length = 4;
      if (LFSR_length > 16) LFSR_length = 16;
      displayParam("Sr", LFSR_length);
      patch.channel[selectedChannel].lfsrLength = LFSR_length;
      if (LFSR_length < patch.channel[selectedChannel].lfsrTap1) { patch.channel[selectedChannel].lfsrTap1 = LFSR_length; } // length >= tap1
      if (LFSR_length < patch.channel[selectedChannel].lfsrTap2) { patch.channel[selectedChannel].lfsrTap2 = LFSR_length; } // length >= tap2
     
    break;  
    
     case 21:
       divisor = divisor + tmpdata;
       if ( divisor > 16) divisor = 16;
       if ( divisor < 1) divisor  =  1;
       displayParam("D ", divisor);
       patch.channel[selectedChannel].clockDivisor = divisor;
       if (divisor <= patch.channel[selectedChannel].divisionOffset) { patch.channel[selectedChannel].divisionOffset = divisor-1; } // keep offset in range
       
    break;  
    
     case 22:
       rand_n = rand_n + tmpdata;
       if ( rand_n > 9) rand_n = 9;
       if ( rand_n < 0) rand_n  =  0;
       displayParam("N ", rand_n);
       patch.channel[selectedChannel].randomThreshold = rand_n;
       
    break;  
    
     case 23:
       div_off = div_off + tmpdata;
       if ( div_off >= patch.channel[selectedChannel].clockDivisor) div_off = patch.channel[selectedChannel].clockDivisor-1;
       if ( div_off < 0) div_off  =  0;
       tempString = "off";
       tempString += div_off;
       displayLED(tempString);
       patch.channel[selectedChannel].divisionOffset = div_off;
       
    break;  
  }
}

void doButton() //handles button pushes for menu system
{ 
  String tempString = "";
        
  switch (encoderMode) {

  case 1: //Adjust Rate Main
      displayLED(F("out "));
      encoderMode = 2; //Go to Main Menu
      if (mainSelector == 1) { 
        displayLED(F("outA"));
      }
      else if (mainSelector == 2) { 
        displayLED(F("outB")); 
      }
      else if (mainSelector == 3) { 
        displayLED(F("outC")); 
      }
      else if (mainSelector == 4) { 
        displayLED(F("outD")); 
      }
      else if (mainSelector == 5) { 
        displayLED(F("outE")); 
      }
      else if (mainSelector == 6) {
        displayLED(F("outF")); 
      }
      else if (mainSelector == 7) displayLED(F("ShUF")); //swing ;)
      else if (mainSelector == 8) displayLED(F("RAtE"));
    break;

  case 2: //Output selector/Main Menu
      if (mainSelector < 7) {
        selectedChannel = mainSelector - 1;
        encoderMode = 3; 
        
        mode = patch.channel[selectedChannel].mode; 
        displayMode(mode);
      }

      else if (mainSelector == 7) {
        encoderMode = 17; //Adjust swing
        if (patch.swing < 0) tempString = "-";
        else tempString = "";
        tempString += abs(patch.swing);
        displayLED(tempString);
      }
      
      else if (mainSelector == 8) {
        encoderMode = 1; //Go back to main rate
      }
    break;

  case 3: //Set output mode
        patch.channel[selectedChannel].mode = mode;
        if (mode == 0) { //arrays
        encoderMode = 4; 
        arrayNumber = patch.channel[selectedChannel].clickTable;
        displayParam("AR", arrayNumber + 1);
      }
      else if (mode == 1) { //logic
        encoderMode = 8;
        logicMode = patch.channel[selectedChannel].logicMode; 
        displayLogicMode(logicMode);
      }
      
      else if (mode == 2) { //euclidean
        encoderMode = 12; 
        displayParam("b ", EuclidBeats);
      }
      
      else if (mode == 3) { //LFSR
        encoderMode = 20; 
        LFSR_length = patch.channel[selectedChannel].lfsrLength;
        displayParam("Sr", LFSR_length);
      } 
       
       else if (mode == 4) { //RANDOM
       // go threshold
       encoderMode = 22;
       rand_n = patch.channel[selectedChannel].randomThreshold;
       displayParam("N  ", rand_n);
       
      }
      
      else if (mode == 5) { //Division
       encoderMode = 21; 
       divisor = patch.channel[selectedChannel].clockDivisor;
       displayParam("D ", divisor);
      
      }

    break;

  case 4: //Set array number
      encoderMode = 5; //setting is done in doUpdates() to hear result right away
      arrayLength = patch.channel[selectedChannel].clickTableLength;
      displayParam("AL", arrayLength);
    break;  

  case 5: //Set array length
      encoderMode = 6;
      patternOffset = patch.channel[selectedChannel].offset;
      relativeOffset = patternOffset - 128;
      displayOffset("O", relativeOffset);
      patch.channel[selectedChannel].offset = patternOffset;
    break;

  case 6: //Set pattern offset
      encoderMode = 7;
      trigGate = patch.channel[selectedChannel].triggerLength;
      if (trigGate == 100) tempString = "GATE";
      else if (trigGate > 9) tempString = "tr";
      else tempString = "tr ";
      if (trigGate < 100) tempString += trigGate;
      displayLED(tempString);
    break;

  case 7: //Select trigger or gate
      encoderMode = 1; //go back to rate
      // new code (makrospex) 
      if (iBPM > 0) {
        // good :)
      }
      else {
        iBPM = (int)(15000/patch.rate);
      }
      if (clockInternal) patch.rate = (int)(((60.0/(float)iBPM) * 250.0) + 0.5);
      BPM = String(iBPM);
      tempString += BPM;
      displayLED(tempString);
    break;

  case 8: //select logic mode
      encoderMode = 9;
      logic1 = patch.channel[selectedChannel].logicOutput1;
      if (logic1 == 0) displayLED(F("L1-A"));
      else if (logic1 == 1) displayLED(F("L1-B"));
      else if (logic1 == 2) displayLED(F("L1-C"));
      else if (logic1 == 3) displayLED(F("L1-D"));
      else if (logic1 == 4) displayLED(F("L1-E"));
      else if (logic1 == 5) displayLED(F("L1-F"));
    break;
    
  case 9: //logic input 1
      encoderMode = 10;
      logic2 = patch.channel[selectedChannel].logicOutput2;
      if (logic2 == 0) displayLED(F("L2-A"));
      else if (logic2 == 1) displayLED(F("L2-B"));
      else if (logic2 == 2) displayLED(F("L2-C"));
      else if (logic2 == 3) displayLED(F("L2-D"));
      else if (logic2 == 4) displayLED(F("L2-E"));
      else if (logic2 == 5) displayLED(F("L2-F"));
    break;
      
  case 10: //logic input 2
      encoderMode = 11;
      patternOffset = patch.channel[selectedChannel].offset;
      relativeOffset = patternOffset - 128;
      displayOffset("O", relativeOffset);
      patch.channel[selectedChannel].offset = patternOffset;
    break;
      
  case 11: // + offset (delay)
      encoderMode = 7;
      trigGate = patch.channel[selectedChannel].triggerLength;
      if (trigGate == 100) tempString = "GATE";
      else if (trigGate > 9) tempString = "Tr";
      else tempString = "Tr ";
      if (trigGate < 100) tempString += trigGate;
      displayLED(tempString);
    break;
    
  case 12: //Euclidean #beats
      encoderMode = 13; //go to #groups
      EuclidGroups =patch.channel[selectedChannel].euclidianPulses;
      displayParam("G ", EuclidGroups);
    break;
    
  case 13: //Euclidean #groups
      encoderMode = 6; //go to adjust offset
      patternOffset = patch.channel[selectedChannel].offset;
      relativeOffset = patternOffset - 128;
      displayOffset("O", relativeOffset);
      patch.channel[selectedChannel].offset = patternOffset;
    break;
    
  case 14: //Save & Read parameters from EEPROM
      if (saveFunction == 1) { //go to Save settings
        encoderMode = 15;
        if (saveSlot > 9) tempString = "S-";
        else tempString = "S--";
        tempString += saveSlot;
        displayLED(tempString);
      }
      else if (saveFunction == 2) {
        encoderMode = 16; //go to Read settings
       if (saveSlot > 9) tempString = "L-";
       else tempString = "L--";
       tempString += saveSlot;
       displayLED(tempString);
      }
      else if (saveFunction == 3) {
        // RESET
        encoderMode = 1; //go back to rate
        initParams();
        synchronize();
        tempString = "";
        tempString += BPM;
        displayLED(tempString);
      } 
      else encoderMode = 1; //go back to rate
      tempString = BPM;
      displayLED(tempString); 
    break;
  
  case 15: //Save settings
      saveSettings();
      encoderMode = 1; //go back to rate
      tempString += BPM;
      displayLED(tempString);
    break;
    
  case 16: //Load settings
      loadSettings();
      encoderMode = 1; //go back to rate

      iBPM = (int)((15000.0/(float)patch.rate) + 0.5);

      if (clockInternal) patch.rate = (int)(((60.0/(float)iBPM) * 250.0) + 0.5); //quarter note(s)
      BPM = String(iBPM);
      tempString += BPM;
      displayLED(tempString);
    break;
    
  case 17: //Adjust swing
      encoderMode = 1; //go back to rate
      if (iBPM > 0) {
        // good :)
      }
      else {
        iBPM = (int)(15000/patch.rate);
      }
      if (clockInternal) patch.rate = (int)(((60.0/(float)iBPM) * 250.0) + 0.5); //quarter note(s)
      BPM = String(iBPM);
      tempString += BPM;
      displayLED(tempString);
    break;
    
  case 18: // adjust tap1
      encoderMode = 19; //go to tap2
      tap_2 = patch.channel[selectedChannel].lfsrTap2;
      displayParam("T2", tap_2);
     
    break; 
    
   case 19: // adjust tap2
      encoderMode = 7; //go to trigger
      trigGate = patch.channel[selectedChannel].triggerLength;
      if (trigGate == 100) tempString = "GATE";
      else if (trigGate > 9) tempString = "Tr";
      else tempString = "Tr ";
      if (trigGate < 100) tempString += trigGate;
      displayLED(tempString);
     
     
    break;  
    
     case 20: // adjust LFSR length
     encoderMode = 18; //go to tap1
     tap_1 = patch.channel[selectedChannel].lfsrTap1;
     displayParam("T1", tap_1);
     
    break;  
    
     case 21: // adjust divisor
      encoderMode = 23; //go to div offset
      div_off = patch.channel[selectedChannel].divisionOffset;
      tempString = "off";
      tempString += div_off;
      displayLED(tempString);
     
    break;  
    
    case 22: // adjust threshold
     encoderMode = 7; //go to trigger
     trigGate = patch.channel[selectedChannel].triggerLength;
     if (trigGate == 100) tempString = "GATE";
     else if (trigGate > 9) tempString = "tr";
     else tempString = "tr ";
     if (trigGate < 100) tempString += trigGate;
     displayLED(tempString);
     
    break;  
    
    case 23:
     encoderMode = 7; //go to trigger
     trigGate = patch.channel[selectedChannel].triggerLength;
     if (trigGate == 100) tempString = "GATE";
     else if (trigGate > 9) tempString = "tr";
     else tempString = "tr ";
     if (trigGate < 100) tempString += trigGate;
     displayLED(tempString);
   
    break;   
    
  default:
      encoderMode = 1;
      if (iBPM > 0) {
        // good :)
      }
      else {
        iBPM = (int)(15000/patch.rate);
      }

      if (clockInternal) patch.rate = (int)(((60.0/(float)iBPM) * 250.0) + 0.5); //quarter note(s)
      BPM = String(iBPM);
  }
}


