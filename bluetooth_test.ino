#include <SoftwareSerial.h>
SoftwareSerial Bluetooth(0,1);
int count = 0;

void setup() {
  Bluetooth.begin(9600);
  Serial.begin(9600);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  if (Bluetooth.available()){
    while (count < 300){
      Bluetooth.println(count);
      count++;
      delay(1000);
    }
  }

}
