#include "mbed.h"

SPI ard(PTD2, PTD3, PTD1);
DigitalOut cs(PTD0);

Serial pc(USBTX, USBRX); // tx, rx

int main()
{
    cs = 1;                     //deselect arduino (active LOW)

    ard.format(8,0);           //spi for 8 bit data, mode 0 (0:0)
    ard.frequency(1000000);     //1MHz clock rate
    
    cs = 0;
    
    uint8_t val = ard.write(0x11);
    ard.write(0x00); //dubmy
    
    cs = 1;
    
    pc.printf("-----\n\r");
    pc.printf("%d\n\r",val);
    
    
}