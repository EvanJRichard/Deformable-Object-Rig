#include <SPI.h>
#include "stdio.h"
#include "pins_arduino.h"

#define  MISO  0
#define  MOSI  1
#define  SS    2
#define  SCK   4
//#define  P_SENSE_PIN A0

//bool isr_flag = LOW;
uint8_t current_val;
char buf[100];
volatile uint8_t pos;
volatile bool rdy;

void setup() {
  //Serial.begin(9600);      //serial

  // Initialize SPI pins.  I AM SLAVE!
  pinMode(MISO, OUTPUT);

  SPCR |= _BV(SPE);          //Enable SPI as slave
  SPI.setDataMode(SPI_MODE0);//Set data mode to 0 (0:0)
  
  pos = 0;
  rdy = false;
  
  SPI.attachInterrupt();      //Turn on SPI interrupts

  //Serial.println("SPI Init");
  //Serial.println("--------");

}

ISR (SPI_STC_vect)
{
  uint8_t c = SPDR;  // grab byte from SPI Data Register
  //Serial.println(SPDR);
  if (pos < sizeof buf)
  {
    buf [pos++] = c;
    if (c == '\n')
      rdy = true;
      SPItransfer(0xFF);
  }
}

uint8_t SPItransfer(uint8_t data)
{
  unsigned char bit = 0;

  _U_SPI_PORT &= ~_U_SS;                    // Need to manually Lower the SS line first

  for(bit = 0; bit < 8; bit++) {            // Loop through 8 bits 
    if(data & 0x80) _U_SPI_PORT |= _U_DO;   // If bit(7) of "data" is high
    else _U_SPI_PORT &= ~_U_DO;             // if bit(7) of "data" is low
    _U_SPI_PORT |= _U_SCK;                  // Serial Clock Rising Edge 
    data <<= 1;                             // Shift "data" to the left by one bit
    if(_U_SPI_PIN & _U_DI) data |= 0x01;    // If bit of slave data is high
    else data &= ~0x01;                     // if bit of slave data is low
    _U_SPI_PORT &= ~_U_SCK;                 // Serial Clock Falling Edge
  }
  _U_SPI_PORT |= _U_SS;                     // Need to manually Raise the SS line Last

  return data;                              // Returns shifted data in value
}

void loop() {
  if (!digitalRead(SS)) {
    //current_val = analogRead(P_SENSE_PIN);
    if (rdy) {
      //Serial.println (buf);
      pos = 0;
      rdy = false;
    } 
  }
}
