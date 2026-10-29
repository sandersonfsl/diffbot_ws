#include <Arduino.h>

// L298N H-Bridge Connection PINs (pinos originais — cruzamento corrigido na fiação)
#define L298N_enA 9   // PWM Motor A (RIGHT)
#define L298N_enB 11  // PWM Motor B (LEFT)
#define L298N_in1 12  // Dir Motor A (RIGHT)
#define L298N_in2 13  // Dir Motor A (RIGHT)
#define L298N_in3 7   // Dir Motor B (LEFT)
#define L298N_in4 8   // Dir Motor B (LEFT)

const int SPEED = 150;
const int TURN_SPEED = 90;

void stop();
void rightMotor(int dir, int spd);
void leftMotor(int dir, int spd);

void setup() {
  Serial.begin(115200);

  pinMode(L298N_enA, OUTPUT);
  pinMode(L298N_enB, OUTPUT);
  pinMode(L298N_in1, OUTPUT);
  pinMode(L298N_in2, OUTPUT);
  pinMode(L298N_in3, OUTPUT);
  pinMode(L298N_in4, OUTPUT);

  stop();
  Serial.println("\n--- DIFFBOT IS READY ---");
  Serial.println("Commands: 'f' FORWARD | 'b' BACKWARD | 's' STOP | 'r' ROTATE CW | 'c' ROTATE CCW");
}

void loop() {
  if (Serial.available() > 0) {
    char chr = Serial.read();

    switch (chr) {
      case 'f': case 'F':
        Serial.println("Moving forward...");
        leftMotor(+1, SPEED); rightMotor(+1, SPEED);
        break;
      case 'b': case 'B':
        Serial.println("Moving backward...");
        leftMotor(-1, SPEED); rightMotor(-1, SPEED);
        break;
      case 's': case 'S':
        Serial.println("Stopping motors.");
        stop();
        break;
      case 'r': case 'R':
        Serial.println("Rotating clockwise...");
        leftMotor(+1, TURN_SPEED); rightMotor(-1, TURN_SPEED);
        break;
      case 'c': case 'C':
        Serial.println("Rotating counter-clockwise...");
        leftMotor(-1, TURN_SPEED); rightMotor(+1, TURN_SPEED);
        break;
    }
  }
}

// RIGHT motor = Motor A — sentido invertido (frente/trás corretos)
void rightMotor(int dir, int spd) {
  if (dir > 0) {
    digitalWrite(L298N_in1, HIGH);
    digitalWrite(L298N_in2, LOW);
  } else if (dir < 0) {
    digitalWrite(L298N_in1, LOW);
    digitalWrite(L298N_in2, HIGH);
  }
  analogWrite(L298N_enA, dir == 0 ? 0 : spd);
}

// LEFT motor = Motor B — sentido invertido (frente/trás corretos)
void leftMotor(int dir, int spd) {
  if (dir > 0) {
    digitalWrite(L298N_in3, HIGH);
    digitalWrite(L298N_in4, LOW);
  } else if (dir < 0) {
    digitalWrite(L298N_in3, LOW);
    digitalWrite(L298N_in4, HIGH);
  }
  analogWrite(L298N_enB, dir == 0 ? 0 : spd);
}

void stop() {
  digitalWrite(L298N_in1, LOW);
  digitalWrite(L298N_in2, LOW);
  analogWrite(L298N_enA, 0);

  digitalWrite(L298N_in3, LOW);
  digitalWrite(L298N_in4, LOW);
  analogWrite(L298N_enB, 0);
}
