/*
  VerSiLib project v05
  3.7.2025 MK
  -----------------------------------------------------
  Pulse Counter for FZU reader with tree modes and I2C LCD Display
  -----------------------------------------------------
  -----------------------------------------------------
  This codes includes three operational modes:
    - DEF: Default mode with periodic pulse counting
    - COUNT: Measures pulses over a specified interval
    - SCAN: Responds to X and Y axis triggers
  -----------------------------------------------------
  Serial comunication (Commands are terminated with ';'):  
  INPUTS:
    - *IDN?                     : Identify the instrument
    - *RST                      : Reset to default settings
    - HELP?                     : Display available commands
    - :STAT:ALL?                : Display current settings
    - :CONF:EDGE <input>,<edge> : Set edge detection for inputs (SPDMA, X, Y), (RISING, FALLING, CHANGE)
    - :CONF:DEF                 : Switch to default mode
    - :CONF:DEF:TIME <ms>       : Set integration time in ms for DEF mode
    - :CONF:DEF:MODE <type>     : Set display mode in DEF (COUNT or FREQ)
    - :CONF:COUNT               : Switch to measure pulses over specified ms
    - :CONF:COUNT:TIME <ms>     : Set integration time in ms for CONF mode
    - :CONF:COUNT:REPE <int>    : Set the maximum number of measurement repetitions for CONF mode
    - :CONF:SCAN                : Enter scan mode
  -----------------------------------------------------
  RESPONSE:
    Arduino UNO - VerSiLib project v0x date: DD.MM.YYYY : Device identification
    reset ok                                            : Confirmation of reset completion
    Help:                                               : Copy of the header
    mode DEF time #### ms mode COUNT/FREQ;              : Confirmation of Default and its configuration
    mode COUNT time #### ms repe ##;                    : Confirmation of Counting and its configuration
      no ## count ####;                                 : Measurement result number and measured value
    mode SCAN;                                          : Confirmation of Scan and its configuration
      ready;                                            : Ready for scan and waiting for start of movement Y X
      L ####;                                           : Confirmation of the start of a new line /move X after Y/ with number of the line 
      F ####;                                           : Measured values ​​of pulses in individual intervals   
      I ####;                                           : Confirmation end of line /move Y after X/ with the number of intervls  in this line

  -----------------------------------------------------
  Hardware Connections:
    - SPDMA sensor: Pin 5
    - X-axis encoder: Pin 3
    - Y-axis encoder: Pin 4
    - I2C LCD: SDA (A4), SCL (A5)
  -----------------------------------------------------
  Notes : 
  Ensure that the LiquidCrystal_I2C library is installed.
  Default Integration Time (DEF): 250 ms
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SPDMA_PIN 5
#define X_PIN 3
#define Y_PIN 4
#define BAUD_RATE 9600
#define project_name "Arduino VerSiLib project v05 03.07.2025"

// LCD init (LCD_ADDRESS, LCD_COLS, LCD_ROWS);
LiquidCrystal_I2C lcd(0x27, 8, 2);

// Pulse counters, mode, timing and flags
volatile unsigned long spdmaCount = 0;
volatile bool xTriggered = false;
volatile bool yTriggered = false;
unsigned long defTime = 250;
unsigned long countTime = 250;
unsigned long countRepe = 1;
enum Mode { DEF, COUNT, SCAN };
Mode currentMode = DEF;
unsigned long actCycle = 0;
bool showFreq = false;

// Edge detection setting
enum EdgeType { RISING_EDGE, FALLING_EDGE, CHANGE_EDGE };
EdgeType spdmaEdge = RISING_EDGE;
EdgeType xEdge = RISING_EDGE;
EdgeType yEdge = RISING_EDGE;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(BAUD_RATE); // set serial comunication
  
  Wire.begin(); // what is it

  // only rinsing
  attachInterrupt(digitalPinToInterrupt(SPDMA_PIN), spdmaISR, RISING);
  attachInterrupt(digitalPinToInterrupt(X_PIN), xISR, RISING);
  attachInterrupt(digitalPinToInterrupt(Y_PIN), yISR, RISING);

// more complicated
  // attachInterrupt(digitalPinToInterrupt(SPDMA_PIN), spdmaISR, getMode(spdmaEdge));
  // attachInterrupt(digitalPinToInterrupt(X_PIN), xISR, getMode(xEdge));
  // attachInterrupt(digitalPinToInterrupt(Y_PIN), yISR, getMode(yEdge));


  // Initialize I2C LCD display (on SDA/SCL)
  // lcd.init();
  // lcd.backlight();
  // updateLCDMode();

  // ini info
  Serial.println(project_name);
  printStatus();
}

void loop() {
  handleSerial(); // vyzkousi jestli je neco na seriove lince
  LED_blink(); // blikne aby bylo jasne ze prg bezi
  if (currentMode == DEF) {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= defTime) {
      lastUpdate = millis();
      unsigned long countSnapshot = spdmaCount;
      spdmaCount = 0;
      actCycle++;
      // Mode DEF only on LED
      Serial.print("Mode DEF cyklus : ");
      Serial.print(actCycle);
      Serial.print(" SPDMA count : ");
      Serial.println(countSnapshot);
      // Mode DEF only on LED but LED is not included yet  
      // lcd.clear();
      // lcd.setCursor(0, 0);
      // lcd.print("DEF");
      // lcd.setCursor(0, 1);
      // if (showFreq) {
      //   float freq = (countSnapshot * 1000.0) / defTime;
      //   lcd.print("F:");
      //   lcd.print(freq, 2);
      //   lcd.print("Hz");
      // } else {
      //   lcd.print("C:");
      //   lcd.print(countSnapshot);
      // }
    }
  }else if (currentMode == COUNT) {
    spdmaCount = 0;
    unsigned long start = millis();
    for (int i = 0; i < countRepe; i++) {
      while (millis() - start < countTime);
      unsigned long countSnapshot = spdmaCount;
      Serial.print("no ");
      Serial.print(i);
      Serial.print(" SPDMA count ");
      Serial.print(countSnapshot);
      Serial.println(";");
      // control for input from serial handleSerial(); 
    }
  }
}

void spdmaISR() { spdmaCount++; }
void xISR() { xTriggered = true; }
void yISR() { yTriggered = true; }

int getMode(EdgeType edge) {
  switch (edge) {
    case RISING_EDGE: return RISING;
    case FALLING_EDGE: return FALLING;
    case CHANGE_EDGE: return CHANGE;
  }
  return RISING;
}

String edgeToStr(EdgeType edge) {
  switch (edge) {
    case RISING_EDGE: return "RISING";
    case FALLING_EDGE: return "FALLING";
    case CHANGE_EDGE: return "CHANGE";
  }
}

void printStatus() {
  Serial.println("printStatus - under the construction");
  Serial.println("Type HELP?; for command list.");
}

void handleSerial() {
  static String Serial_input = "";
  while (Serial.available()) {
    char c = Serial.read();
    if ((c == '\n')||(c == ';')) {
      Serial_input.trim();
      Serial_input.toUpperCase();

      if (Serial_input == "*IDN?") {
        Serial.println(project_name);
      } else if (Serial_input == "*RST") {
        defTime = 250;
        currentMode = DEF;
        showFreq = false;
        spdmaEdge = xEdge = yEdge = RISING_EDGE;
        detachInterrupt(digitalPinToInterrupt(SPDMA_PIN));
        detachInterrupt(digitalPinToInterrupt(X_PIN));
        detachInterrupt(digitalPinToInterrupt(Y_PIN));
        attachInterrupt(digitalPinToInterrupt(SPDMA_PIN), spdmaISR, getMode(spdmaEdge));
        attachInterrupt(digitalPinToInterrupt(X_PIN), xISR, getMode(xEdge));
        attachInterrupt(digitalPinToInterrupt(Y_PIN), yISR, getMode(yEdge));
        Serial.println("reset done;");
      } else if (Serial_input == "HELP?") {
        Serial.println("Help:");
        Serial.println("*IDN?;                  : Identify the instrument");
        Serial.println("*RST;                   : Reset to default settings");
        Serial.println("HELP?;                  : Display available commands");
        Serial.println(":STAT:ALL?;             : Display current settings");
        Serial.println(":CONF:EDGE <in>,<edge>; : Set edge detection (SPDMA,X,Y),(RISING,FALLING,CHANGE)");
        Serial.println(":CONF:DEF;              : Switch to default mode");
        Serial.println(":CONF:DEF:TIME <ms>;    : Set integration time in DEF mode");
        Serial.println(":CONF:DEF:MODE <type>;  : Set display mode in DEF (COUNT or FREQ)");
        Serial.println(":CONF:COUNT;            : Switch to COUNT mode");
        Serial.println(":CONF:COUNT:TIME <ms>;  : Set time in COUNT mode");
        Serial.println(":CONF:COUNT:REPE <ms>;  : Set repetitions in COUNT mode");
        Serial.println(":CONF:SCAN;             : Enter scan mode");
      } else if (Serial_input.startsWith(":CONF:DEF:TIME")) {
        defTime = Serial_input.substring(17).toInt();
        Serial.print("Mode DEF integration time ");
        Serial.print(defTime);
        Serial.println("ms;");
        actCycle = 0;
      } else if (Serial_input.startsWith(":CONF:DEF:MODE")) {
        String mode = Serial_input.substring(17);
        if (mode == "COUNT") {
          showFreq = false;
          Serial.println("mode count;");
        } else if (mode == "FREQ") {
          showFreq = true;
          Serial.println("mode freq;");
        }
        actCycle = 0;
      } else if (Serial_input == ":CONF:DEF") {
        currentMode = DEF;
        Serial.println("mode def;");
      } else if (Serial_input.startsWith(":CONF:COUNT:TIME")) {
        countTime = Serial_input.substring(17).toInt();
        Serial.print("Mode COUNT integration time ");
        Serial.print(countTime);
        Serial.println("ms;");
        actCycle = 0;
      } else if (Serial_input.startsWith(":CONF:COUNT:REPE")) {
        countTime = Serial_input.substring(17).toInt();
        Serial.print("Mode COUNT Repetition ");
        Serial.print(countRepe);
        Serial.println(";");
        actCycle = 0;
      } else if (Serial_input == ":CONF:COUNT") {
        currentMode = COUNT;
        // Confirmation of Counting and its configuration
        Serial.print("mode COUNT time ");
        Serial.print(countTime);
        Serial.print(" ms Repetition");
        Serial.println(countRepe);
        Serial.println(";");
        actCycle = 0;
      } else if (Serial_input == ":CONF:SCAN") {
        currentMode = SCAN;
        Serial.println("mode scan;");
        Serial.println("ready;");
        // Further SCAN logic should be placed here
      // } else if (Serial_input.startsWith(":CONF:EDGE")) {
      //   int commaIndex = Serial_input.indexOf(',');
      //   String target = Serial_input.substring(12, commaIndex);
      //   String edge = Serial_input.substring(commaIndex + 1);
      //   Edge e;
      //   if (edge == "RISING") e = RISING_EDGE;
      //   else if (edge == "FALLING") e = FALLING_EDGE;
      //   else e = CHANGE_EDGE;
      //   if (target == "SPDMA") {
      //     spdmaEdge = e;
      //     attachInterrupt(digitalPinToInterrupt(SPDMA_PIN), spdmaISR, getMode(spdmaEdge));
      //     Serial.println("edge spdma set;");
      //   } else if (target == "X") {
      //     xEdge = e;
      //     attachInterrupt(digitalPinToInterrupt(X_PIN), xISR, getMode(xEdge));
      //     Serial.println("edge x set;");
      //   } else if (target == "Y") {
      //     yEdge = e;
      //     attachInterrupt(digitalPinToInterrupt(Y_PIN), yISR, getMode(yEdge));
      //     Serial.println("edge y set;");
      //   }
      } else if (Serial_input == ":STAT:ALL?") {
        Serial.print("mode ");
        if (currentMode == DEF) Serial.println("def;");
        else if (currentMode == COUNT) Serial.println("count;");
        else Serial.println("scan;");
        Serial.print("integration ");
        Serial.print(defTime);
        Serial.println("ms;");
        Serial.print("display ");
        Serial.println(showFreq ? "freq;" : "count;");
        Serial.print("edge spdma ");
        Serial.println(edgeToStr(spdmaEdge));
        Serial.print("edge x ");
        Serial.println(edgeToStr(xEdge));
        Serial.print("edge y ");
        Serial.println(edgeToStr(yEdge));
      } else {
        Serial.print(Serial_input);
        Serial.println(" -> unknown command. Type HELP?;");
      }
      Serial_input = ""; // reset buffer
    } else {
      Serial_input += c;
      Serial.print("Serial input > ");
      Serial.println(Serial_input);
    }
  }
}

void LED_blink() { // the function signal runs 
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
  // i++;
  // Serial.print("cyklus : ");
  // Serial.println(i);
}


