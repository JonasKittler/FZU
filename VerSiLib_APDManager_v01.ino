#include <LiquidCrystal.h>

/* APD protection and counter unit for scanning microscope
 * Connected to APD (Timer T0, pin D4). 
 * Function: 
 *    .) When powered: continuously monitor APD count rate and put pin Dxx to high/low to disable the APD.
 *    .) communication with PC
 *    Acuqisition modes: 
 *    .) Accumulation between external events (pixel size configured in the motorcontroller)
 *         Raising edge on pin T1/D5 stores & sends APD count and resets counter 
 *    .) Accumulation with fixed time triggered by external signal: 
 *         Accumulation time configured via PC. c:\Users\kittler\Documents\Arduino\Counter\Adruino_counter_v01.ino\Adruino_counter_v01.ino.ino
 +         Raising edge on pin T1/D5 resets the APD count and starts a timer
 *         Timer interrupt stops timer and stores / sends counter value
 *    .) Spatial mode: T1 with pin D5 used as step counter similar to time mode but instead of time, counting the pulses on D5 and stopping the APD count after a configurable number of counts
 *
 *    Communication protocol: 
 *    Start character: 
          ! ... Commands
          ? ... Queries (expect response from microcontroller)
 *    Delimiter: ; 
 *    Commands examples: 
 *       !s;   Start 
 *       !p;   Stop
 *       !t5000;  Accumulation with fixed time. Time = 5000µs. 
 *       !a;  Accumulation between external events
 */

#define APD_CONTROL_PIN 4  // APD signal
#define APD_INPUT_PIN 5
#define STEP_INPUT_PIN 3  // Connect to Motor control SYNC_OUT
#define FGEN_PIN 7        // Mockup frequency generator output port for testing
#define BAUD_RATE 9600

// operation modes of the device
#define MODE_COUNT 1  // accumulate counts during step signals
#define MODE_FREQ 2   // normalize counts during step signals to 1


unsigned int Timer1OverflowCount;
bool fgen = 0;                  // dummy frequency generator
unsigned int count;             // main loop counter
unsigned long APD_count;        // accumulated apd counts
unsigned long last_timestamp;   // micros() value at last count_update
unsigned long timestamp;        // micros() timestamp read by the step interrupt
bool count_update = 0;          // APD count updated by motor step
unsigned int mode = MODE_FREQ;  // operation mode

// Setup function
void setup() {

  count_update = 0;    // APD count updated flag
  APD_count = 0;       // APD count between the last two interrupts
  fgen = 0;            // frequency generator flag
  last_timestamp = 0;  // last timestamp value
  timestamp = 0;       // timestamp taken at step
  // dummy
  // loop counter
  count = 0;

  // Initialize APD control pin
  pinMode(APD_CONTROL_PIN, OUTPUT);
  digitalWrite(APD_CONTROL_PIN, HIGH);  // Enable APD by default

  // configure frequency generator pin for test purpose
  pinMode(FGEN_PIN, OUTPUT);
  digitalWrite(FGEN_PIN, LOW);

  // Initialize Timer1 for pulse counting
  Timer1OverflowCount = 0;  // APD counter

  // configure counters for external source
  //timer0SetExternalClock();
  timer1SetExternalClock();

  // initialize the position encoder input interrupt
  attachInterrupt(digitalPinToInterrupt(STEP_INPUT_PIN), step, RISING);

  // Setup Serial Communication
  Serial.begin(BAUD_RATE);
}

// Main loop
void loop() {
  // Check Timer1 count rate
  if (checkCountRate()) {
    disableAPD();
  }
  if (fgen && ++count == 1) {
    digitalWrite(FGEN_PIN, !digitalRead(FGEN_PIN));
    count = 0;
  }

  if (count_update) {
    count_update = 0;
    if (mode == MODE_COUNT) {
      Serial.print("A");
      Serial.print(APD_count);
      Serial.print(";");
    } else if (mode == MODE_FREQ) {
      Serial.print("F");
      Serial.print((unsigned long)(APD_count * (1000000.0 / (timestamp - last_timestamp))));
      Serial.print(";");
      //      Serial.print("F"); Serial.print((unsigned long)(timestamp-last_timestamp)); Serial.print(";");
      last_timestamp = timestamp;
    }
  }
  // Handle Serial Commands
  if (Serial.available()) {                        // je prtomen znak naseriove lince
    String command = Serial.readStringUntil(';');  // cte dokud neni ;
    handleCommand(command);
  }
}

// Function to check the count rate
bool checkCountRate() {
  // Implementation to check if count rate exceeds 2MHz
}

// Function to handle different commands
void handleCommand(String command) {
  // Parse and execute commands
  switch (command.charAt(0)) {
    case 'f':
      fgen = !fgen;
      break;
    case '1':
      Serial.println(TCNT1);
      break;
    case 'e':
      Serial.println("e");
      break;
    case '4':
      digitalWrite(FGEN_PIN, 1);
      Serial.println(digitalRead(APD_INPUT_PIN));
      digitalWrite(FGEN_PIN, 0);
      Serial.println(digitalRead(APD_INPUT_PIN));
    default:
      break;
  };
}

// Function to disable APD
void disableAPD() {
  digitalWrite(APD_CONTROL_PIN, LOW);
}

// Function to enable APD
void enableAPD() {
  digitalWrite(APD_CONTROL_PIN, HIGH);
}

// Additional functions for data acquisition modes

void timer1SetExternalClock(void) {
  // Timer1 Setting - Reset - this will also stop the timer
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  // clear Timer
  TCNT1 = 0x0000;
  //Timer1 Overflow Interrupt Enable
  TIMSK1 |= 1 << TOIE1;

  pinMode(APD_INPUT_PIN, INPUT);

  // external clock on pin38 rising edge  (start timer)
  TCCR1B |= (1 << CS12) | (1 << CS11) | (1 << CS10);
  // TCCR1B = 0x07;
}

/*
// interrut service routines
// ISR Timer0 overflow (Position counter)
ISR(TIMER0_OVF_vect){ 
  TCNT0  = 0;                  //First, set the timer back to 0 so it resets for next interrupt
  Timer0OverflowCount++;
  Serial.println("T0 OVF");
}
*/

// ISR Timer1 overflow (APD counter)
ISR(TIMER1_OVF_vect) {
  TCNT0 = 0;  //First, set the timer back to 0 so it resets for next interrupt
  Timer1OverflowCount++;
  //  Serial.println("T1 OVF");
}

// interrupt service routine for position input
void step(void) {
  APD_count = Timer1OverflowCount * 0x10000 + TCNT1;
  //  APD_count = TCNT1;
  TCNT1 = 0;
  Timer1OverflowCount = 0;
  count_update = 1;
  timestamp = micros();
}