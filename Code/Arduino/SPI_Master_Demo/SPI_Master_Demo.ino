#include "SPI.h"

#define MOSI 11//MOSI
#define MISO  12//MISO 
#define SCLK  13//sck
#define CS 10//ss

void setup() {
  Serial.begin(9600);
  Serial.println("Starting up...");

  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCLK,OUTPUT);
  pinMode(CS,OUTPUT);
  digitalWrite(CS,HIGH); //disable device
  SPI.begin();
  SPI.setDataMode(SPI_MODE3); //possibly mode2? CPOL = idle high definitely CPHA = falling edge possibly
  SPI.setBitOrder(MSBFIRST); //datasheet confirmed
  SPI.setClockDivider(SPI_CLOCK_DIV128); //arbitrary
  Serial.println("SPI and pins set up.");
  
  while (digitalRead(MISO)){
  //Serial.println("waiting for MISO to be pulled low...");
  //Serial.println(digitalRead(MISO));
  }
  //Serial.println("waiting 5000.");
  //delay(5000);
  //Serial.println("transferring 0x38 to set AD7788 in continuous-conversion mode...");
  SPI.transfer(0x38);
  //Serial.println("0x38 transferred.");
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("Delaying 1000ms, then transferring 0x55 again...");
  //SPI.begin();
  //delay(1000);
  
  SPI.transfer(0x38);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  /*
  while (digitalRead(MISO)){
  Serial.println("waiting for MISO to be pulled low...");
  Serial.println(digitalRead(MISO));
  }
  Serial.println("Value1 read:");
  Serial.print(SPI.transfer(0x00));
  Serial.println("Value2 read:");
  Serial.print(SPI.transfer(0x00));
  */
  
  //SPI.end();
}
