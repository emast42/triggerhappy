int mainSelector = 1; //top level (was int)
int encoderMode = 1; //menu location pointer (was int)
int mode = 1; //variable for output mode (was int)
int arrayNumber = 0; //variable for array number assignment (was int)
String tempString = ""; //variable to build display text
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

void doUpdates() //handles encoder input and display for menu system
{ 
  
  switch (encoderMode) {
    
    case 1: //Adjust Rate Main
      // new code (makrospex)
      if (clockInternal) {
        if (iBPM > 0) {
          iBPM = iBPM + tmpdata;
        }
        else {
          if (rateMain > 0) {
            iBPM = (int)((15000/rateMain) + tmpdata);
          }
          /*
          else {
            rateMain = (int)(((60.0/(float)iBPM) * 250.0) + 0.5); //quarter note(s)
          }
          */  
        }
        rateMain = (int)(((60.0/(float)iBPM) * 250.0) + 0.5); //quarter note(s)
        BPM = String(iBPM);
        // end new code
        // old code:
        /*
        // rateMain = rateMain - (tmpdata * rateMain/30); //make amount of change proportional to rate
        rateMain = rateMain - (tmpdata * rateMain/60);
        BPM = String(15000/rateMain);
        */
        // end old code
        tempString = "";
        tempString += BPM;
        displayLED(tempString);
        // possible startup bug(?):
        // lastBPMupdate=nowTime;
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
      if (mode == 0) displayLED(F("ARR "));
      else if (mode == 1) displayLED(F("LoG "));
      else if (mode == 2) displayLED(F("EuC "));
      else if (mode == 3) displayLED(F("LFSR"));
      else if (mode == 4) displayLED(F("RAnd"));
      else if (mode == 5) displayLED(F("d--n"));
    break;
    
    case 4: //Select array number
      arrayNumber = arrayNumber + tmpdata;
      if (arrayNumber < 0) arrayNumber = 15;
      if (arrayNumber > 15) arrayNumber = 0;
      if (arrayNumber > 8) tempString = "AR";
      else tempString = "AR ";
      tempString += (arrayNumber + 1);
      displayLED(tempString);
      outParam[paramA][3] = arrayNumber;
    break;
    
    case 5: //Select array length
      arrayLength = arrayLength + tmpdata;
      if (arrayLength < 1) arrayLength = 1;
      if (arrayLength > 16) arrayLength = 16;
      if (arrayLength > 9) tempString = "AL";
      else tempString = "AL ";
      tempString += arrayLength;
      displayLED(tempString);
      outParam[paramA][4] = arrayLength;
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
      // tempString += patternOffset - 128;
      tempString.concat(relativeOffset);
      displayLED(tempString);
      outParam[paramA][1] = patternOffset;
    break;
    
    case 7: // adjust trigger length
      trigGate = trigGate + tmpdata;
      if (trigGate > 100) trigGate = 100;
      else if (trigGate < 5) trigGate = 5;
      if (trigGate > 9) tempString = "tr";
      else tempString = "tr ";
      if (trigGate == 100) tempString="GAtE";
      else tempString += trigGate;
      displayLED(tempString);
      outParam[paramA][2] = trigGate;
    break;
    
    case 8: //select logic mode
      logicMode = logicMode + tmpdata;
      if (logicMode < 0) logicMode = 6;
      if (logicMode > 6) logicMode = 0;
      if (logicMode == 0) displayLED(F("Not "));
      else if (logicMode == 1) displayLED(F("AND "));
      else if (logicMode == 2) displayLED(F(" oR "));
      else if (logicMode == 3) displayLED(F("NAND"));
      else if (logicMode == 4) displayLED(F("nor "));
      else if (logicMode == 5) displayLED(F("Hor "));
      else if (logicMode == 6) displayLED(F("Hnor"));
      outParam[paramA][5] = logicMode;
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
      outParam[paramA][6] = logic1;
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
      outParam[paramA][7] = logic2;
    break;
    
    case 11: // + offset (delay)
      patternOffset = patternOffset + tmpdata;
      relativeOffset = patternOffset - 128;
      if (patternOffset < (128 - 16)) patternOffset = (128 - 16);
      if (patternOffset > (128 + 16)) patternOffset = (128 + 16);
      if (relativeOffset > 9) tempString = "O ";
      else if (relativeOffset < -9) tempString ="O";
      else if (relativeOffset < 0) tempString ="O ";
      else tempString = "O  ";
      tempString.concat(relativeOffset);
      // tempString += patternOffset - 128;
      displayLED(tempString);
      outParam[paramA][1] = patternOffset;
    break;
    
    case 12: //Euclidean #beats
      EuclidBeats = EuclidBeats + tmpdata;
      if (EuclidBeats < 1) EuclidBeats = 1;
      if (EuclidBeats > 32) EuclidBeats = 32;
      if (EuclidBeats > 9) tempString = "b ";
      else tempString = "b  "; 
      tempString += EuclidBeats;
      displayLED(tempString);
      outParam[paramA][8] = EuclidBeats;
    break;
    
    case 13: //Euclidean #groups
      EuclidGroups = EuclidGroups + tmpdata;
      if (EuclidGroups < 1) EuclidGroups = 1;
      if (EuclidGroups > EuclidBeats) EuclidGroups = EuclidBeats;
      if (EuclidGroups > 9) tempString = "G ";
      else tempString = "G  "; 
      tempString += EuclidGroups;
      displayLED(tempString);
      outParam[paramA][9] = EuclidGroups;
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
      if (saveSlot < 0) saveSlot = 0;
      if (saveSlot > 99) saveSlot = 99;
      if (saveSlot > 9) tempString = "S-";
      else tempString = "S--";
      tempString += saveSlot;
      displayLED(tempString);
    break;
    
    case 16: //Load settings
      saveSlot = saveSlot + tmpdata;
      if (saveSlot < 0) saveSlot = 0;
      if (saveSlot > 99) saveSlot = 99;
      if (saveSlot > 9) tempString = "L-";
      else tempString = "L--";
      tempString += saveSlot;
      displayLED(tempString);
    break;
    
    case 17: //Adjust swing
      swing = swing + tmpdata;
      if (swing < -99) swing = -99;
      if (swing > 99) swing = 99;
      if (swing < 0) tempString = "-";
      else tempString = "";
      tempString += abs(swing);
      displayLED(tempString);
    break;
    
    case 18: // adjust tap1
      tap_1 = tap_1 + tmpdata;
      if (tap_1 < 1) tap_1 = 0;
      if (tap_1 > outParam[paramA][12]) tap_1 = outParam[paramA][12];
      if (tap_1 > 9) tempString = "T1";
      else tempString = "T1 ";
      tempString += tap_1;
      displayLED(tempString);
      outParam[paramA][10] = tap_1;
    break; 
    
   case 19: // adjust tap2
      tap_2 = tap_2 + tmpdata;
      if (tap_2 < 1) tap_2 = 0;
      if (tap_2 > outParam[paramA][12]) tap_2 = outParam[paramA][12];
      if (tap_2 > 9) tempString = "T2";
      else tempString = "T2 ";
      tempString += tap_2;
      displayLED(tempString);
      outParam[paramA][11] = tap_2;
     
    break;  
    
    case 20: // adjust LFSR length
      LFSR_length = LFSR_length + tmpdata;
      if (LFSR_length < 4) LFSR_length = 4;
      if (LFSR_length > 16) LFSR_length = 16;
      if (LFSR_length > 9) tempString = "Sr";
      else tempString = "Sr ";
      tempString += LFSR_length;
      displayLED(tempString);
      outParam[paramA][12] = LFSR_length;
      if (LFSR_length < outParam[paramA][10]) { outParam[paramA][10] = LFSR_length; } // length >= tap1
      if (LFSR_length < outParam[paramA][11]) { outParam[paramA][11] = LFSR_length; } // length >= tap2
     
    break;  
    
     case 21:
       divisor = divisor + tmpdata;
       if ( divisor > 16) divisor = 16;
       if ( divisor < 1) divisor  =  1;
       if ( divisor > 9) tempString = "D ";
       else tempString = "D  ";
       tempString += divisor;
       displayLED(tempString);
       outParam[paramA][13] = divisor;
       if (divisor <= outParam[paramA][15]) { outParam[paramA][15] = divisor-1; } // keep offset in range
       
    break;  
    
     case 22:
       rand_n = rand_n + tmpdata;
       if ( rand_n > 9) rand_n = 9;
       if ( rand_n < 0) rand_n  =  0;
       tempString = "N  ";
       tempString += rand_n;
       displayLED(tempString);
       outParam[paramA][14] = rand_n;
       
    break;  
    
     case 23:
       div_off = div_off + tmpdata;
       if ( div_off >= outParam[paramA][13]) div_off = outParam[paramA][13]-1;
       if ( div_off < 0) div_off  =  0;
       tempString = "off";
       tempString += div_off;
       displayLED(tempString);
       outParam[paramA][15] = div_off;
       
    break;  
  }
}

void doButton() //handles button pushes for menu system
{ 
  switch (encoderMode) {

  case 1: //Adjust Rate Main
      displayLED(F("out "));
      encoderMode = 2; //Go to Main Menu
      if (mainSelector == 1) { 
        displayLED(F("outA"));
        activeChannel = 0;
      }
      else if (mainSelector == 2) { 
        displayLED(F("outB")); 
        activeChannel = 1;
      }
      else if (mainSelector == 3) { 
        displayLED(F("outC")); 
        activeChannel = 2;
      }
      else if (mainSelector == 4) { 
        displayLED(F("outD")); 
        activeChannel = 3;
      }
      else if (mainSelector == 5) { 
        displayLED(F("outE")); 
        activeChannel = 4;
      }
      else if (mainSelector == 6) {
        displayLED(F("outF")); 
        activeChannel = 5;
      }
      else if (mainSelector == 7) displayLED(F("ShUF")); //swing ;)
      else if (mainSelector == 8) displayLED(F("RAtE"));
    break;

  case 2: //Output selector/Main Menu
      if (mainSelector < 7) {
        paramA = mainSelector - 1;
        encoderMode = 3; 
        
        mode = outParam[paramA][0]; 
        if (mode == 0) displayLED(F("ARR "));
        else if (mode == 1) displayLED(F("LoG "));
        else if (mode == 2) displayLED(F("EuC "));
        else if (mode == 3) displayLED(F("LFSR"));
        else if (mode == 4) displayLED(F("RAnd"));
        else if (mode == 5) displayLED(F("d--n"));
      }

      else if (mainSelector == 7) {
        encoderMode = 17; //Adjust swing
        if (swing < 0) tempString = "-";
        else tempString = "";
        tempString += abs(swing);
        displayLED(tempString);
      }
      
      else if (mainSelector == 8) {
        encoderMode = 1; //Go back to main rate
      }
    break;

  case 3: //Set output mode
        outParam[paramA][0] = mode;
        if (mode == 0) { //arrays
        encoderMode = 4; 
        arrayNumber = outParam[paramA][3];
        if (arrayNumber > 8) tempString = "AR";
        else tempString = "AR ";
        tempString += (arrayNumber + 1);
        displayLED(tempString);
      }
      else if (mode == 1) { //logic
        encoderMode = 8;
        logicMode = outParam[paramA][5]; 
        if (logicMode == 0) displayLED(F("NoT "));
        else if (logicMode == 1) displayLED(F("AND "));
        else if (logicMode == 2) displayLED(F(" oR "));
        else if (logicMode == 3) displayLED(F("NAND"));
        else if (logicMode == 4) displayLED(F("nor "));
        else if (logicMode == 5) displayLED(F("Hor "));
        else if (logicMode == 6) displayLED(F("Hnor"));
      }
      
      else if (mode == 2) { //euclidean
        encoderMode = 12; 
        if (EuclidBeats > 9) tempString = "b ";
        else tempString = "b  "; 
        tempString += EuclidBeats;
        displayLED(tempString);
      }
      
      else if (mode == 3) { //LFSR
       encoderMode = 20; 
       LFSR_length = outParam[paramA][12];
       if ( LFSR_length > 9) tempString = "Sr";
       else tempString = "Sr ";
       tempString += LFSR_length;
       displayLED(tempString);
      } 
       
       else if (mode == 4) { //RANDOM
       // go threshold
       encoderMode = 22;
       rand_n = outParam[paramA][14];
       tempString = "N  ";
       tempString += rand_n;
       displayLED(tempString);
       
      }
      
      else if (mode == 5) { //Division
       encoderMode = 21; 
       divisor = outParam[paramA][13];
       if ( divisor > 9) tempString = "D ";
       else tempString = "D  ";
       tempString += divisor;
       displayLED(tempString);
      
      }

    break;

  case 4: //Set array number
      encoderMode = 5; //setting is done in doUpdates() to hear result right away
      arrayLength = outParam[paramA][4];
      if (arrayLength > 9) tempString = "AL";
      else tempString = "AL ";
      tempString += arrayLength;
      displayLED(tempString);
    break;  

  case 5: //Set array length
      encoderMode = 6;
      patternOffset = outParam[paramA][1];
      relativeOffset = patternOffset - 128;
      if (relativeOffset > 9) tempString = "O ";
      else if (relativeOffset < -9) tempString ="O";
      else if (relativeOffset < 0) tempString ="O ";
      else tempString = "O  ";
      // tempString += patternOffset - 128;
      tempString.concat(relativeOffset);
      displayLED(tempString);
      /*
      if (patternOffset - 128 > 9) tempString = "O ";
      else tempString = "O  ";
      tempString += patternOffset - 128;
      displayLED(tempString);
      */
      outParam[paramA][1] = patternOffset;
    break;

  case 6: //Set pattern offset
      encoderMode = 7;
      trigGate = outParam[paramA][2];
     // if (trigGate) displayLED("GATE");
     // else displayLED("TRIG");
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
        iBPM = (int)(15000/rateMain);
        // BPM = String(15000/rateMain);
      }
      // iBPM = BPM.toInt();
      if (clockInternal) rateMain = (int)(((60.0/(float)iBPM) * 250.0) + 0.5);
      BPM = String(iBPM);
      // old code: 
      // BPM = String(15000/rateMain);
      // end old code
      tempString = "";
      tempString += BPM;
      displayLED(tempString);
    break;

  case 8: //select logic mode
      encoderMode = 9;
      logic1 = outParam[paramA][6];
      if (logic1 == 0) displayLED(F("L1-A"));
      else if (logic1 == 1) displayLED(F("L1-B"));
      else if (logic1 == 2) displayLED(F("L1-C"));
      else if (logic1 == 3) displayLED(F("L1-D"));
      else if (logic1 == 4) displayLED(F("L1-E"));
      else if (logic1 == 5) displayLED(F("L1-F"));
    break;
    
  case 9: //logic input 1
      encoderMode = 10;
      logic2 = outParam[paramA][7];
      if (logic2 == 0) displayLED(F("L2-A"));
      else if (logic2 == 1) displayLED(F("L2-B"));
      else if (logic2 == 2) displayLED(F("L2-C"));
      else if (logic2 == 3) displayLED(F("L2-D"));
      else if (logic2 == 4) displayLED(F("L2-E"));
      else if (logic2 == 5) displayLED(F("L2-F"));
    break;
      
  case 10: //logic input 2
      encoderMode = 11;
      patternOffset = outParam[paramA][1];
      relativeOffset = patternOffset - 128;
      if (relativeOffset > 9) tempString = "O ";
      else if (relativeOffset < -9) tempString ="O";
      else if (relativeOffset < 0) tempString ="O ";
      else tempString = "O  ";
      // tempString += patternOffset - 128;
      tempString.concat(relativeOffset);
      displayLED(tempString);
      outParam[paramA][1] = patternOffset;
      /*
      if (patternOffset - 128 > 9) tempString = "O ";
      else tempString = "O  ";
      tempString += patternOffset - 128;
      displayLED(tempString);
      */
    break;
      
  case 11: // + offset (delay)
      encoderMode = 7;
      trigGate = outParam[paramA][2];
      if (trigGate == 100) tempString = "GATE";
      else if (trigGate > 9) tempString = "Tr";
      else tempString = "Tr ";
      if (trigGate < 100) tempString += trigGate;
      displayLED(tempString);
     // displayLED(String(trigGate));
    break;
    
  case 12: //Euclidean #beats
      encoderMode = 13; //go to #groups
      EuclidGroups = outParam[paramA][9];
      if (EuclidGroups > 9) tempString = "G ";
      else tempString = "G  "; 
      tempString += EuclidGroups;
      displayLED(tempString);
    break;
    
  case 13: //Euclidean #groups
      encoderMode = 6; //go to adjust offset
      patternOffset = outParam[paramA][1];
      relativeOffset = patternOffset - 128;
      if (relativeOffset > 9) tempString = "O ";
      else if (relativeOffset < -9) tempString ="O";
      else if (relativeOffset < 0) tempString ="O ";
      else tempString = "O  ";
      // tempString += patternOffset - 128;
      tempString.concat(relativeOffset);
      displayLED(tempString);
      /*
      if (patternOffset - 128 > 9) tempString = "O ";
      else tempString = "O  ";
      tempString += patternOffset - 128;
      displayLED(tempString);
      */
      outParam[paramA][1] = patternOffset;
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
      // old code:
      // BPM = String(15000/rateMain);
      // end old code;
      tempString = BPM;
      displayLED(tempString); 
    break;
  
  case 15: //Save settings
      saveSettings();
      encoderMode = 1; //go back to rate
      // old code:
      // BPM = String(15000/rateMain);
      // end old code;
      tempString = "";
      tempString += BPM;
      displayLED(tempString);
    break;
    
  case 16: //Load settings
      loadSettings();
      encoderMode = 1; //go back to rate
      // new code (makrospex) 
      iBPM = (int)((15000.0/(float)rateMain) + 0.5);
      // BPM = String(15000/rateMain);
      if (clockInternal) rateMain = (int)(((60.0/(float)iBPM) * 250.0) + 0.5); //quarter note(s)
      BPM = String(iBPM);
      // old code:
      // BPM = String(15000/rateMain);
      // end old code;
      tempString = "";
      tempString += BPM;
      displayLED(tempString);
    break;
    
  case 17: //Adjust swing
      encoderMode = 1; //go back to rate
      // new code (makrospex) 
      if (iBPM > 0) {
        // good :)
      }
      else {
        iBPM = (int)(15000/rateMain);
      }
      if (clockInternal) rateMain = (int)(((60.0/(float)iBPM) * 250.0) + 0.5); //quarter note(s)
      BPM = String(iBPM);
      tempString = "";
      tempString += BPM;
      displayLED(tempString);
    break;
    
  case 18: // adjust tap1
      encoderMode = 19; //go to tap2
      tap_2 = outParam[paramA][11];
      if (tap_2 > 9) tempString = "T2";
      else tempString = "T2 ";
      tempString += tap_2;
      displayLED(tempString);
     
    break; 
    
   case 19: // adjust tap2
      encoderMode = 7; //go to trigger
      trigGate = outParam[paramA][2];
      if (trigGate == 100) tempString = "GATE";
      else if (trigGate > 9) tempString = "Tr";
      else tempString = "Tr ";
      if (trigGate < 100) tempString += trigGate;
      displayLED(tempString);
     
     
    break;  
    
     case 20: // adjust LFSR length
     encoderMode = 18; //go to tap1
     tap_1 = outParam[paramA][10];
     if (tap_1 > 9) tempString = "T1";
     else tempString = "T1 ";
     tempString += tap_1;
     displayLED(tempString);
     
    break;  
    
     case 21: // adjust divisor
      encoderMode = 23; //go to div offset
      div_off = outParam[paramA][15];
      tempString = "off";
      tempString += div_off;
      displayLED(tempString);
     
    break;  
    
    case 22: // adjust threshold
     encoderMode = 7; //go to trigger
     trigGate = outParam[paramA][2];
     if (trigGate == 100) tempString = "GATE";
     else if (trigGate > 9) tempString = "tr";
     else tempString = "tr ";
     if (trigGate < 100) tempString += trigGate;
     displayLED(tempString);
     
    break;  
    
    case 23:
     encoderMode = 7; //go to trigger
     trigGate = outParam[paramA][2];
     if (trigGate == 100) tempString = "GATE";
     else if (trigGate > 9) tempString = "tr";
     else tempString = "tr ";
     if (trigGate < 100) tempString += trigGate;
     displayLED(tempString);
   
    break;   
    
  default:
      encoderMode = 1;
      tempString = "";
      // new code (makrospex) 
      if (iBPM > 0) {
        // good :)
      }
      else {
        iBPM = (int)(15000/rateMain);
        // BPM = String(15000/rateMain);
      }
      // iBPM = BPM.toInt();
      if (clockInternal) rateMain = (int)(((60.0/(float)iBPM) * 250.0) + 0.5); //quarter note(s)
      BPM = String(iBPM);
      // old code:
      // end old code;     
  }
}

