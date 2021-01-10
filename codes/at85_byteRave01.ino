/*
     code used in jamuary02 - Bytebeat
     https://youtu.be/8HQUXlStqK8
     schematic at https://imgur.com/SekUTgN
     noizhardware.com
     enjoy :)

     ported to ATtiny Punk Console by noisio
     runs now (in need for the audio bootloader) at 16MHz
*/
#include <avr/io.h>

#define POT1 3
#define POT2 2
#define POT3 1
#define POT4 0

#define SYNC 0   // input
#define WAVE 1   // output

volatile int t, val1, val2, val3;

void setup() {
  pinMode(WAVE, OUTPUT);
  pinMode(POT1, INPUT);

  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                                      // Timer interrupts OFF
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10;   // PWM OCR1A, clear on match, 1:1 prescale

  // Set up Timer/Counter0 for 40kHz interrupt to output samples.
  TCCR0A = 3 << WGM00;                            // Fast PWM
  TCCR0B = 1 << WGM02 | 2 << CS00;                // 1/8 prescale
  TIMSK = 1 << OCIE0A;                            // Enable compare match, disable overflow
  OCR0A = 49;                                     // Divide by A3 >> 2
}

void loop() {

  //     out = (t%127)*((t-65&t)>>((analogRead(POT1)>>2)/14+7));

  val1 = (analogRead(POT1) >> 2) / (analogRead(POT2) >> 5) + 7;
  val2 = analogRead(POT3) >> 1;

  /* can't check at the moment but should be fun:
  val2 = analogRead(POT4) - 512; //POT4 needs to be wired with a 10k to ground to not reset the chip - https://github.com/noisio/ATtiny-Punk-Console/blob/master/docs/ATPCv3.pdf
  OCR0A = analogRead(POT3) >> 2; //change the speed of the interrupt routine 
  */
}

// Interrupt Service Routine called at 40 KHz
ISR(TIMER0_COMPA_vect) {
  OCR1A = (t % val2) * ((t - 65 & t) >> (val1));
  t++;
}
