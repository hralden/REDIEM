#include <Arduino.h>
#include <avr/io.h>
#include <Wire.h>

//Flex Sensor Arduino Pins
int flex_1 = 18;
int flex_2 = 19;
int flex_3 = 20;
int flex_4 = 21;
int flex_5 = 10;
int flex_6 = 9;

//EMG arduino pin
int EMG_pin = 6;

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

//EMG Variables
float rawEMG = 0;
float Vout_EMG = 0;


//accelerometer variables
float X, Y, Z;

//Constants
int Vin = 5;
int R0 = 15000;
const int MPU = 0x68; //accelerometer I2C address

//BIOZ Registers
const int BIOZ = 0x0D;
const int BIOZ_PTR = 0xB0;
const int START_FREQ_R1 = 0X82;
const int START_FREQ_R2 = 0X83;
const int START_FREQ_R3 = 0X84;
const int FREQ_INCR_R1 = 0x85;
const int FREQ_INCR_R2 = 0x86;
const int FREQ_INCR_R3 = 0x87;
const int NUM_INCRE_R1 = 0x88;
const int NUM_INCRE_R2 = 0x89;
const int NUM_CYCLES_R1 = 0x8A;
const int NUM_CYCLES_R2 = 0x8B;
const int REAL_DATA_R1 = 0x94;
const int REAL_DATA_R2 = 0x95;
const int IMAGINARY_DATA_R1 = 0x96;
const int IMAGINARY_DATA_R2 = 0x97;
const int CTRL_REG = 0x80;
const int CTRL_REG2 = 0x81;
const int STATUS_REG = 0x8F;

//BIOZ Constants
const float MCLK = 16.76*pow(10,6);
const float start_freq = 50*pow(10,3);

//BIOZ Variables
short real;
short imaginary;
double frequency;
double phase;
double magnitude;
double gain;
double impedance;
double gain_factor;
double bioz;
 



void setup() {
  Serial.begin(115200);
  Wire.begin();
  writeData(MPU, 0x6B, 0x00); //reset accelerometer

  writeData(BIOZ, CTRL_REG, 0x00);
  writeData(BIOZ, CTRL_REG2, 0x10);
  program_bioz_registers();
  

}

void loop() {

  //EMG
  rawEMG = analogRead(EMG_pin);
  Vout_EMG = (rawEMG/ 1024) * 5;
  Serial.print(Vout_EMG * 1000);
  Serial.print(",");

  //Flex Sensors
  R1 = calculate_flex_R(18);
  Serial.print(R1);
  Serial.print(",");
  R2 = calculate_flex_R(19);
  Serial.print(R2);
  Serial.print(",");
  R3 = calculate_flex_R(20);
  Serial.print(R3);
  Serial.print(",");
  R4 = calculate_flex_R(21);
  Serial.print(R4);
  Serial.print(",");
  R5 = calculate_flex_R(10);
  Serial.print(R5);
  Serial.print(",");
  R6 = calculate_flex_R(9);
  Serial.print(R6);
  Serial.print(",");

  //BIOZ
  bioz = getBioz();
  Serial.print(bioz);
  Serial.print(",");

  //Accelerometer
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  X = (Wire.read() << 8 | Wire.read()) / 16384.0; //read X axis data
  Serial.print(X);
  Serial.print(",");
  Y = (Wire.read() << 8 | Wire.read()) / 16384.0; //read Y axis data
  Serial.print(Y);
  Serial.print(",");
  Z = (Wire.read() << 8 | Wire.read()) /16384.0; // read Z axis data
  Serial.println(Z);

  
}

//function to calculate resistance 
int calculate_flex_R(int pin){
  flex = analogRead(pin);
  vout = (flex * Vin) /1024;
  resistance = ((Vin / vout) -1) * R0;
  return resistance;
}

void writeData(int device_address, int register_address, int data){
  Wire.beginTransmission(device_address);
  Wire.write(register_address);
  Wire.write(data);
  Wire.endTransmission(true);
  delay(1);
}

int readData(int device_address, int register_address){
  int data;
  Wire.beginTransmission(device_address);
  Wire.write(BIOZ_PTR);
  Wire.write(register_address);
  Wire.endTransmission(true);
  delay(1);
  Wire.requestFrom(device_address, 1);
  data = Wire.read();
  return data;
}

void program_bioz_registers(){
  //range 1 gain 1
  writeData(BIOZ, CTRL_REG, 0x01);
  //number of settling cycles 250 = 5ms
  writeData(BIOZ, NUM_CYCLES_R1, 0x02);
  writeData(BIOZ, NUM_CYCLES_R2, 0x7D);

  //start frequency
  writeData(BIOZ, START_FREQ_R1, 0x19);
  writeData(BIOZ, START_FREQ_R2, 0x99);
  writeData(BIOZ, START_FREQ_R3, 0x99);

  //Increment by 0
  writeData(BIOZ, FREQ_INCR_R1, 0x00);
  writeData(BIOZ, FREQ_INCR_R2, 0x00);
  writeData(BIOZ, FREQ_INCR_R3, 0x00);

  //number of increments = 0
  writeData(BIOZ, NUM_INCRE_R1, 0x00);
  writeData(BIOZ, NUM_INCRE_R2, 0x00);
  
}

double getBioz(){
  int i = 0;
  program_bioz_registers();
  writeData(BIOZ, CTRL_REG, (readData(BIOZ, CTRL_REG) & 0x07) | 0xB0);
  writeData(BIOZ, CTRL_REG, (readData(BIOZ, CTRL_REG) & 0x07) | 0x10);
  writeData(BIOZ, CTRL_REG, (readData(BIOZ, CTRL_REG) & 0x07) | 0x20);
  while ((readData(BIOZ, STATUS_REG) & 0x07) < 4){
    int flag = readData(BIOZ, STATUS_REG) & 2;
    if (flag == 2){
      real = (readData(BIOZ, REAL_DATA_R1) << 8) | readData(BIOZ, REAL_DATA_R2);
      imaginary = (readData(BIOZ, IMAGINARY_DATA_R1) << 8) | readData(BIOZ, IMAGINARY_DATA_R2);
      magnitude = sqrt(pow(real, 2) + pow(imaginary, 2));
      gain_factor = (1/.22)/19.6;
      impedance = 1/ (gain_factor * magnitude);
      if ((readData(BIOZ, STATUS_REG) & 0x07) < 4){
        writeData(BIOZ, CTRL_REG, (readData(BIOZ, CTRL_REG) & 0x07) | 0x30);
        i++;
      }
    }
  }
  writeData(BIOZ, CTRL_REG, (readData(BIOZ, CTRL_REG) & 0x07) |0xA0);
  return impedance;
}
