//PreliminaryFlightCode

#include <Wire.h>

const float APOGEE_THRESHOLD = -0.5;
const float LAUNCH_ACCEL = 15.0;

enum FlightState {
  IDLE,
  POWERED_ASCENT,
  COAST,
  APOGEE,
  DESCENT,
  LANDED
};

FlightState currentState = IDLE;

float altitude = 0.0;
float velocity = 0.0;
float acceleration = 0.0;

unsigned long lastTime = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
}

void loop() {
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  readSensors(deltaTime);
  updateFlightState();
  handleDeployments();

  delay(50);
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

    case LANDED:
      break;
  }
}

void handleDeployments() {
  if (currentState == APOGEE) {
    digitalWrite(8, HIGH);
  }

  if (currentState == DESCENT && altitude < 150.0) {
    digitalWrite(9, HIGH);
  }
}
