//DURUSflightSafetyCode

#include <Wire.h>

const float APOGEE_THRESHOLD = -0.5;
const float LAUNCH_ACCEL = 15.0;
const unsigned long FAILSAFE_TIME = 30000;

enum FlightState {
  DISARMED,
  IDLE,
  POWERED_ASCENT,
  COAST,
  APOGEE,
  DESCENT,
  LANDED
};

FlightState currentState = DISARMED;

float altitude = 0.0;
float velocity = 0.0;
float acceleration = 0.0;

bool armed = false;
bool drogueDeployed = false;
bool mainDeployed = false;

unsigned long lastTime = 0;
unsigned long launchTime = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(7, INPUT_PULLUP);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
}

void loop() {
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  checkArm();
  readSensors(deltaTime);
  updateFlightState();
  handleDeployments();
  failsafe(currentTime);
  logData();

  delay(50);
}

void checkArm() {
  if (!armed && digitalRead(7) == LOW) {
    armed = true;
    currentState = IDLE;
  }
}

void readSensors(float dt) {
  acceleration = readAcceleration();
  velocity += acceleration * dt;
  altitude += velocity * dt;
}

float readAcceleration() {
  return 0.0;
}

void updateFlightState() {
  switch (currentState) {
    case IDLE:
      if (acceleration > LAUNCH_ACCEL) {
        currentState = POWERED_ASCENT;
        launchTime = millis();
      }
      break;

    case POWERED_ASCENT:
      if (acceleration < 1.0) {
        currentState = COAST;
      }
      break;

    case COAST:
      if (velocity < APOGEE_THRESHOLD) {
        currentState = APOGEE;
      }
      break;

    case APOGEE:
      currentState = DESCENT;
      break;

    case DESCENT:
      if (altitude < 50.0) {
        currentState = LANDED;
      }
      break;

    default:
      break;
  }
}

void handleDeployments() {
  if (currentState == APOGEE && !drogueDeployed) {
    digitalWrite(8, HIGH);
    drogueDeployed = true;
  }

  if (currentState == DESCENT && altitude < 150.0 && !mainDeployed) {
    digitalWrite(9, HIGH);
    mainDeployed = true;
  }
}

void failsafe(unsigned long t) {
  if (armed && launchTime > 0 && t - launchTime > FAILSAFE_TIME) {
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
  }
}

void logData() {
  Serial.print(millis());
  Serial.print(",");
  Serial.print(altitude);
  Serial.print(",");
  Serial.print(velocity);
  Serial.print(",");
  Serial.println(currentState);
}
