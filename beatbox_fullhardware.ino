// Pin init Chinese Nano Knockoff
#define pin_RX0 0 // serial with PC
#define pin_TX1 1 // serial with PC
#define pin_D2 2
#define pin_D3 3
#define pin_D4 4
#define pin_D5 5
#define pin_D6 6
#define pin_D7 7
#define pin_D8 8
#define pin_D9 9
#define pin_D10 10
#define pin_D11 11
#define pin_D12 12
#define pin_D13 13
#define pin_A0 14
#define pin_A1 15
#define pin_A2 16
#define pin_A3 17
#define pin_A4 18
#define pin_A5 19
#define pin_A6 20  // analogRead() ONLY
#define pin_A7 21  // analogRead() ONLY

// Beatbox specific inputs
#define Sel0 pin_A2
#define Sel1 pin_A3
#define Sel2 pin_A4
#define Z pin_A5
#define PP_LED pin_A1
#define ROT_A pin_A6
#define ROT_B pin_A7
#define LINE_OUT pin_D12
#define TEMPO_LED pin_D11

// DEBUG
bool DEBUG_FLAG = 1;
unsigned long DEBUG_previousMillis = 0;

// Rotary Encoder control
int currentStateCLK;
int lastStateCLK;
int CLK, DT;
String currentDir = ""; // Only needed for Serial Data
int rotaryMod = 0;

// Include Display Chip module
#include <ICM7218C.h>

#define HEXA ICM7218C::HEXA
#define CODEB ICM7218C::CODEB
#define NO_PIN ICM7218C::NO_PIN

enum DISP_FONT {
  GREEN, RED, YELLOW
};

// State Machine control
//  States will be defined below Setup() and Loop()
int MASTER_BPM = 69;
int displayNum = 69;
int presetBPM = 0;
unsigned long presetTime = 0;
unsigned long presetTimeMillis = 0;
int beatSyncWidth = 3;
DISP_FONT displayFont = GREEN;

#include <StateMachine.h>

StateMachine machine = StateMachine();

State* S0 = machine.addState([](){
  if(machine.executeOnce){
    //Serial.println("State 0: Execute Once");
  }
});;

/* STATE MACHINE FLOW
 *  
 *                 =0==========
 *                 = POWER ON =
 *                 ============
 *                       |
 *                      \|/
 *                 =1==========   HOLD ACTION    =2==========
 *                 = SHOW BPM =   ----------->   = LIVE BPM =
 *                 ============                  ============
 *                 == GREEN ===   RSLS ACTION    = GREEN PL =
 *                 ============   <-----------   ============
 *                 
 *                 
 *                 
 *  
 */







// State functions defined below
State* S1 = machine.addState(&state1);
State* S2 = machine.addState(&state2); // Live BPM
State* S3 = machine.addState(&state3); // Pre-set BPM
State* S4 = machine.addState(&state4); // Set time-in
State* S5 = machine.addState(&state5); // Enact Set time-in BPM



// Button and timing helper variables
bool state_pending;
bool debounce_flag;
uint16_t lastButtonState;
unsigned long lastButtonPress = 0;

int beatState = 0;
unsigned long previousMillis = 0;
unsigned long displayBlipMillis = 0;
unsigned long stateCancelMillis = 0;
unsigned long playPauseMillis = 0;

bool playPAUSE = 1; // 1 = play; 0 = PAUSE
bool playPAUSE_help = 0;
bool lastStatePlayPause = 1;

int TEMPO_LED_help = 0;

// Configure the 10 OUTPUT pins used to interface with ICM7218C: D2-D9, mode, write
//  Use NO_PIN if pin is not connected to Arduino
ICM7218C beatDISP(pin_D4, pin_D5, pin_D3, pin_D2, NO_PIN, pin_D10, pin_D9, pin_D6, pin_D7, pin_D8);


//Set up Button reading
int buttonPoll() {
  uint16_t pinput = 0xFF;

  int h = 0;
  for(int k = 0; k < 2; k++) {
    for(int j = 0; j < 2; j++) {
      for(int i = 0; i < 2; i++) {
        digitalWrite(Sel0, i);
        digitalWrite(Sel1, j);
        digitalWrite(Sel2, k);
        if (!digitalRead(Z)) bitClear(pinput, h);
        h++;
      }
    }
  }
  return pinput;
}


void readButtons() {
  // Read Inputs  
  int buttonState = buttonPoll();
 
  
  //if ( !bitRead(buttonState, 2) )  Serial.println("Did Button 3") ;

  //If input int changes, a button is pressed.
  if( lastButtonState != buttonState ) {
    lastButtonState = buttonState;
    debounce_flag = 1;
    // Remember last button press event
    lastButtonPress = millis();
  }

  // When 50ms have passed since change, the resulting state is
  //  presumed to be the correct input sequence.
  if ( debounce_flag && (millis() - lastButtonPress > 50) ) {
    lastButtonState = buttonState;
    //Serial.println("Button changed!");
    //for (int i = 15; i >= 0; i--) Serial.print(bitRead(lastButtonState, i));
    //Serial.println(" ");
    debounce_flag = 0;
  }
  
}


void togglePlayPause() {
  bool i = bitRead(lastButtonState, 3);
  if ( (lastStatePlayPause != i) && (i == 0) ) {
    playPAUSE = !playPAUSE;
    digitalWrite(PP_LED, playPAUSE);
  }
  lastStatePlayPause = i;
}



// Set up Display functions
void clearDisplay() {
  beatDISP.print("        ");
}


void fullDisplay() {
  beatDISP.print("88888888");
}


// -DIG1- DIG2 DIG3 DIG4 green
// -DIG5- DIG6 DIG7 DIG8 red
void colourDisplay(int s, DISP_FONT c) {
  char* s_out = "         ";
    //int j = 0;
    //Serial.print("int s initiated as: ");  Serial.println(s);
  int i = 0;

  switch(c) {
    case GREEN:
      for (int i = 3; i >= 0; i--) {
        s_out[i] = '0' + (s % 10);
        s_out[8-i] = ' ';
        s /= 10;
      }
    break;

    case RED:
      for (int i = 3; i >= 0; i--) {
        s_out[i] = ' ';
        s_out[8-i] = '0' + (s % 10);
        s /= 10;
      }
    break;

    case YELLOW:
      for (int i = 3; i >= 0; i--) {
        s_out[i] = '0' + (s % 10);
        s_out[8-i] = '0' + (s % 10);
        s /= 10;
      }
    break;
  }
  
  beatDISP.print(s_out);
}


void readEncoder() {
  
  if ( analogRead(ROT_A) > 500 ) { CLK = 1; }
    else { CLK = 0; }
  if ( analogRead(ROT_B) > 500 ) { DT = 1; }
    else { DT = 0; }
  
  // Read the current state of CLK
  currentStateCLK = CLK;
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (DT != currentStateCLK) {
      rotaryMod = -1;
      currentDir ="CCW";
    } else {
      // Encoder is rotating CW so increment
      rotaryMod = 1;
      currentDir ="CW";
    }
  }
  // Remember last CLK state
  lastStateCLK = currentStateCLK;
}




void beatPulse(){
  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();
    
  // Update from BPM
  int beatMillis = 60000 / MASTER_BPM;
  // Convert BPM to PPEN (PO Sync)
  beatMillis /= 2;

  if (!playPAUSE) previousMillis = currentMillis - beatMillis;
  if (beatState == false) {
      if (currentMillis - previousMillis >= beatMillis) {
          // Set up next ON Toggle
          previousMillis += beatMillis;
          // ON Sync Pulse
          beatState = true;
          TEMPO_LED_help++;
      }
  } else {
      if (currentMillis - previousMillis >= beatSyncWidth) {
          // Check if duration exceded, next turn on already set by previous if.
          beatState = false;
      }
  }

  if (playPAUSE) {
    if (TEMPO_LED_help >= 2){
      digitalWrite(TEMPO_LED, !digitalRead(TEMPO_LED) );
      TEMPO_LED_help = 0;
    }
    digitalWrite(LINE_OUT, beatState);
  }
  
}




void setup() {
  /*
  // Initialize Serial
  Serial.begin(9600);
  while (! Serial); // Wait until Serial is ready
    Serial.println("");
    Serial.println("Init READY OWO");
  */

// Setup Pins
  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);
  digitalWrite(PP_LED, HIGH);
  pinMode(PP_LED, OUTPUT);
  
  pinMode(Sel0, OUTPUT);
  pinMode(Sel1, OUTPUT);
  pinMode(Sel2, OUTPUT);
  pinMode(Z, INPUT);
  
  digitalWrite(LINE_OUT, LOW);
  pinMode(LINE_OUT, OUTPUT);

  digitalWrite(TEMPO_LED, HIGH);
  pinMode(TEMPO_LED, OUTPUT);


// End Setup Pins

// Rotary Encoder variable initialization
  if ( analogRead(ROT_A) > 500 ) { CLK = 1; }
    else { CLK = 0; }
  if ( analogRead(ROT_B) > 500 ) { DT = 1; }
    else { DT = 0; }
  lastStateCLK = CLK;

// Button Polling initialization
  state_pending = 0;
  debounce_flag = 0;
  lastButtonState = buttonPoll();


// End variable init

  clearDisplay();


// Transitions defined below
  S0->addTransition(&transitionS0S1,S1);
  S1->addTransition(&transitionS1S2,S2); // Hold Action
  S1->addTransition(&transitionS1S3,S3); // Actuate Rotary
  S1->addTransition(&transitionS1S4p1,S4); // Preset 1
  S1->addTransition(&transitionS1S4p2,S4); // Preset 2
  S1->addTransition(&transitionS1S4p3,S4); // Preset 3
  S2->addTransition(&transitionS2S1,S1); // Release Action
  S3->addTransition(&transitionS3S4,S4); // To Hold Action
  S3->addTransition(&transitionS3S4p1,S4); // Preset 1
  S3->addTransition(&transitionS3S4p2,S4); // Preset 2
  S3->addTransition(&transitionS3S4p3,S4); // Preset 3
  S3->addTransition(&transitionS3S1cx,S1); // Cancel condition
  S4->addTransition(&transitionS4S1,S1); // Release Action Instant
  S4->addTransition(&transitionS4S5,S5); // Release Action Set-time
  S4->addTransition(&transitionS4S1cx,S1); // Cancel condition
  S5->addTransition(&transitionS5S1,S1); // Success Set-time
  S5->addTransition(&transitionS5S1cx,S1); // Cancel condition


  //Serial.println("Setup done");
}


void loop() {
  // Most important function each round.  Will have to make into Hardware
  //   Timer Interrupt later
  //if (playPAUSE) // 1 = play; 0 = PAUSE
    beatPulse();

  // Read Inputs
  readEncoder();
  readButtons();

  togglePlayPause();


  machine.run();

  // Consider repeating beatPulse() here if there are performance issues.
    beatPulse();
  // RED font flashing for PlayPAUSE
  DISP_FONT holdFont = displayFont;
  if (!playPAUSE) { // 1 = play; 0 = PAUSE
    digitalWrite(TEMPO_LED, 1); // 1 should be OFF
    unsigned long currentMillis = millis();
    if ( currentMillis - playPauseMillis >= 500 ) {
      playPauseMillis = currentMillis;
      playPAUSE_help = !playPAUSE_help;
    }
    if (playPAUSE_help) holdFont = RED;
  }

  if (lastButtonState == 0b0000000011111011 ) {
    // Easter Egg
  }
  
  //colourDisplay(displayNum, displayFont);
  colourDisplay(displayNum, holdFont);
}



//=======================================

bool transitionS0S1(){
  //Serial.println("Default S0 to S1 at Setup()");
  return true;
}

//-------------------------
void state1(){
  if(machine.executeOnce){
    //Serial.println("State 1: Execute Once");
    displayFont = GREEN;
  }
}

bool transitionS1S2(){ // Live BPM
  if (lastButtonState == 0b0000000011111110 ) {
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS1S3(){ // Dial in PBM
  if (lastButtonState == 0b0000000011111111 && rotaryMod != 0 ) return true;
  return false;
}

bool transitionS1S4p1(){
  if (lastButtonState == 0b0000000011101111 ){
    presetBPM = 80;
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS1S4p2(){
  if (lastButtonState == 0b0000000011011111 ){
    presetBPM = 120;
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS1S4p3(){
  if (lastButtonState == 0b0000000010111111 ){
    presetBPM = 140;
    rotaryMod = 0;
    return true;
  }
  return false;
}


//-------------------------
void state2(){ // Live BPM
  if(machine.executeOnce){
    //Serial.println("State 2: Execute Once");
  }

  unsigned long currentMillis = millis();
  if (beatState == 1) {
    displayBlipMillis = currentMillis;
  }
  if (currentMillis - displayBlipMillis > 20) {
    displayFont = GREEN;
  } else {
    displayFont = RED;
  }
  
  if (rotaryMod != 0) {
    displayNum += rotaryMod;
    
    // counter limits
    if (displayNum > 300){
      displayNum = 300;
    }
    if (displayNum < 40){
      displayNum = 40;
    }

    MASTER_BPM = displayNum;
    rotaryMod = 0;
    Serial.print(" | displayNum: ");
    Serial.println(displayNum);
  }
}

bool transitionS2S1(){
  if (lastButtonState == 0b0000000011111111 ) return true;
  return false;
}

//------------------------
void state3(){ // Dial Preset BPM
  if(machine.executeOnce){
    //Serial.println("State 3: Execute Once");
    displayFont = RED;
  }

  if (rotaryMod != 0) {
    displayNum += rotaryMod;
    
    // counter limits
    if (displayNum > 300){
      displayNum = 300;
    }
    if (displayNum < 40){
      displayNum = 40;
    }

    presetBPM = displayNum;
    rotaryMod = 0;
    stateCancelMillis = millis();
  }
}

// Add all-else exit condition as well.
bool transitionS3S4(){
  if (lastButtonState == 0b0000000011111110 ){
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS3S1cx(){ // Cancel Condition
  unsigned long currentMillis = millis();
  if (lastButtonState == 0b0000000011111101 || currentMillis - stateCancelMillis >= 20000 ){
    displayNum = MASTER_BPM;
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS3S4p1(){
  if (lastButtonState == 0b0000000011101111 ){
    presetBPM = 80;
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS3S4p2(){
  if (lastButtonState == 0b0000000011011111 ){
    presetBPM = 120;
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS3S4p3(){
  if (lastButtonState == 0b0000000010111111 ){
    presetBPM = 140;
    rotaryMod = 0;
    return true;
  }
  return false;
}


//------------------------
void state4(){ // Dial in set-time-in
  if(machine.executeOnce){
    //Serial.println("State 4: Execute Once - Holding Action");
    displayNum = 0;
    displayFont = YELLOW;
  }

  if (rotaryMod != 0) {
    displayNum += rotaryMod * 5;
    
    // timer limits
    if (displayNum > 900){
      displayNum = 900;
    }
    if (displayNum < 0){
      displayNum = 0;
    }

    presetTime = displayNum * 100;
    rotaryMod = 0;
  }


  
}

bool transitionS4S1(){ // Success Instant
  if (lastButtonState == 0b0000000011111111 && presetTime == 0){
    MASTER_BPM = presetBPM;
    displayNum = MASTER_BPM;
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS4S5(){ // Success Set-time
  if (lastButtonState == 0b0000000011111111 && presetTime != 0){
    presetTime /= abs(presetBPM - MASTER_BPM);
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS4S1cx(){ // Cancel Condition
  if (lastButtonState == 0b0000000011111100 ||
      lastButtonState == 0b0000000011101101 ||
      lastButtonState == 0b0000000011011101 ||
      lastButtonState == 0b0000000010111101) {
    displayNum = MASTER_BPM;
    rotaryMod = 0;
    return true;
  }
  return false;
}

//------------------------
void state5(){ // Enact Set time-in BPM
  if(machine.executeOnce){
    //Serial.println("State 5: Execute Once - Enact Set time-in BPM");
    displayFont = YELLOW;  // Starting on YELLOW means next instruction
                           //  toggles it to GREEN.
  }

  
  unsigned long currentMillis = millis();
  if ( currentMillis - displayBlipMillis >= (60000 / MASTER_BPM) ) {
    displayBlipMillis = currentMillis;
    if (displayFont == GREEN) {
      displayFont = YELLOW;
    } else if (displayFont == YELLOW) {
      displayFont = GREEN;
    }
  }

  if ( currentMillis - presetTimeMillis >= presetTime ) {
    presetTimeMillis = currentMillis;
    int sign_incr;
    if ( presetBPM - MASTER_BPM > 0 ) sign_incr = 1;
    if ( presetBPM - MASTER_BPM < 0 ) sign_incr = -1;
    MASTER_BPM = MASTER_BPM + sign_incr;
  } 
  
  displayNum = MASTER_BPM;
/*  Serial.print("ST displayNum: ");
  Serial.print(displayNum);
  Serial.print(" | presetTime: ");
  Serial.print(presetTime);
  Serial.print(" | presetTimeMillis: ");
  Serial.println(presetTimeMillis); */
  
}

bool transitionS5S1(){ // Success Set-time
  if (presetBPM == MASTER_BPM ) {
    presetBPM = 0;
    presetTime = 0;
    displayNum = MASTER_BPM;
    rotaryMod = 0;
    return true;
  }
  return false;
}

bool transitionS5S1cx(){ // Cancel Condition
  if (lastButtonState == 0b0000000011111101 ) {
    presetBPM = 0;
    presetTime = 0;
    displayNum = MASTER_BPM;
    rotaryMod = 0;
    return true;
  }
  return false;
}
