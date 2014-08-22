#include "mbed.h"

DigitalOut gpo0(PTD0);
DigitalOut gpo1(PTD1);
DigitalOut gpo2(PTD2);
DigitalOut gpo3(PTD3);
DigitalOut led(LED_RED);

int main()
{
    while (true) {
        gpo0 = !gpo0; // toggle pin
        gpo1 = !gpo1; // toggle pin
        gpo2 = !gpo2; // toggle pin
        gpo3 = !gpo3; // toggle pin
        led = !led; // toggle led
        wait(0.2f);
    }
}