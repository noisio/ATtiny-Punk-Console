// Playing the Atari Punk Console with ATtiny85 using two more Potentiometers for sweep between 2 states
//
// ATtiny85 @ 16Mhz internal

// assign variables to
int pot1 = 3;                                     // 4 analog
int pot2 = 2;
int pot3 = 1;
int pot4 = 0;

int sync = 0;                                     // 2 digital pins
int wave = 1;

unsigned int time1;                               // 1st oscillator
unsigned int pulseWidth, pulseWidthTarget;        // 2 pulse widths
unsigned int duration;                            // Time to reach target pulse width

unsigned int tick1, tick2, tick3;                 // Cycle counters

bool pulseStart;
bool startUp = true;                              // Is only true at startup


void setup() {

  pinMode(wave, OUTPUT);                          // PWM output pin
  pinMode(sync, INPUT);                           // Sync pin as input

  // For setting up Timers and learn much about ATtinys go for 
  // David Johnson-Davies blog: technoblogy.com

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

  time1 = 134 - (analogRead(pot1) >> 3);          // Set frequency for 1st oscillator

  pulseWidthTarget = 518 - (analogRead(pot2) >> 1);    // Read base pulse width

  // If there is a trigger signal and sweep is ended (pulse width = target pulse width).
  if (digitalRead(sync) && (pulseWidth) == (pulseWidthTarget)) {
    pulseWidth = 518 - (analogRead(pot3) >> 1);   // Read new value for pulse width start sweep
    tick3 = 0;                                    // Set ticker3 to start
  }

  duration = (analogRead(pot4) - 512) >> 1;       // Read time variable for sweep

  // Set a starting pulseWidth only once
  if (startUp) {                                  // If you are 1st time here
    pulseWidth = pulseWidthTarget;
    startUp = false;                              // Never come back again
  }
}

// Interrupt Service Routine called 40000 times a second
ISR(TIMER0_COMPA_vect) {

  // Increment tickers
  tick1 += 1;
  tick2 += 1;
  tick3 += 1;

  if (tick1 % time1 == 0 && !pulseStart) {        // If counter1 is multiple of oscillator1 and no pulse started before
    OCR1A = 255;                                  // Set square wave to top
    pulseStart = true;                            // Remember that pulse is started
    tick1 = 0;
    tick2 = 1;                                    // Start 2nd ticker
  }

  //Step closer to target pulseWidth if duration ticker is a multiple of duration time
  if (tick3 % duration == 0) {
    if (pulseWidth < pulseWidthTarget) {          // either count one step up
      pulseWidth += 1;
    } else if (pulseWidth > pulseWidthTarget) {   // or one step down
      pulseWidth -= 1;
    }
  }

  if (tick2 % pulseWidth == 0)                    // If tick2 is multiple of pulseWidth
  {
    OCR1A = 0;                                    // Square wave to bottom
    pulseStart = false;                           // Wait for next turn
  }
}
