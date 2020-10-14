
/* update lfsr: */

void update_lfsr(struct LFSR* _lfsr) {

// taps 1, 2:

    boolean t1 = getbitX(_lfsr->tap1, _lfsr->data);   
    boolean t2 = getbitX(_lfsr->tap2, _lfsr->data);
   
//  feedback:
    boolean bit_n = paritygen(_lfsr->bit_0, t1, t2, _lfsr->length); 
    
// update shift register:
    _lfsr->data = (_lfsr->data >> 1) | bit_n;  

// let's not get stuck:
     if (!_lfsr->data || _lfsr->data == 0xFFFF) {_lfsr->data = random(_lfsr->length);} 

// output value:
    _lfsr->bit_0 = _lfsr->data & 1u; 
  
};

/* return bit x of lsfr: */
	
boolean getbitX (uint8_t  _bit, uint16_t _lsfr) {  
	return (_lsfr >> _bit) & 1u;
}

/* calc. feedback: */

boolean paritygen(uint8_t bit_out, boolean _tap1, boolean _tap2, uint16_t _SRlength) {

        return ((bit_out ^ _tap1 ^ _tap2) << (_SRlength - 1));   // XOR taps and bit 0

}


