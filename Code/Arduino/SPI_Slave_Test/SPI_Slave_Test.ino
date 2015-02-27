#include <SPI.h>
#include "stdio.h"
#include "pins_arduino.h"

#define P_SENSE_PIN A0

//bool isr_flag = LOW;
uint8_t current_val;
char buf[100];
volatile uint8_t pos;
volatile bool rdy;

void setup() {
  Serial.begin(9600);      //serial

  // Initialize SPI pins.  I AM SLAVE!
  pinMode(MISO, OUTPUT);

  SPCR |= _BV(SPE);          //Enable SPI as slave
  SPI.setDataMode(SPI_MODE0);//Set data mode to 0 (0:0)
  
  pos = 0;
  rdy = false;
  
  SPI.attachInterrupt();      //Turn on SPI interrupts

  Serial.println("SPI Init");
  Serial.println("--------");

}

ISR (SPI_STC_vect)
{
  uint8_t c = SPDR;  // grab byte from SPI Data Register
  Serial.println(SPDR);
  if (pos < sizeof buf)
  {
    buf [pos++] = c;
    if (c == '\n')
      rdy = true;
      SPItransfer(0xFF);
  }
}

uint8_t SPItransfer(uint8_t trans_byte) {
  SPDR = trans_byte;
  while (!(SPSR & (1 << SPIF))) {}
  delay(100);
  return SPDR;
}

void loop() {
  if (!digitalRead(SS)) {
    current_val = analogRead(P_SENSE_PIN);
    if (rdy) {
      Serial.println (buf);
      pos = 0;
      rdy = false;
    } 
  }
}
