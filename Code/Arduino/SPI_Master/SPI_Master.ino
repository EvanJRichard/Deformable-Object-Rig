#include <SPI.h>

#define MOSI 11//MOSI
#define MISO  12//MISO 
#define SCLK  13//sck
#define CS 10//ss

int response = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting up...");
  digitalWrite(CS,HIGH); //disable device
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCLK,OUTPUT);
  pinMode(CS,OUTPUT);
  
  SPI.begin();
  SPI.setDataMode(SPI_MODE2); //datasheet confirmed: mode 2 or maybe 3
  SPI.setBitOrder(MSBFIRST); //datasheet confirmed
  SPI.setClockDivider(SPI_CLOCK_DIV128); //arbitrary
  Serial.println("SPI and pins set up.");

  while (digitalRead(MISO)){
    Serial.println("waiting for MISO to be pulled low...");
    Serial.println(digitalRead(MISO));
  }
  Serial.println("waiting 5000.");
  delay(5000);
  digitalWrite(CS,LOW);//enable device
  /*
  Serial.println("transferring 0x38 to set AD7788 in continuous-conversion mode...");
   SPI.transfer(0x38);
   Serial.println("0x38 transferred.");*/

}

void loop() {
  // put your main code here, to run repeatedly:
  //delay(1000);
  while (digitalRead(MISO)){
    //Serial.println("waiting for MISO to be pulled low...");
    //Serial.println(digitalRead(MISO));
  }
  SPI.transfer(0x08); //read status register
  SPI.transfer(0x00); //clock in info
  //delay(0); 
  SPI.transfer(0x18); //read mode register
  SPI.transfer(0x00); //clock in info
  //delay(1000);
  //Serial.println("Repsonse:");
  //delay(1000);
  //Serial.println(response);


}


