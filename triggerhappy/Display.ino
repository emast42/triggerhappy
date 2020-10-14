void setupDisplay()  {
  mySerial.begin(9600); // initialize communication to the display
  delay(500);
  //mySerial.write(0x7A); // command byte for brightness
  //mySerial.write(0x01); // display brightness (lower = brighter)
  
}

void displayLED(String ledMessage)
{
  uint8_t strLength = ledMessage.length(); 
  if (strLength <= 4) {
    if (strLength == 1) mySerial.write("   ");
    else if (strLength == 2) mySerial.write("  ");
    else if (strLength == 3) mySerial.write(" ");
    else {
      // no padding needed here
    }
    for(int i = 0; i < strLength; i++){
      mySerial.write(ledMessage[i]);   
    }
  }
}



