//////////////// Failure Unit ... Bootloader @8MHz //////////////////

int pot1 = 3;
int pot2 = 2;
int pot3 = 1;
int pot4 = 0;
int sync = 0;
int wave = 1;

unsigned int Acc = 255;
unsigned int freq;                                       // main frequency

unsigned long tick;                                      // debounce counter
unsigned int tack;                                       // step counter of Arp
unsigned int buff = 256;
boolean trig = false;
unsigned int arpCount;

bool intTrig = false;
bool intTrigFlag = false;

unsigned int arpTempo;                                   // tempo of Arpeggiator
boolean trigSync;
unsigned int light;                                      // length of sync Signal

void setup() {

  pinMode(sync, INPUT);

  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                                             // Timer interrupts OFF
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10;          // PWM OCR1A, clear on match, 1:1 prescale

  pinMode(wave, OUTPUT);                                 // Enable PWM output pin

  // Set up Timer/Counter0 for 20kHz interrupt to output samples.
  TCCR0A = 3 << WGM00;                                   // Fast PWM
  TCCR0B = 1 << WGM02 | 2 << CS00;                       // 1/8 prescale
  TIMSK = 1 << OCIE0A;                                   // Enable compare match, disable overflow
  OCR0A = 49;                                            // Divide by A3 >> 2
}

void loop() {

  // This routine called only once at beginning
  // gives you one chance to start internal trigger
  if (digitalRead(sync) && !intTrigFlag) {               // If there is signal on sync and internal trigger flag is unset
    intTrig = true;                                      // Start Internal Trigger Mode
    pinMode(sync, OUTPUT);                               // Set pinMode to output
  } else {
    intTrigFlag = true;                                  // From now on you'll get no chance again
  }

  /////////////////

  OCR0A += analogRead(pot1) >> 2;                        // start crazy mode: increment register every loop
  freq = ((analogRead(pot2) + tack * analogRead(pot3)) << 4) + 1 ;  // read the freq starting point, add arp-status multiplied by A2-read

  if (intTrig) {
    
    arpTempo = (analogRead(pot4) - 542) << 3;
    tick += 1;

    if (tick > arpTempo) {
      if (tack >= 8) {
        tack = 0;
      }
      tack += 1;
      tick = 0;
      trigSync = true;
    }

    if (trigSync) {
      digitalWrite(0, HIGH);
      light = 0;
    }

    trigSync = false;
    light += 1;

    if (light >= 64) {
      digitalWrite(0, LOW);
    }

  } else {

    arpCount = (528 - (analogRead(pot4) - 544)) >> 5;

    if (digitalRead(0) && !trig) {
      trig = true;                                      // set trigger flag
      tick = 0;                                         // start counting for debounce

      if (tack >= arpCount) {                           // if arp-counter end is reached
        tack = 0;                                       // start from beginning
      }

      tack += 1;                                        // increment arp-counter
    }

    if (trig && tick < buff) {
      tick += 1;
    }

    if (tick >= buff) {
      trig = false;
    }
  }

}

// Interrupt Service Routine called 20000 times a second
ISR(TIMER0_COMPA_vect) {

  Acc = Acc + freq;
  if (Acc & 0x80) {
    OCR1A = OCR0A;
  } else {
    OCR1A = 0;
  }

}
