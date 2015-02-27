#include "TinyWireS.h"                  // wrapper class for I2C slave routines

#define I2C_SLAVE_ADDR   0b0000001      // i2c slave address (2)
#define LED1_PIN         4              // ATtiny Pin 3    green
#define LED2_PIN         1              // ATtiny Pin 6    yellow

uint8_t cmd[2];
uint16_t cmd_combine;
int num_byte = 0;                       // byte order, start at 0

void setup(){
  pinMode(LED1_PIN,OUTPUT);             // for general DEBUG use
  pinMode(LED2_PIN,OUTPUT);             // for verification
  Blink(LED1_PIN,2);                    // show it's alive
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
}

void loop() {
  byte byte_rcvd = 0;
  if (TinyWireS.available()){
    byte_rcvd = TinyWireS.receive();
    cmd[num_byte] = byte_rcvd;
    //cmd &= byte_rcvd << (8*num_byte);
    num_byte ++;
    //cmd_combine = cmd[0] && byte_rcvd << (8*num_byte);
  }
  if (num_byte >= 2) {
    TinyWireS.send(cmd[0]);
    TinyWireS.send(cmd[1]);
    num_byte = 0;
    Blink(LED2_PIN,cmd[0]);
    Blink(LED1_PIN,1);
    Blink(LED2_PIN,cmd[1]);
    Blink(LED1_PIN,1);
  }
  cmd_combine = cmd[0] && cmd[1] << (8*num_byte);
}

void Blink(byte led, uint16_t times){ // poor man's display
  for (uint16_t i=0; i< times; i++){
    digitalWrite(led,HIGH);
    delay (250);
    digitalWrite(led,LOW);
    delay (175);
  }
}
