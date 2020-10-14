

char charmessage[4];
int intmessage[4];

void setupDisplay()  {
  mySerial.begin(9600); // initialize communication to the display
  mySerial.write(0x7A); // command byte for brightness
  mySerial.write(0x01); // display brightness (lower = brighter)
}

void displayLED(String ledMessage)
{
  int strLength = ledMessage.length(); 
  if (strLength <= 4) {
    if (strLength < 4) mySerial.write(" ");
    if (strLength < 3) mySerial.write(" ");
    if (strLength < 2) mySerial.write(" ");
    for(int i = 0; i < strLength; i++){
      mySerial.write(ledMessage[i]);   
    }
  }
}

