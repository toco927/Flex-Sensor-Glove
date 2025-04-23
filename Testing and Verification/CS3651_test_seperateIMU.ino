/*
#include <Wire.h>
const int MPU_ADDR = 0x68; 
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  
  Wire.write(0);    
  Wire.endTransmission(true);
  Serial.begin(9600);
}

void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);  // Should be 14 bytes if you want Temp too

  AcX = Wire.read() << 8 | Wire.read();    
  AcY = Wire.read() << 8 | Wire.read();  
  AcZ = Wire.read() << 8 | Wire.read();  
  Tmp = Wire.read() << 8 | Wire.read();  // Optional: temp in raw
  GyX = Wire.read() << 8 | Wire.read();  
  GyY = Wire.read() << 8 | Wire.read();  
  GyZ = Wire.read() << 8 | Wire.read();  

  Serial.print("Accelerometer: ");
  Serial.print("X = "); Serial.print(AcX);
  Serial.print(" | Y = "); Serial.print(AcY);
  Serial.print(" | Z = "); Serial.println(AcZ); 
  
  Serial.print("Gyroscope: ");
  Serial.print("X = "); Serial.print(GyX);
  Serial.print(" | Y = "); Serial.print(GyY);
  Serial.print(" | Z = "); Serial.println(GyZ);
  
  Serial.println(" ");
  delay(40);
}
*/

/* Get all possible data from MPU6050
 * Accelerometer values are given as multiple of the gravity [1g = 9.81 m/s²]
 * Gyro values are given in deg/s
 * Angles are given in degrees
 * Note that X and Y are tilt angles and not pitch/roll.
 *
 * License: MIT
 */

#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

unsigned long timer = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("STARTING!!");
  Wire.begin();
  Serial.println("WIRE BEGAN!!");
  
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(true,true); // gyro and accelero
  Serial.println("Done!\n");
  
}

void loop() {
  mpu.update();

  if(millis() - timer > 100){ // print data every second
    //Serial.print(F("TEMPERATURE: "));Serial.println(mpu.getTemp());
    //Serial.print(F("ACCELERO  X: "));Serial.print(mpu.getAccX());
    //Serial.print("\tY: ");Serial.print(mpu.getAccY());
    //Serial.print("\tZ: ");Serial.println(mpu.getAccZ());
  
    //Serial.print(F("GYRO      X: "));Serial.print(mpu.getGyroX());
    //Serial.print("\tY: ");Serial.print(mpu.getGyroY());
    //Serial.print("\tZ: ");Serial.println(mpu.getGyroZ());
  
   // Serial.print(F("ACC ANGLE X: "));Serial.print(mpu.getAccAngleX());
    //Serial.print("\tY: ");Serial.println(mpu.getAccAngleY());
    
    Serial.print(F("ANGLE     X: "));Serial.print(mpu.getAngleX());
    Serial.print("\tY: ");Serial.println(mpu.getAngleY());
   // Serial.print("\tZ: ");Serial.println(mpu.getAngleZ());
    //Serial.println(F("=====================================================\n"));
    timer = millis();
  }

}

/*
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor
  Serial.println("I2C Scanner");
}

void loop() {
  byte error, address;
  int devices = 0;

  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      devices++;
    }
  }
  
  if (devices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("Scan done\n");

  delay(2000);
}
*/
