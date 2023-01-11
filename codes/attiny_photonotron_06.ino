// ################ flash @ 8MHz ###################


// assign variables to
int ldr1 = 3;     // 3 analog and
int ldr2 = 1;
int pot = 0;
int mode = 0;     // 3 digital pins
int waveP = 1;    // positive Welle
int waveN = 4;    // negative Welle

unsigned int Acc1, Acc2, freq1, freq2, vol, tune, temp;                              // Cycle counters as timing variables
bool sinus;

const byte sine256[] PROGMEM = { // sine wavetable
  0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 128,
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246,
  248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211,
  208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65,
  62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0
};




void setup() {

  pinMode(waveP, OUTPUT);                          // PWM output pin
  pinMode(waveN, OUTPUT);                          // PWM output pin negative

  // Find clear and very well written articles about setup ATtiny85 for audio at
  // David Johnson-Davies blog: technoblogy.com

  // Following this instructions start with:
  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                                      // Timer interrupts OFF
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10;   // PWM OCR1A, clear on match, 1:1 prescale

  // Set up Timer/Counter0 for 20kHz interrupt to output samples.
  TCCR0A = 3 << WGM00;                            // Fast PWM
  TCCR0B = 1 << WGM02 | 2 << CS00;                // 1/8 prescale
  TIMSK = 1 << OCIE0A;                            // Enable compare match, disable overflow
  OCR0A = 49;                                     // Divide by A3 >> 2
}



void loop() {

  sinus = !digitalRead(mode);

  if (sinus) {

    OCR0A = 49;
    freq1 = analogRead(ldr1) << 3;            // read frequency
    freq2 = analogRead(ldr2) << 2;
    vol = map(analogRead(pot), 602, 1024, 255, 0);

  } else {

    freq1 = analogRead(ldr1) >> 3;
    //    freq2 = (512 - analogRead(ldr2)) >> 3;
    vol = (analogRead(pot) - 667) >> 5;
    tune = analogRead(ldr2);
    freq2 = 255;
    OCR0A += 255 + vol;

  }


}

// Interrupt Service Routine called 20k times a second
ISR(TIMER0_COMPA_vect) {


  Acc1 = Acc1 + freq1;
  Acc2 = Acc2 + freq2;

  if (sinus) {

    temp = ((pgm_read_byte(&sine256[Acc1 >> 8]) + (pgm_read_byte(&sine256[Acc2 >> 8]))) * vol) >> 9;
    OCR1A = temp;
    OCR1B = temp ^ 255;

  } else {

    temp = (OCR0A * tune) >> 8;

    if (Acc1 & 0x80) {
      OCR1A = temp;
      OCR1B = temp ^ 255;
    } else if (Acc2 & 0x80) {
      OCR1A = temp ^ 255;
      OCR1B = temp;
    } else {
      OCR1A = 0;
      OCR1B = 0;
    }
  }
}
