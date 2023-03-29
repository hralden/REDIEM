#include <Arduino.h>

//Flex Sensor Arduino Pins
int flex_1 = 18;
int flex_2 = 19;
int flex_3 = 20;
int flex_4 = 21;
int flex_5 = 10;
int flex_6 = 9;

//Flex Resistor Values for Arduino Printing
//remove when only bluetooth
int R1 = 0;
int R2 = 0;
int R3 = 0;
int R4 = 0;
int R5 = 0;
int R6 = 0;

//Flex Variables
int flex = 0;
byte flex_byte =0;
int vout = 0;
int resistance;


//Constants
int Vin = 5;
int R0 = 15000;

void setup() {
  // put your setup code here, to run once:

}

void loop() {

  //Flex Sensors
  R1 = calculate_flex_R(18);
  Serial.print(R1);
  Serial.print(,);
  R2 = calculate_flex_R(19);
  Serial.print(R2);
  Serial.print(,);
  R3 = calculate_flex_R(20);
  Serial.print(R3);
  Serial.print(,);
  R4 = calculate_flex_R(21);
  Serial.print(R4);
  Serial.print(,);
  R5 = calculate_flex_R(10);
  Serial.print(R5);
  Serial.print(,);
  R6 = calculate_flex_R(9);
  Serial.println(R6);


}

int calculate_flex_R(int pin){
  flex = analogRead(pin);
  vout = (flex * Vin) /1024;
  resistance = ((Vin / vout) -1) * R0;
  return resistance;
  
}
