#include <Arduino.h>

// Function Prototype (Required by PlatformIO/C++)
void rightEncoderCallback();

// L298N H-Bridge Connection PINs
#define L298N_enA 9
#define L298N_in2 13
#define L298N_in1 12

#define right_encoder_phaseA 3
#define right_encoder_phaseB 5

// Added 'volatile' for interrupt safety
volatile unsigned int right_encoder_counter = 0;
String right_encoder_sign = "p";
double right_wheel_meas_vel = 0.0;

void setup() {
  pinMode(L298N_enA, OUTPUT);
  pinMode(L298N_in1, OUTPUT);
  pinMode(L298N_in2, OUTPUT);

  digitalWrite(L298N_in1, HIGH);
  digitalWrite(L298N_in2, LOW);

  Serial.begin(115200);

  pinMode(right_encoder_phaseB, INPUT);
  // Now the compiler knows what rightEncoderCallback is
  attachInterrupt(digitalPinToInterrupt(right_encoder_phaseA), rightEncoderCallback, RISING);
}

void loop() {
  // Critical Section: Disable interrupts briefly while reading/resetting counter
  noInterrupts();
  unsigned int temp_counter = right_encoder_counter;
  right_encoder_counter = 0;
  interrupts();

  right_wheel_meas_vel = (10 * temp_counter * (60.0/385.0)) * 0.10472;

  String encoder_read = "r" + right_encoder_sign + String(right_wheel_meas_vel);
  Serial.println(encoder_read);

  analogWrite(L298N_enA, 100);
  delay(100);
}

void rightEncoderCallback()
{
  if(digitalRead(right_encoder_phaseB) == HIGH)
  {
    right_encoder_sign = "p";
  }
  else
  {
    right_encoder_sign = "n";
  }
  right_encoder_counter++;
}
