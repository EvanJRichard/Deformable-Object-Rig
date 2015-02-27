#include "mbed.h"

InterruptIn button(PTC6);
I2C i2c(PTE25, PTE24);
 
const char addr = 0x02;

int flag = 0;
 
void send() {
 
    flag = 1;
 
}
 
int main() {
    button.rise(&send);
    printf(" Device Address: 0x%02X\r\n", addr);
    while (1) {
        if (flag) {
            printf("-------------------\r\n");
            char cmd[2];
            cmd[0] = 0x01;
            cmd[1] = 0x02;
            
            i2c.write(addr, &cmd[0], 1);
            printf("Send:       0x%02X     %u\r\n", cmd[0], cmd[0]);
            
            wait(.125);
            
            i2c.write(addr, &cmd[1], 1);
            printf("Send:       0x%02X     %u\r\n", cmd[1], cmd[1]);
            
            wait(.25);
            
            //i2c.write(addr, 0x00, 1);
            cmd[0] = 0x00;
            i2c.read(addr, &cmd[0], 1);
            printf("Recieve:    0x%02X     %u\r\n", cmd[0], cmd[0]);
            
            //wait(.125);
            
            //i2c.write(addr, 0x00, 1);        //write blank to allow read
            cmd[1] = 0x00;      
            i2c.read(addr, &cmd[1], 1);
            printf("Recieve:    0x%02X     %u\r\n", cmd[1], cmd[1]);
            
            wait(.125);
            
            flag = 0;
        }
    }
}