#define TM1637

#ifdef TM1637
#include <TM1637Display.h>

#define TM1637CLK 4
#define TM1637DIO 5

TM1637Display display(TM1637CLK, TM1637DIO);

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D

const uint8_t SEG_CHAR[] = {
  SEG_A|SEG_B|SEG_C|SEG_E|SEG_F|SEG_G, //A
  SEG_C|SEG_D|SEG_E|SEG_F|SEG_G, //B
  SEG_A|SEG_D|SEG_E|SEG_F, //C
  SEG_B|SEG_C|SEG_D|SEG_E|SEG_G, //D
  SEG_A|SEG_D|SEG_E|SEG_F|SEG_G, //E
  SEG_A|SEG_G|SEG_E|SEG_F, //F
  SEG_A|SEG_C|SEG_D|SEG_E|SEG_F, //G
  SEG_C|SEG_E|SEG_F|SEG_G, //H
  SEG_C, //I
  0, //J           
  SEG_B|SEG_C|SEG_E|SEG_F|SEG_G, //K;
  SEG_D|SEG_E|SEG_F, //L
  0, //M
  SEG_C|SEG_E|SEG_G, //N
  SEG_C|SEG_D|SEG_E|SEG_G, //O
  0, //P
  0, //Q
  SEG_E|SEG_G, //R
  SEG_A|SEG_C|SEG_D|SEG_F|SEG_G, //S
  SEG_D|SEG_E|SEG_F|SEG_G, //T
  SEG_C|SEG_D|SEG_E, //U
  SEG_B|SEG_C|SEG_D|SEG_E|SEG_F, //V
  0, //W
  0, //X
  SEG_B|SEG_C|SEG_D|SEG_F|SEG_G, //Y
  0, //Z
  SEG_G //-
};
const int iMinus = 26;
#endif

void setupDisplay()  {
#ifdef SERIALDISPLAY
  mySerial.begin(9600); // initialize communication to the display
  delay(500);
  //mySerial.write(0x7A); // command byte for brightness
  //mySerial.write(0x01); // display brightness (lower = brighter)
#endif  
#ifdef TM1637
  display.setBrightness(0x01);
#endif
}


void displayLED(String ledMessage)
{
  uint8_t strLength = ledMessage.length(); 
  if (strLength <= 4) {
#ifdef SERIALDISPLAY
    if (strLength == 1) mySerial.write("   ");
    else if (strLength == 2) mySerial.write("  ");
    else if (strLength == 3) mySerial.write(" ");
    else {
      // no padding needed here
    }
    for(int i = 0; i < strLength; i++){
      mySerial.write(ledMessage[i]);   
    }
#endif

#ifdef TM1637
    ledMessage.toUpperCase();
    ledMessage = "    "+ledMessage;
    ledMessage = ledMessage.substring(ledMessage.length()-4);
    //Serial.println("message: -"+ledMessage+"-");

    uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
    for (int i = 3; i>=0; i--) {
      if (isDigit(ledMessage[i])) {
           data[i] = display.encodeDigit(ledMessage[i]-'0');
      } else 
      if (ledMessage[i]=='-') {
        data[i] = SEG_CHAR[iMinus];
      } else {
        data[i] = SEG_CHAR[ledMessage[i]-'A'];
      }
    } //for
    display.setSegments(data);    
#endif
  }
}




