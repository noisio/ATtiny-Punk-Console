//////////////// Sweep ... Bootloader @8MHz //////////////////

#include <avr/pgmspace.h>

unsigned int Acc = 255;
unsigned int note, noteTop, noteBase, type;                   //

unsigned int vol, sweepTime;
unsigned long tick;                                     // counter
boolean trig, play;                                     //



const byte sine256[] PROGMEM = { // sine wavetable
  0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 128,
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246,
  248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211,
  208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65,
  62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0
};



void setup() {

  pinMode(0, INPUT);

  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                                          // Timer interrupts OFF
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10;       // PWM OCR1A, clear on match, 1:1 prescale
  pinMode(1, OUTPUT);                                 // Enable PWM output pin

  // Set up Timer/Counter0 for 20kHz interrupt to output samples.
  TCCR0A = 3 << WGM00;                                // Fast PWM
  TCCR0B = 1 << WGM02 | 2 << CS00;                    // 1/8 prescale
  TIMSK = 1 << OCIE0A;                                // Enable compare match, disable overflow
  OCR0A = 49;                                         //
}

void loop() {


  if (digitalRead(0)) {                               // if trigger
    sweepTime = (analogRead(A3) >> 3) + 1;            // read
    noteBase = (analogRead(A2) + 512) >> 3;           // read low note
    noteTop = (analogRead(A1) + 64) << 3;             // read high note
    type = (analogRead(A0) - 512) >> 7;               // waveform
    trig = true;                                      // set trigger flag
  };




}


ISR(TIMER0_COMPA_vect) {

  if (trig && !play) {
    play = true;                                      // play
    vol = 255;                                        // play loud!
    tick = 0;                                         // start counting
    Acc = 0;
  }

  if (play) {

    if (tick == 0) {
      note = noteTop;                                 // start with the top note
    }

    switch (type) {

      case 0:                                         // Square
        Acc = Acc + note;
        if ((Acc >> 8) & 0x80) {
          OCR1A = (255 * vol) >> 8;
        } else {
          OCR1A = 0;
        }
        break;

      case 1:                                         // Ramp Up
        Acc = Acc + note;
        OCR1A = ((Acc >> 8) * vol) >> 8;
        break;

      case 2:                                         // Triangle
        Acc = Acc + note;
        signed char Temp, Mask;
        Temp = Acc >> 8;
        Mask = Temp >> 7;
        OCR1A = (((Temp ^ Mask) << 1) * vol) >> 8 ;
        break;

      case 3:                                         // Sinus
        Acc = Acc + note;
        OCR1A = (pgm_read_byte(&sine256[Acc >> 8]) * vol) >> 8;
        break;

    }


    if (note > noteBase && tick % sweepTime == 0) {            //
      note -= 8;
    }

    if (note <= noteBase && tick % 4 == 0) {
      vol -= 1;
    }

    if (vol <= 1) {
      play = !play;
      trig = !trig;
    }

    tick += 1;                                                    // count one up
  }
}
