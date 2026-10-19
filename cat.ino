#include <Servo.h>

// ---------------- Pins ----------------
const int LID_SERVO_PIN = 10;
const int PAW_SERVO_PIN = 11;

const int TRIG_PIN[2] = {6,8};
const int ECHO_PIN[2] = {7,9};

const int SWITCH_PIN = 16;
// ---------------- Settings ----------------
const int DIST_THRESHOLD_CM = 20;
int ANGRY_SLAM = 1;
// Angles (0..180) - adjust to your mechanism
const int LID_CLOSED = 33; //33
int LID_PREOPEN = 45;   // anticipation position
const int LID_OPEN = 55;      // full open

const int PAW_HIDDEN = 33;
int PAW_PEEK = 70;      // anticipation position
const int PAW_HIT = 112;       // full extend

// First 3 presses: delayed reaction
const int NUM_FAKE_OFF_PRESSES = 3;
const unsigned long FAKE_DELAY_MIN_MS = 300;
const unsigned long FAKE_DELAY_MAX_MS = 1500;

// Motion tuning
const int SERVO_STEP_DEG = 1;
const unsigned long SERVO_STEP_DELAY_MS = 1;

const unsigned long PAW_OUT_HOLD_MS = 350;
const unsigned long AFTER_OPEN_PAUSE_MS = 120;

Servo lidServo;
Servo pawServo;

// ---------------- State ----------------
int pressCount = 0;
bool preOpenActive = false;

// Track where we *think* the servos are (so smooth moves start correctly)
int lidPos = LID_CLOSED;
int pawPos = PAW_HIDDEN;

// ---------------- Helpers ----------------
bool detectPresenceCm(int thresholdCm, unsigned long sampleWindowMs) {
  unsigned long startTime = millis();

  while (millis() - startTime < sampleWindowMs) 
  {
    for(int i=0;i<2;i++)
    {
      // Send trigger pulse
      digitalWrite(TRIG_PIN[i], LOW);
      delayMicroseconds(2);
      digitalWrite(TRIG_PIN[i], HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIG_PIN[i], LOW);

      // Listen for echo (short timeout = less noise)
      unsigned long duration = pulseIn(ECHO_PIN[i], HIGH, 12000UL); 
      // 12000 µs ≈ 200 cm max range

      // Reject invalid readings
      if (duration > 0) {
        int distanceCm = duration / 58;

        if (distanceCm > 0 && distanceCm <= thresholdCm) {
          return true;  // Valid detection inside threshold
        }
      }
      delay(20);
    }
    delay(3); // small pause between pings
    
  }

  return false; // Nothing detected in time window
}


void moveServoSmooth(Servo &s, int &currentPos, int targetPos) {
  targetPos = constrain(targetPos, 0, 180);

  if (currentPos < targetPos) {
    for (int p = currentPos; p <= targetPos; p += SERVO_STEP_DEG) {
      s.write(p);
      currentPos = p;
      delay(SERVO_STEP_DELAY_MS);
    }
  } else if (currentPos > targetPos) {
    for (int p = currentPos; p >= targetPos; p -= SERVO_STEP_DEG) {
      s.write(p);
      currentPos = p;
      delay(SERVO_STEP_DELAY_MS);
    }
  }
}

void angrySlam(Servo& s, int amount){
  for(int i=0;i<amount;i++){
    s.write(LID_OPEN);
    delay(200);
    s.write(LID_CLOSED-1);
    delay(200);
  }
}

void setPreOpen(bool enable) {
  if (enable && !preOpenActive) {
    // Anticipation: lid cracks open and paw peeks
    moveServoSmooth(lidServo, lidPos, LID_PREOPEN);
    moveServoSmooth(pawServo, pawPos, PAW_PEEK);
    preOpenActive = true;
  } 
  else if (!enable && preOpenActive) {
    // Return to closed only if we're not in the middle of action
    moveServoSmooth(pawServo, pawPos, PAW_HIDDEN);
    delay(300);
    moveServoSmooth(lidServo, lidPos, LID_CLOSED);
    preOpenActive = false;
  }
}

void doCatAction() {
  // Full sequence: open -> paw hit -> retract -> close
  moveServoSmooth(lidServo, lidPos, LID_OPEN);
  delay(AFTER_OPEN_PAUSE_MS);

  moveServoSmooth(pawServo, pawPos, PAW_HIT);
  delay(PAW_OUT_HOLD_MS);

  moveServoSmooth(pawServo, pawPos, PAW_HIDDEN);
  delay(300);
  moveServoSmooth(lidServo, lidPos, LID_CLOSED);

  preOpenActive = false; // action ends closed
}

bool keyPressed() {
  if (Serial.available() > 0) {
    char c = (char)Serial.read();
    // ignore newline/carriage return if your terminal sends them
    if (c == '\n' || c == '\r') return false;
    return true;
  }
  return false;
}

bool switchTurnedOn() {
  static unsigned long lastChange = 0;
  static bool stableState = HIGH;

  bool reading = digitalRead(SWITCH_PIN);

  if (reading != stableState) {
    lastChange = millis();
    stableState = reading;
  }

  if (millis() - lastChange > 50) {
    return stableState == LOW;
  }

  return false;
}



// ---------------- Setup / Loop ----------------
void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 2; i++) {
  pinMode(TRIG_PIN[i], OUTPUT);
  pinMode(ECHO_PIN[i], INPUT);
}
  lidServo.attach(LID_SERVO_PIN);
  pawServo.attach(PAW_SERVO_PIN);

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  // Start positions
  lidServo.write(LID_CLOSED);
  pawServo.write(PAW_HIDDEN);
  lidPos = LID_CLOSED;
  pawPos = PAW_HIDDEN;

  randomSeed(analogRead(A0)); // for fake delay randomness (leave A0 floating or use a noise source)

  Serial.println("Cat-in-a-box ready.");
  Serial.println("Press any key in Serial Monitor/terminal to trigger.");
}
int ok=0;
int detection=0;
void loop() {
  // After the first 3 presses, use ultrasonic to "anticipate"
  if (pressCount>=7 && PAW_PEEK==70){
    PAW_PEEK=100;
  }
  if (pressCount >= NUM_FAKE_OFF_PRESSES && ok==0) {
    if(pressCount%3!=0 || pressCount==NUM_FAKE_OFF_PRESSES){
      if (detectPresenceCm(DIST_THRESHOLD_CM, 40)) {
        setPreOpen(true);
        detection=0;
      } else {
        detection++;
        if(detection>=5){
          setPreOpen(false);
          detection=0;
        }
      }
    }
    else if(ok==0){
        ok=1;
        setPreOpen(false);
        ANGRY_SLAM++;
        
    }
  }

  // Keyboard (serial) press triggers action
  //if (keyPressed()) {
    //pressCount++;
  //Switch logic
  if(switchTurnedOn())
  {
    pressCount++;
    // First 3 presses: delayed reaction
    if (pressCount <= NUM_FAKE_OFF_PRESSES) {
      unsigned long fakeDelay = random(FAKE_DELAY_MIN_MS, FAKE_DELAY_MAX_MS + 1);
      delay(fakeDelay);
    }
    if(ok==1){
      angrySlam(lidServo,ANGRY_SLAM);
      unsigned long fakeDelay = random(FAKE_DELAY_MIN_MS, FAKE_DELAY_MAX_MS + 1);
      delay(fakeDelay);
      ok=0;
    }
    // If we were pre-open, we keep that as the starting position and go to full action.
    doCatAction();
  }

  delay(100); // small loop delay to reduce sensor spam
}
