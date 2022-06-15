/*  
  # # # ATtiny85 LFO and Envelope Generator # # #  
               _     _
   _ __   ___ (_)___(_) ___
  | '_ \ / _ \| / __| |/ _ \
  | | | | (_) | \__ \ | (_) |
  |_| |_|\___/|_|___/_|\___/
                        2021


  Start ADSR mode by pressing the push button at startup >>

  AT85 clock must be set to 8MHz.
*/

#include <avr/pgmspace.h>

int sync = 0;                                     // digital input pin

unsigned long latch;
unsigned int freq, type, depht, bias, temp;


const byte sine256[] PROGMEM = { // sine wavetable
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246,
  248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211,
  208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65,
  62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33,
  35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 128
};

const int pot1 = 3;
const int pot2 = 2;
const int pot3 = 1;
const int pot4 = 0;

unsigned int pot1Read, pot2Read, pot3Read, pot4Read, att, dec, sus, rel, Acc, AccSus;
byte adsrStep;                                        // 0 = stopped, 1 = attack, 2 = decay, 3 = sustain, 4 = release
bool gateOpen, prevGate, ADSRMode;
bool startUp = true;



void setup() {

  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                                          // Timer interrupts OFF
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10;       // PWM OCR1A, clear on match, 1:1 prescale
  pinMode(1, OUTPUT);                                 // Enable PWM output pin

  // Set up Timer/Counter0 for interrupt to output samples.
  TCCR0A = 3 << WGM00;                                // Fast PWM
  TCCR0B = 1 << WGM02 | 3 << CS00;                    // 1/64 prescale
  TIMSK = 1 << OCIE0A;                                // Enable compare match, disable overflow
  OCR0A = 249;                                         // Divide by 250 gives 1kHz ISR
}

void loop() {

  gateOpen = PINB & 0x01;

  if (startUp) {
    if (gateOpen) {
      ADSRMode = true;
    } else {
      pinMode(0, OUTPUT);
      digitalWrite(0, HIGH);
    }
    startUp = false;
  }

  if (gateOpen && !prevGate) {
    prevGate = true;
    sus = analogRead(pot3) >> 2;
    AccSus = sus << 8;
    dec = AccSus / (analogRead(pot2) + 1);
    att = 65535 / (analogRead(pot1) + 1);
    rel = AccSus / ((map(analogRead(pot4), 1023, 552, 1023, 0)) + 1);
    Acc = 0;
    adsrStep = 1;                                       // start with attack;
  }


  if (!ADSRMode) {

    type = analogRead(pot1);              //

    freq = (analogRead(pot2) << 1) + 3;

    depht = analogRead(pot3) >> 2;

    bias = map(analogRead(pot4), 1023, 552, 255, 0);
  }

}


ISR(TIMER0_COMPA_vect) {

  if (ADSRMode) {

    switch (adsrStep) {

      case 0:                                             // nothing here
        {
          break;
        }

      case 1:                                             // attack
        {
          int prevAcc = Acc;
          Acc += att;
          if (Acc < prevAcc) {                              // goto next step if register flows over
            OCR1A = 255;
            adsrStep = 2;
            Acc = 65535;
            break;
          }
          OCR1A = Acc >> 8;
          break;
        }

      case 2:                                             // decay
        {
          Acc -= dec;
          if (Acc < AccSus) {                              // register overflow
            Acc = AccSus;
            adsrStep = 3;
            break;
          }
          OCR1A = Acc >> 8;
          break;
        }

      case 3:                                             // sustain
        {
          if (gateOpen) {

            break;

          } else {
            adsrStep = 4;
            prevGate = false;
            break;
          }
        }

      case 4:                                             // release
        {
          int prevAcc = Acc;
          Acc -= rel;
          if (Acc > prevAcc) {                              // register overflow
            OCR1A = 0;
            adsrStep = 0;
            break;
          } else {
            OCR1A = Acc >> 8;
            break;

          }
        }
    }


  } else {

    if (type <= 204) {                          // Square

      Acc = Acc + freq;
      if ((Acc >> 8) & 0x80) {
        temp = 255;
      } else {
        temp = 0;
      }

    } else if (type > 205 && type <= 408) {                          // Sinus

      Acc = Acc + freq;
      temp = pgm_read_byte(&sine256[Acc >> 8]);

    } else if (type > 409 && type <= 612) {                          // Triangle

      Acc = Acc + freq;
      signed char Temp, Mask;
      Temp = Acc >> 8;
      Mask = Temp >> 7;
      temp = (Temp ^ Mask) << 1 ;

    } else if (type > 613 && type <= 817) {                          // Ramp Up

      Acc = Acc + freq;
      temp = 255 - (Acc >> 8);

    } else if (type > 818 ) {                                         // Ramp Down

      Acc = Acc + freq;
      temp = Acc >> 8;

    }

    temp = ((temp * depht) >> 8) + bias - (depht >> 1);

    if (temp > 255 && temp < 511) {
      temp = 255;
    } else if (temp > 511) {
      temp = 0;
    }

    OCR1A = temp;
  }
}
