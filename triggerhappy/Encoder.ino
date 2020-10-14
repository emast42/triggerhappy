/*Rotary Encoder code from Oleg, Circuits@Home 
http://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino
plus interrupt code by "LEo" and "bikedude"
*/

#define ENC_A 2 //encoder pins
#define ENC_B 3
#define ENC_PORT PIND
int encoderCounter = 2; //variable for smoothing encoder response

void setupEncoder()
{
  /* Setup encoder pins as inputs */
  pinMode(ENC_A, INPUT);
  digitalWrite(ENC_A, HIGH);
  pinMode(ENC_B, INPUT);
  digitalWrite(ENC_B, HIGH);

  // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoder, CHANGE);

  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoder, CHANGE);
}

void doEncoder()
{
  encoderCounter = encoderCounter + read_encoder(); 
  //make encoder less jumpy by waiting for 2 clicks in same direction!
  if (encoderCounter > 2) {
    tmpdata = 1;
    encoderCounter = 0;
  }
  if (encoderCounter < -2) {
    tmpdata = -1;
    encoderCounter = 0;
  }
}

/* returns change in encoder state (-1,0,1) */
int8_t read_encoder()
{
  static int8_t enc_states[] = {
    0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0    };
  static uint8_t old_AB = 0;
  uint8_t new_AB = ENC_PORT;

  new_AB >>= 2; //Shift the bits two positions to the right
  old_AB <<= 2;
  old_AB |= (new_AB & 0x03); //add current state
  return ( enc_states[( old_AB & 0x0f )]);
}


