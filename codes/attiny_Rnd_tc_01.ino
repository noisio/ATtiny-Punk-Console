//////////////// Random Tickr ... Bootloader @8MHz //////////////////

int pot1 = 3;
int pot2 = 2;
int pot3 = 1;
int pot4 = 0;
int sync = 0;
int wave = 1;

unsigned int Acc = 255;
unsigned int freq = 857; // main frequency
unsigned int width;

unsigned long tick = 0;



void setup() {

  pinMode(sync, INPUT);

  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                                    // Timer interrupts OFF
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10; // PWM OCR1A, clear on match, 1:1 prescale

  pinMode(wave, OUTPUT);                        // Enable PWM output pin

  // Set up Timer/Counter0 for 20kHz interrupt to output samples.
  TCCR0A = 3 << WGM00;                          // Fast PWM
  TCCR0B = 1 << WGM02 | 2 << CS00;              // 1/8 prescale
  TIMSK = 1 << OCIE0A;                          // Enable compare match, disable overflow
  OCR0A = 49;                                   // Divide by A3 >> 2
}

void loop() {

OCR0A += analogRead(pot1) >> 2;
freq = analogRead(pot2);
freq += random(analogRead(pot3));
width = (1048 - (analogRead(pot4) << 1)) >> 2;

}

// Interrupt Service Routine called 20000 times a second
ISR(TIMER0_COMPA_vect) {

  Acc = Acc + freq;

 if ((Acc >> 8) & width) {
    OCR1A = OCR0A << 7;
  } else {
    OCR1A = 0;
  }

  tick += 1;
}
