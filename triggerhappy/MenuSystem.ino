
int mainSelector = 1; //top level
int encoderMode = 1; //menu location pointer
int mode = 0; //variable for output mode
int arrayNumber = 0; //variable for array number assignment
String tempString = ""; //variable to build display text
int arrayLength = 32; //variable to set array length
int patternOffset = 128; //variable to set pattern offset
boolean trigGate = true; //variable for trigger or gate selection
int logicMode = 0; //variable for logic mode selection
int logic1 = 0; //first logic input
int logic2 = 0; //second logic input
int EuclidBeats = 8; //Number of beats for Euclidean pattern
int EuclidGroups = 2; //Number of groups within Euclidean pattern
int saveFunction = 1; //Save or Load files

void doUpdates() //handles encoder input and display for menu system
{ 
  switch (encoderMode) {
    
    case 1: //Adjust Rate Main
      rateMain = rateMain - (tmpdata * rateMain/30); //make amount of change proportional to rate
      BPM = String(15000/rateMain);
      displayLED(BPM);
    break;
    
    case 2: //Output selector/Main Menu
      mainSelector = mainSelector + tmpdata;
      if (mainSelector > 8) mainSelector = 1;
      if (mainSelector < 1) mainSelector = 8;
      if (mainSelector == 1) displayLED("outA");
      else if (mainSelector == 2) displayLED("outB");
      else if (mainSelector == 3) displayLED("outC");
      else if (mainSelector == 4) displayLED("outD");
      else if (mainSelector == 5) displayLED("outE");
      else if (mainSelector == 6) displayLED("outF");
      else if (mainSelector == 7) displayLED("S3nG"); //swing ;)
      else if (mainSelector == 8) displayLED("RATE");
    break;
    
    case 3: //Select output mode
      mode = mode + tmpdata;
      if (mode > 2) mode = 0;
      if (mode < 0) mode = 2;
      if (mode == 0) displayLED("ARR ");
      else if (mode == 1) displayLED("LoG ");
      else if (mode == 2) displayLED("EuC ");
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
      if (patternOffset < (128 - offsetRange)) patternOffset = (128 - offsetRange);
      if (patternOffset > (128 + offsetRange)) patternOffset = (128 + offsetRange);
      tempString = String(patternOffset - 128);
      displayLED(tempString);
      outParam[paramA][1] = patternOffset;
    break;
    
    case 7: //Select trigger or gate behavior
      trigGate = !trigGate;
      if (trigGate) displayLED("GATE");
      else displayLED("TRIG");
      outParam[paramA][2] = trigGate;
    break;
    
    case 8: //select logic mode
      logicMode = logicMode + tmpdata;
      if (logicMode < 0) logicMode = 6;
      if (logicMode > 6) logicMode = 0;
      if (logicMode == 0) displayLED("NoT ");
      else if (logicMode == 1) displayLED("AND ");
      else if (logicMode == 2) displayLED(" oR ");
      else if (logicMode == 3) displayLED("NAND");
      else if (logicMode == 4) displayLED("nor ");
      else if (logicMode == 5) displayLED("Hor ");
      else if (logicMode == 6) displayLED("Hnor");
      outParam[paramA][5] = logicMode;
    break;
    
    case 9: //select logic input 1
      logic1 = logic1 + tmpdata;
      if (logic1 < 0) logic1 = 5;
      if (logic1 > 5) logic1 = 0;
      if (logic1 == 0) displayLED("L1-A");
      else if (logic1 == 1) displayLED("L1-B");
      else if (logic1 == 2) displayLED("L1-C");
      else if (logic1 == 3) displayLED("L1-D");
      else if (logic1 == 4) displayLED("L1-E");
      else if (logic1 == 5) displayLED("L1-F");
      outParam[paramA][6] = logic1;
    break;
    
    case 10: //select logic input 2
      logic2 = logic2 + tmpdata;
      if (logic2 < 0) logic2 = 5;
      if (logic2 > 5) logic2 = 0;
      if (logic2 == 0) displayLED("L2-A");
      else if (logic2 == 1) displayLED("L2-B");
      else if (logic2 == 2) displayLED("L2-C");
      else if (logic2 == 3) displayLED("L2-D");
      else if (logic2 == 4) displayLED("L2-E");
      else if (logic2 == 5) displayLED("L2-F");
      outParam[paramA][7] = logic2;
    break;
    
    case 11: // + offset (delay)
      patternOffset = patternOffset + tmpdata;
      if (patternOffset < 128) patternOffset = 128;
      if (patternOffset > (128 + offsetRange)) patternOffset = (128 + offsetRange);
      tempString = String(patternOffset - 128) ;
      displayLED(tempString);
      outParam[paramA][1] = patternOffset;
    break;
    
    case 12: //Euclidean #beats
      EuclidBeats = EuclidBeats + tmpdata;
      if (EuclidBeats < 1) EuclidBeats = 1;
      if (EuclidBeats > 250) EuclidBeats = 250;
      if (EuclidBeats > 99) tempString = "b";
      else if (EuclidBeats > 9) tempString = "b ";
      else tempString = "b  "; 
      tempString += EuclidBeats;
      displayLED(tempString);
      outParam[paramA][8] = EuclidBeats;
    break;
    
    case 13: //Euclidean #groups
      EuclidGroups = EuclidGroups + tmpdata;
      if (EuclidGroups < 1) EuclidGroups = 1;
      if (EuclidGroups > EuclidBeats) EuclidGroups = EuclidBeats;
      if (EuclidGroups > 99) tempString = "G";
      else if (EuclidGroups > 9) tempString = "G ";
      else tempString = "G  "; 
      tempString += EuclidGroups;
      displayLED(tempString);
      outParam[paramA][9] = EuclidGroups;
    break;
    
    case 14: //Save & Read parameters from EEPROM
      saveFunction = saveFunction + tmpdata;
      if (saveFunction < 1) saveFunction = 3;
      if (saveFunction > 3) saveFunction = 1;
      if (saveFunction == 1) displayLED("SAUE");
      else if (saveFunction == 2) displayLED("LOAD");
      else displayLED("BACH"); //"back" ;)
    break;
    
    case 15: //Save settings
      saveSlot = saveSlot + tmpdata;
      if (saveSlot < 0) saveSlot = 0;
      if (saveSlot > 99) saveSlot = 99;
      if (saveSlot > 9) tempString = "S ";
      else tempString = "S  ";
      tempString += saveSlot;
      displayLED(tempString);
    break;
    
    case 16: //Load settings
      saveSlot = saveSlot + tmpdata;
      if (saveSlot < 0) saveSlot = 0;
      if (saveSlot > 99) saveSlot = 99;
      if (saveSlot > 9) tempString = "L ";
      else tempString = "L  ";
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
  }
}

void doButton() //handles button pushes for menu system
{ 
  switch (encoderMode) {

  case 1: //Adjust Rate Main
      displayLED("out ");
      encoderMode = 2; //Go to Main Menu
      if (mainSelector == 1) displayLED("outA");
      else if (mainSelector == 2) displayLED("outB");
      else if (mainSelector == 3) displayLED("outC");
      else if (mainSelector == 4) displayLED("outD");
      else if (mainSelector == 5) displayLED("outE");
      else if (mainSelector == 6) displayLED("outF");
      else if (mainSelector == 7) displayLED("S3nG"); //swing ;)
      else if (mainSelector == 8) displayLED("RATE");
    break;

  case 2: //Output selector/Main Menu
      if (mainSelector < 7) {
        paramA = mainSelector - 1;
        encoderMode = 3; 
        
        mode = outParam[paramA][0]; 
        if (mode == 0) displayLED("ARR ");
        else if (mode == 1) displayLED("LoG ");
        else if (mode == 2) displayLED("EuC ");
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
        if (logicMode == 0) displayLED("NoT ");
        else if (logicMode == 1) displayLED("AND ");
        else if (logicMode == 2) displayLED(" oR ");
        else if (logicMode == 3) displayLED("NAND");
        else if (logicMode == 4) displayLED("nor ");
        else if (logicMode == 5) displayLED("Hor ");
        else if (logicMode == 6) displayLED("Hnor");
      }
      
      else if (mode == 2) { //euclidean
        encoderMode = 12; 
        if (EuclidBeats > 99) tempString = "b";
        else if (EuclidBeats > 9) tempString = "b ";
        else tempString = "b  "; 
        tempString += EuclidBeats;
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
      tempString = String(patternOffset - 128);
      displayLED(tempString);
    break;

  case 6: //Set pattern offset
      encoderMode = 7;
      trigGate = outParam[paramA][2];
      if (trigGate) displayLED("GATE");
      else displayLED("TRIG");
    break;

  case 7: //Select trigger or gate
      encoderMode = 1; //go back to rate
      BPM = String(15000/rateMain);
      displayLED(BPM);
    break;

  case 8: //select logic mode
      encoderMode = 9;
      logic1 = outParam[paramA][6];
      if (logic1 == 0) displayLED("L1-A");
      else if (logic1 == 1) displayLED("L1-B");
      else if (logic1 == 2) displayLED("L1-C");
      else if (logic1 == 3) displayLED("L1-D");
      else if (logic1 == 4) displayLED("L1-E");
      else if (logic1 == 5) displayLED("L1-F");
    break;
    
  case 9: //logic input 1
      encoderMode = 10;
      logic2 = outParam[paramA][7];
      if (logic2 == 0) displayLED("L2-A");
      else if (logic2 == 1) displayLED("L2-B");
      else if (logic2 == 2) displayLED("L2-C");
      else if (logic2 == 3) displayLED("L2-D");
      else if (logic2 == 4) displayLED("L2-E");
      else if (logic2 == 5) displayLED("L2-F");
    break;
      
  case 10: //logic input 2
      encoderMode = 11;
      patternOffset = outParam[paramA][1];
      tempString = String(patternOffset - 128) ;
      displayLED(tempString);
    break;
      
  case 11: // + offset (delay)
      encoderMode = 7;
      trigGate = outParam[paramA][2];
      if (trigGate) displayLED("GATE");
      else displayLED("TRIG");
    break;
    
  case 12: //Euclidean #beats
      encoderMode = 13; //go to #groups
      EuclidGroups = outParam[paramA][9];
      if (EuclidGroups > 99) tempString = "G";
      else if (EuclidGroups > 9) tempString = "G ";
      else tempString = "G  "; 
      tempString += EuclidGroups;
      displayLED(tempString);
    break;
    
  case 13: //Euclidean #groups
      encoderMode = 6; //go to adjust offset
      patternOffset = outParam[paramA][1];
      tempString = String(patternOffset - 128);
      displayLED(tempString);
    break;
    
  case 14: //Save & Read parameters from EEPROM
      if (saveFunction == 1) { //go to Save settings
        encoderMode = 15;
        if (saveSlot > 9) tempString = "S ";
        else tempString = "S  ";
        tempString += saveSlot;
        displayLED(tempString);
      }
      
      else if (saveFunction == 2) {
        encoderMode = 16; //go to Read settings
       if (saveSlot > 9) tempString = "L ";
       else tempString = "L  ";
       tempString += saveSlot;
       displayLED(tempString);
      }
      
      else encoderMode = 1; //go back to rate
      //BPM = String(15000/rateMain);
      //displayLED(BPM); 
    break;
  
  case 15: //Save settings
      saveSettings();
      encoderMode = 1; //go back to rate
      BPM = String(15000/rateMain);
      displayLED(BPM); 
    break;
    
  case 16: //Load settings
      loadSettings();
      encoderMode = 1; //go back to rate
      BPM = String(15000/rateMain);
      displayLED(BPM); 
    break;
    
  case 17: //Adjust swing
      encoderMode = 1; //go back to rate
      BPM = String(15000/rateMain);
      displayLED(BPM);
    break;
    
  default:
      encoderMode = 1;
  }
}
