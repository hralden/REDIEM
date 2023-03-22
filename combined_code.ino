#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#define ADDR_PTR 0xB0
#define START_FREQ_R1 0x82
#define START_FREQ_R2 0x83
#define START_FREQ_R3 0x84

#define FREG_INCRE_R1 0x85
#define FREG_INCRE_R2 0x86
#define FREG_INCRE_R3 0x87

#define NUM_INCRE_R1 0x88
#define NUM_INCRE_R2 0x89

#define NUM_SCYCLES_R1 0x8A 
#define NUM_SCYCLES_R2 0x8B 

#define RE_DATA_R1 0x94
#define RE_DATA_R2 0x95

#define IMG_DATA_R1 0x96
#define IMG_DATA_R2 0x97


#define STATUS_REG 0x8F

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif
//addresses & pin assignments
const int MPU = 0x68; // MPU6050 I2C address
const int BIOZ_ADDR = 0x0D;

const int CTRL_REG = 0x80;
const int CTRL_REG2 = 0x81;
int flex_pin1 = 18; //flex1
int flex_pin2 = 19; //flex2
int flex_pin3 =20; //flex3
int flex_pin4 = 21; //flex4
int flex_pin5 =10; //flex3
int flex_pin6 = 9; //flex4
int emg_Pin = 6; //EMG

//variable def
float AccX, AccY, AccZ;
int flex1 = 0;
int flex2 = 0;
int flex3 = 0;
int flex4 = 0;
int bio_z = 0;
int flex5 = 0;
int flex6 = 0;
byte flex1_byte = 0;
byte flex2_byte = 0;
byte flex3_byte = 0;
byte flex4_byte = 0;
byte flex5_byte = 0;
byte flex6_byte = 0;
float rawEMG = 0;
byte rawEMG_byte = 0;
int Vin = 5;
int R0 = 15000;
int R1 = 0;
int R2 = 0;
int R3 = 0;
int R4 = 0;
int R5 = 0;
int R6 = 0;
float Vout_flex1 = 0;
float Vout_flex2 = 0;
float Vout_flex3 = 0;
float Vout_flex4 = 0;
float Vout_flex5 = 0;
float Vout_flex6 = 0;
float Vout_EMG = 0;
short re;
short img;
double freq;
double mag;
double phase;
double gain;
double Impedance;
double GF;
double FFW;
double wt;
double BF;
double tot = 0;
double magcount = 0;
double impcount = 0;
double avgmag;
double totimp = 0;
double avgimp;
  int i=0;
    #define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

void setup(void)
{
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);


  //Disable command echo
  ble.echo(false);


  ble.verbose(false);  

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }


  // Set module to DATA mode
  ble.setMode(BLUEFRUIT_MODE_DATA);
  
  //ACCELEROMETER
  writeData(0x6B, 0x00, MPU);

  //BIO-Z
  writeData(CTRL_REG, 0x0, BIOZ_ADDR);
  writeData(CTRL_REG2, 0x10, BIOZ_ADDR);

  setup_BIOZ();
  
}

void loop(void)
{

//EMG
    rawEMG = analogRead(emg_Pin);
    rawEMG_byte = rawEMG / 4;
    Vout_EMG = (rawEMG /1024)*5;
    ble.print(Vout_EMG*1000);

//FLEX SENSORS
  flex1 = analogRead(flex_pin1);
  flex2 = analogRead(flex_pin2);
  flex3 = analogRead(flex_pin3);
  flex4 = analogRead(flex_pin4);
  flex5 = analogRead(flex_pin5);
  flex6 = analogRead(flex_pin6);
  flex1_byte = flex1 / 4;
  flex2_byte = flex2 / 4;
  flex3_byte = flex3 / 4;
  flex4_byte = flex4 / 4;
  flex5_byte = flex5 / 4;
  flex6_byte = flex6 / 4;
  Vout_flex1 = (flex1 * Vin)/1024.0;
  Vout_flex2 = (flex2 * Vin)/1024.0;
  Vout_flex3 = (flex3 * Vin)/1024.0;
  Vout_flex4 = (flex4 * Vin)/1024.0;
  Vout_flex5 = (flex5 * Vin)/1024.0;
  Vout_flex6 = (flex6 * Vin)/1024.0;
  R1 = ((Vin/Vout_flex1) - 1) * R0;
  R2 = ((Vin/Vout_flex2) - 1) * R0;
  R3 = ((Vin/Vout_flex3) - 1) * R0;
  R4 = ((Vin/Vout_flex4) - 1) * R0;
  R5 = ((Vin/Vout_flex5) - 1) * R0;
  R6 = ((Vin/Vout_flex6) - 1) * R0;
  ble.print(R1);
  ble.print(R2);
  ble.print(R3);
  ble.print(R4);
  ble.print(R5);
  ble.print(R6);

//BIO-Z
bio_z = getImpedance();
ble.print(bio_z);
delay(4);

    
//ACCELEROMETER
    Wire.beginTransmission(MPU);
    Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
    // Send Accelerometer data to host via Bluefruit
    ble.print(AccX);
    ble.print(AccY);
    ble.print(AccZ);

// Echo received data
  while ( ble.available() )
  {
    int c = ble.read();

    Serial.print((char)c);

    // Hex output too, helps w/debugging!
    Serial.print(" [0x");
    if (c <= 0xF) Serial.print(F("0"));
    Serial.print(c, HEX);
    Serial.print("] ");
  }
}

void writeData(int register_address, int data, int device_address){
  Wire.beginTransmission(device_address);
  Wire.write(register_address);
  Wire.write(data);
  Wire.endTransmission(true);
  delay(1);
}

void readData(int device_address, int register_address){
  int data;
  Wire.beginTransmission(device_address);
  Wire.write(ADDR_PTR);
  Wire.write(register_address);
  Wire.endTransmission(true);
  delay(1);
  Wire.requestFrom(device_address,1);

  
  Wire.requestFrom(BIOZ_ADDR,1);

  if (Wire.available() >= 1){
    data = Wire.read();
  }
  else {
    data = -1;
  }

  delay(1);
  return data;
}

void setup_BIOZ(){
  writeData(CTRL_REG, 0x01, BIOZ_ADDR);

  writeData(NUM_SCYCLES_R1, 0x07, BIOZ_ADDR);
  writeData(NUM_SCYCLES_R2, 0xFF, BIOZ_ADDR);
  
}
  
int getImpedance(){
  setup_BIOZ();
    // 1. Standby '10110000' Mask D8-10 of avoid tampering with gains
  writeData(CTRL_REG,(readData(BIOZ_ADDR, CTRL_REG) & 0x07) | 0xB0, BIOZ_ADDR);

  // 2. Initialize sweep
  writeData(CTRL_REG,(readData(BIOZ_ADDR, CTRL_REG) & 0x07) | 0x10, BIOZ_ADDR);

  // 3. Start sweep
  writeData(CTRL_REG,(readData(BIOZ_ADDR, CTRL_REG) & 0x07) | 0x20, BIOZ_ADDR); 

  while((readData(BIOZ_ADDR, STATUS_REG)& 0x07) < 4 ) {  // Check that status reg != 4, sweep not complete
    delay(8); // delay between measurements

    int flag = readData(BIOZ_ADDR, STATUS_REG)& 2;

    if (flag==2) {

      byte R1 = readData(BIOZ_ADDR, RE_DATA_R1);
    
      byte R2 = readData(BIOZ_ADDR, RE_DATA_R2);
      re = (R1 << 8) | R2;

      R1  = readData(BIOZ_ADDR, IMG_DATA_R1);
      R2  = readData(BIOZ_ADDR, IMG_DATA_R2);
      img = (R1 << 8) | R2;

      freq = start_freq + i*incre_freq;
      mag = sqrt(pow(double(re),2)+pow(double(img),2));
                        tot = tot+mag;
                        magcount = magcount+1;
                        GF = (1/.22)/19.06;
                        Impedance = 1/(GF*mag);
                        if (Impedance < 100){
                          impcount = impcount+1;
                          totimp = totimp+Impedance;
                        }             
    
      //Increment frequency
      if((readData(BIOZ_ADDR, STATUS_REG) & 0x07) < 4 ){
        writeData(CTRL_REG,(readData(BIOZ_ADDR, CTRL_REG) & 0x07) | 0x30, BIOZ_ADDR);
        i++;
      }
                        avgmag = tot/magcount;
                        avgimp = totimp/impcount;
                        wt = 118/2.205;
                        FFW = (0.396*(pow(1.54,2)/(avgimp*1000))+0.143*wt+8.399)*1.37*2;
                        BF = ((wt-FFW)/wt)*100;
                        
    }
                        
  }
  return avgimp;
}
