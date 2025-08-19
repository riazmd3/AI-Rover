#define MOTOR_1_PIN_1    14
#define MOTOR_1_PIN_2    15
#define MOTOR_2_PIN_1    13
#define MOTOR_2_PIN_2    12

void setup() {
  Serial.begin(115200);
  
  // Initialize motor pins
  pinMode(MOTOR_1_PIN_1, OUTPUT);
  pinMode(MOTOR_1_PIN_2, OUTPUT);
  pinMode(MOTOR_2_PIN_1, OUTPUT);
  pinMode(MOTOR_2_PIN_2, OUTPUT);
  
  // Start with all motors off
  stopMotors();
  
  Serial.println("Motor Test Starting...");
  delay(2000);
}

void loop() {
  Serial.println("Moving FORWARD for 3 seconds");
  moveForward();
  delay(3000);
  
  Serial.println("Moving LEFT for 3 seconds");
  moveLeft();
  delay(3000);
  
  Serial.println("Moving BACKWARD for 3 seconds");
  moveBackward();
  delay(3000);
  
  Serial.println("Moving RIGHT for 3 seconds");
  moveRight();
  delay(3000);
  
  Serial.println("Stopping for 2 seconds");
  stopMotors();
  delay(2000);
}

void moveForward() {
  digitalWrite(MOTOR_1_PIN_1, 1);
  digitalWrite(MOTOR_1_PIN_2, 0);
  digitalWrite(MOTOR_2_PIN_1, 1);
  digitalWrite(MOTOR_2_PIN_2, 0);
}

void moveBackward() {
  digitalWrite(MOTOR_1_PIN_1, 0);
  digitalWrite(MOTOR_1_PIN_2, 1);
  digitalWrite(MOTOR_2_PIN_1, 0);
  digitalWrite(MOTOR_2_PIN_2, 1);
}

void moveLeft() {
  digitalWrite(MOTOR_1_PIN_1, 0);
  digitalWrite(MOTOR_1_PIN_2, 1);
  digitalWrite(MOTOR_2_PIN_1, 1);
  digitalWrite(MOTOR_2_PIN_2, 0);
}

void moveRight() {
  digitalWrite(MOTOR_1_PIN_1, 1);
  digitalWrite(MOTOR_1_PIN_2, 0);
  digitalWrite(MOTOR_2_PIN_1, 0);
  digitalWrite(MOTOR_2_PIN_2, 1);
}

void stopMotors() {
  digitalWrite(MOTOR_1_PIN_1, 0);
  digitalWrite(MOTOR_1_PIN_2, 0);
  digitalWrite(MOTOR_2_PIN_1, 0);
  digitalWrite(MOTOR_2_PIN_2, 0);
}