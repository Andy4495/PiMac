// iMac G5 Raspberry Pi System Management Code: "PiMac"
//  https://github.com/Andy4495/PiMac
//
// Version 0.1 - 7/11/2017 - A.T. - Initial code
// Version 0.2 - 7/12/2017 - A.T. - Re-organized into generic state-machine code with function pointers
//                                - Added states to wait for button release
//                                - Other clean-up
// Version 1.0 - 4/16/2018 - A.T. - Release to GitLab
//
// Monitor power button to turn power supply on/off
// Control front panel LED (future iterations will have improved LED pulsating effects)
// Monitor POWER_GOOD signal from power supply
// Write simple status messages and time stamps to hardware serial
//

const int POWER_BUTTON = 8;        // Active LOW input
const int POWER_GOOD = 6;          // Active HIGH input
const int PS_ON      = 4;          // Active HIGH output (activates low-side switch to pull ATX pin low)
const int FRONT_LED  = 5;          // Active HIGH output
const int LCD_BRIGHTNESS = 9;      // PWM output, 0-3.3V (not used in current iteration)

typedef void *(*StateFunc)();

// Declare the States here
void *sleep_State();          // Powered down. Waiting for power button.
void *powerUp_State();        // Powered up. Waiting for power button released.
void *powerGood_State();      // Powered up. Waiting for POWER_GOOD signal.
void *running_State();        // Powered up. Waiting for power button or loss of POWER_GOOD signal.
void *shutdown_State();       // Powered down. Waiting for power button released.
//

StateFunc statefunc;  // Initial state
StateFunc Laststatefunc;

unsigned long currentTime;

int buttonPressed = 0;               // Track whether button was pressed the last time through loop()
unsigned long initialButtonTime;     // Track how long the power button has been pressed
const int MIN_BUTTON_TIME = 100;     // Length in time in ms that power button needs to be pressed to turn on/off

unsigned long pwrgoodTimer;         // Track time it takes to detect POWER_GOOD signal
const int PWRGOOD_MAX_TIME = 750;   // ATX supplies should indicate POWER_GOOD within 500 ms

unsigned long dimmerTime;
unsigned long prevDimmerTime = 9999;
int dimmerValue = 0;
int upDown = 1; // Start off incrementing


void setup() {

  Serial.begin(9600);
  currentTime = millis();
  Serial.print("Welcome to PiMac: ");
  Serial.println(currentTime);
  digitalWrite(PS_ON, LOW); // Make sure pin is at inactive level when switched to output mode.
  pinMode(PS_ON, OUTPUT);
  digitalWrite(PS_ON, LOW); // Make sure pin is at inactive level when switched to output mode.
  pinMode(FRONT_LED, OUTPUT);
  //pinMode(LCD_BRIGHTNESS, OUTPUT); Not used at this time. DO NOT ACTIVATE unless 5->3.3 level shifting circuit in place!
  pinMode(POWER_BUTTON, INPUT_PULLUP);
  statefunc = &sleep_State;
  // Debug code
  Laststatefunc = NULL;
  Serial.print("sleep_State: ");
  Serial.println((long) &sleep_State);
  Serial.print("powerUp_State: ");
  Serial.println((long) &powerUp_State);
  Serial.print("powerGood_State: ");
  Serial.println((long) &powerGood_State);
  Serial.print("running_State: ");
  Serial.println((long) &running_State);
  Serial.print("shutdown_State: ");
  Serial.println((long) &shutdown_State);
  //

}

void loop() {
  currentTime = millis();
  // Debug code
  if (Laststatefunc != statefunc) {
    Serial.print("currentTime: ");
    Serial.println(currentTime);
    Serial.print("statefunc: ");
    Serial.println((long) statefunc);
    Laststatefunc = statefunc;
  }
  //
  statefunc = (StateFunc)(*statefunc)();
}



void * sleep_State() {

  StateFunc retval = sleep_State;

  BreatheLED();       // Pulse the LED while in sleep state

  if (digitalRead(POWER_BUTTON) == LOW) { // See how long it has been LOW
    if (buttonPressed == 0) {   // Not previously pressed, so start the counter
      buttonPressed = 1;
      initialButtonTime = currentTime;
      Serial.print("Power ON button press detected: ");
      Serial.println(currentTime);
    }
    else {
      if (currentTime - initialButtonTime > MIN_BUTTON_TIME) {
        buttonPressed = 0;
        initialButtonTime = currentTime;  // Reset the button timer to make sure it is off for a period of time before active again
        digitalWrite(PS_ON, HIGH);    // Turn on power supply
        Serial.print("Power ON button exceeded threshold: ");
        Serial.println(currentTime);
        analogWrite(FRONT_LED, 128);
        retval = powerUp_State;
      }
    }
  }
  else { // Power button HIGH, not pressed) {
    buttonPressed = 0;
  }
  return retval;
}

void * powerUp_State() {

  StateFunc retval = powerUp_State;

  if (digitalRead(POWER_BUTTON) == HIGH) { // See how long it has been released (HIGH)
    if (buttonPressed == 0) {   // Not previously released, so start the counter
      buttonPressed = 1;
      initialButtonTime = currentTime;
      Serial.print("Power ON button release detected: ");
      Serial.println(currentTime);
    }
    else {
      if (currentTime - initialButtonTime > MIN_BUTTON_TIME) {
        buttonPressed = 0;
        analogWrite(FRONT_LED, 192);   // Turn LED on brighter
        Serial.print("Power ON button release exceeded threshold: ");
        Serial.println(currentTime);
        pwrgoodTimer = currentTime;
        retval = powerGood_State;
      }
    }
  }
  else { // Power button is LOW: not released yet
    buttonPressed = 0;
  }

  return retval;
}

void * powerGood_State() {

  if (digitalRead(POWER_GOOD) == HIGH) {
    analogWrite(FRONT_LED, 255);   // Turn on the LED full brightness
    buttonPressed = 0;
    Serial.print("POWER_GOOD detected: ");
    Serial.println(currentTime);
    return running_State;
  }
  if (currentTime - pwrgoodTimer > PWRGOOD_MAX_TIME) {
    buttonPressed = 0;
    initialButtonTime = currentTime;
    analogWrite(FRONT_LED, 0);
    digitalWrite(PS_ON, LOW);
    Serial.print("POWER_GOOD took too long: ");
    Serial.println(currentTime);
    return sleep_State;
  }
  return powerGood_State;
}


void * running_State() {

  StateFunc retval = running_State;

  if (digitalRead(POWER_BUTTON) == LOW) { // See how long it has been LOW
    if (buttonPressed == 0) {   // Not previously pressed, so start the counter
      buttonPressed = 1;
      initialButtonTime = currentTime;
      Serial.print("Power OFF button press detected: ");
      Serial.println(currentTime);
    }
    else {
      if (currentTime - initialButtonTime > MIN_BUTTON_TIME) {
        buttonPressed = 0;
        initialButtonTime = currentTime;  // Reset the button timer to make sure it is off for a period of time before active again
        digitalWrite(PS_ON, LOW);    // Turn off power supply
        Serial.print("Power OFF button exceeded threshold: ");
        Serial.println(currentTime);
        analogWrite(FRONT_LED, 128);  // Dim the LED until button is released
        retval = shutdown_State;
      }
    }
  }
  else { // Power button HIGH, not pressed)
    buttonPressed = 0;
  }
  return retval;
}

void * shutdown_State() {

  StateFunc retval = shutdown_State;

  if (digitalRead(POWER_BUTTON) == HIGH) { // See how long it has been released (HIGH)
    if (buttonPressed == 0) {   // Not previously released, so start the counter
      buttonPressed = 1;
      initialButtonTime = currentTime;
      Serial.print("Power OFF button release detected: ");
      Serial.println(currentTime);
    }
    else {
      if (currentTime - initialButtonTime > MIN_BUTTON_TIME) {
        buttonPressed = 0;
        analogWrite(FRONT_LED, 0);   // Turn LED fully off
        Serial.print("Power OFF button release exceeded threshold: ");
        Serial.println(currentTime);
        retval = sleep_State;
      }
    }
  }
  else { // Power button still LOW: not released yet
    buttonPressed = 0;
  }
  return retval;
}

void BreatheLED() {
  if (currentTime != prevDimmerTime) {
    dimmerTime = currentTime % 100;
    if (dimmerTime == 0) {
      prevDimmerTime = currentTime;
      if (upDown == 1) {
        if (dimmerValue > 145) {
          dimmerValue += 15;
        }
        else {
          dimmerValue += 5;
        }
        if (dimmerValue > 235) {
          dimmerValue = 235;
          upDown = 0;
        }
      }
      else {
        if (dimmerValue > 145) {
          dimmerValue -= 15;
        }
        else {
          dimmerValue -= 5;
        }
        if (dimmerValue < 0) {
          dimmerValue = 0;
          upDown = 1;
        }
      }
      analogWrite(FRONT_LED, dimmerValue);
    }
  }
}
