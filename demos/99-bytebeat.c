#include <stdio.h>
#include <inttypes.h>

/*
https://www.tastyfish.cz/lrs/bytebeat.html
Outputting the variable i creates a periodical saw-shaped beat, multiplication/division decreases/increases the speed, addition/subtraction shifts the phase backward/forward.
Squaring (and other powers) create a wah-wah effect.
Crazier patterns can be achieved by using the variable in places of numerical constants, e.g. i << ((i / 512) % 8) (shifting by a value that depends on the variable).
Modulo (%) increases the frequency and decreases volume (limits the wave peak).
So called Sierpinski harmonies are often used melodic expressions of the form i*N & i >> M.
Bitwise and (&) can add distortion (create steps in the wave).
A macro structure of the song (silent/louds parts, verse/chorus, ...) can be achieved by combining multiple patterns with some low-frequency pattern, e.g. this alternates a slower and faster beat: int cond = (i & 0x8000) == 0;, cond * (i / 16) + !cond * (i / 32)
Extra variables can add more complexity (e.g. precompute some variable a which will subsequently be used multiple times in the final formula).
*/

int main(void) {
  	for (int t = 0;; ++t) {
    	putchar(
    		t|(t<<((t/920)%16))|(t/3*t&(t<<13)*t)|(t%16386?123:t&203?148:3)&(t/920)
    		// (t/8)>>(t>>9)*t/((t>>14&3)+4)
    		// ((1-(((t+10)>>((t>>9)&((t>>14))))&(t>>4&-2)))*2)*(((t>>10)^((t+((t>>6)&127))>>10))&1)*32+128
    		// t*((0xbadbea75>>((t>>12)&30)&3)*0.25*(0x5afe5>>((t>>16)&28)&3))
    		// ((t>>4)*(13&(0x8898a989>>(t>>11&30)))&255)+((((t>>9|(t>>2)|t>>8)*10+4*((t>>2)&t>>15|t>>8))&255)>>1)
            // t*((t>>12|t>>8)&63&t>>4)
      		// ((0x47 >> ((t >> 9) % 32)) & (t >> (t % 32))) | (0x57 >> ((t >> 7) % 32)) | (0x06 >> ((t >> ((((t * 11) >> 14) & 0x0e) % 32)) % 32))
    	);
	}

  	return 0;
}
