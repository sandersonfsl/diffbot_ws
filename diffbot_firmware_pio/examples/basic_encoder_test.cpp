#include <Arduino.h>

/*/
RUN the code,open monitor , move motors

Whats is expected :
Rest :
D2=1 D3=0 D4=0 D5=0
Moving Left motor:
D2=1 D3=0 D4=1 D5=0
Moving Right motor:
D2=0 D3=1 D4=1 D5=0
/*/

void setup() {
  Serial.begin(115200);
  for (int p = 2; p <= 8; p++) pinMode(p, INPUT);
}
void loop() {
  Serial.print("D2="); Serial.print(digitalRead(2));
  Serial.print(" D3="); Serial.print(digitalRead(3));
  Serial.print(" D4="); Serial.print(digitalRead(4));
  Serial.print(" D5="); Serial.println(digitalRead(5));
  delay(50);
}
