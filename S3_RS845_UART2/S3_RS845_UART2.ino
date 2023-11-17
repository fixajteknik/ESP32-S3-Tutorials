#include <HardwareSerial.h>

void setup() {
  // put your setup code here, to run once:
  Serial2.begin(9600, SERIAL_8N2, 7, 14);
  Serial.begin(115200);
}

void loop() {

  Serial2.write("READ");
  delay(3000);
  Serial.print("Sıcaklık ve NEM: ");
  while (Serial2.available() > 0) {
    Serial.print(char(Serial2.read()));
  }
  Serial.println(".");
}
