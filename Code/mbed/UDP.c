//mbed FRDM K64F communication over ethernet
//by Evan Richard

//inclusions 
#include "mbed.h"
#include "EthernetInterface.h"
#include "UDPSocket.h"
#include "Socket.h"

//definitions
#define UDP_FRAME_SIZE 256

//declarations
EthernetInterface eth; //our connection object
UDPSocket commSocket; //our socket object
uint8_t g_in_buffer[UDP_FRAME_SIZE] = {0};
uint8_t g_out_buffer[UDP_FRAME_SIZE] = {0};
uint16_t objects[11] = {0}; //the array of the 12 16-bit object values
//declarations continued - be wary of the names of these pins
DigitalOut D2(PTB9);  //S3
DigitalOut D3(PTA1); // S2
DigitalOut D4(PTB23); // S1
DigitalOut D5(PTA2); //S0
DigitalOut CS(PTC2); //mux input AKA chip select
DigitalOut D7(PTC3); //mux enable
SPI spi(D8, D9, D10); // mosi, miso, sclk


/*****COMMUNICATIONS PROTOCOL*****
LITTLE ENDIAN
first 4 bytes buffer[0-3]: SOURCE IP as unsigned 32bit integer
next 4 bytes buffer[4-7]: SEQUENCE COUNT as unsigned 32bit integer
    -> sequences messages
next 4 bytes buffer[8-11]: COMMAND as unsigned 32bit int
next 1 byte buffer[12]: # bytes remaining in the frame that are arguments
next buffer[12] bytes buffer[13-(12+buffer[12])]: arguments
remainder: padding
*****END COMMUNICATION PROTOCOL*****/


//UDP socket event handler c/o Shobhit Kukreti 
//http://mbed.org/forum/helloworld/topic/1689/
//this will be called whenever there is an available message to be read.
/*
void onUDPSocketEvent(UDPSocketEvent e) {
     if ( e == UDPSOCKET_READABLE ) {
         commSocket.receiveFrom(destination, g_in_buffer, sizeof(g_in_buffer)); //host, buffer, length
         bufferCallback(g_in_buffer);
    }
}*/

//read the currently-selected ADC and return the 16 bit response
uint16_t readADC(){
    CS = 0; //make sure ADC CS is asserted
    uint8_t hiByte, loByte;
    spi.write(0x38); //AD7788 command: read data register
    hiByte = spi.write(0x00); //clock 16 bits out
    loByte = spi.write(0x00); //of the AD7788
    uint16_t response;
    response = (hiByte << 8);
    response &= loByte;
    CS = 1; //de-assert ADC CS
    return response;
}

//select given mux channel
//NOTE THIS IS DISTINCT FROM SELECTING AN OBJECT
//object number designations are distinct from mux channel numbers!
void muxSelect(unsigned char channel){
    unsigned char S0, S1, S2, S3; //mux selection values
    S0 = (channel << 7); //just the last bit followed by 7 0s
    S1 = (channel << 6); 
    S2 = (channel << 5);
    S3 = (channel << 4);
    
    S0 &= 0x80; //bitwise and operation with 0 b 1000 0000
    S1 &= 0x80;
    S2 &= 0x80;
    S3 &= 0x80;
    
    if S0 == 0x80 {
        D5 = 1; //S0 pin
        }
    else { 
        D5 = 0;
        }
        
    if S1 == 0x80 {
        D4 = 1; //S1 pin
        }
    else { 
        D4 = 0;
        }
        
    if S2 == 0x80 {
        D3 = 1; //S2 pin
        }
    else { 
        D3 = 0;
        }
        
    if S3 == 0x80 {
        D2 = 1; //S3 pin
        }
    else { 
        D2 = 0;
        }
        
}

//Takes in an object selection from 0 to 11 and selects the proper multiplex configuration
//NOTE THAT OBJECT NUMBERS ARE NOT MAPPED DIRECTLY TO MUX OUTPUTS
void objSelect(unsigned char object){    
    if (object < 12) {
        if (object > 5) {
            object += 3; //object 6 is on channel 9, object 7 is on channel 10, and so on
        }
        muxSelect(object);
    }
}

//process buf[0-3] into one uint32_t IP 
uint32_t getIP(uint8_t buf){
    uint32_t result = 0; 
    //little endian therefore buf[0] contains least significant byte
    result |= (buf[3] << 24);
    result |= (buf[2] << 16);
    result |= (buf[1] << 8);
    result |= buf[0];
    return result;
}

//process buf[4-7] into one uint32_t SEQCOUNT
uint32_t getSequenceCount(uint8_t buf){
    uint32_t result = 0; 
    //little endian therefore buf[4] contains least significant byte
    result |= (buf[7] << 24);
    result |= (buf[6] << 16);
    result |= (buf[5] << 8);
    result |= buf[4];
    return result;
}

//process buf[8-11] into one uint32_t COMMAND
uint32_t getCommand(uint8_t buf){
    uint32_t result = 0; 
    //little endian therefore buf[8] contains least significant byte
    result |= (buf[11] << 24);
    result |= (buf[10] << 16);
    result |= (buf[9] << 8);
    result |= buf[8];
    return result;
    
    //see 8/15 notes. hex code commands should "mean" something 
    //0x _  _  _  _ 
    //AD__ read ADC # __, providing both converted value and raw ADC count
    //AD99 read all ADCs
    //CD__ a command to be passed on to an ADC (perhaps to set ADC mode)
    //00__ a command configuring the mbed in some way
    //FFFF software reset
    //anyways these are handled in bufferCallback but redundant comments can be helpful sometimes
    // -- EJR 8/18/2014
}

//process the buffer and act accordingly
void bufferCallback(uint8_t buf){
    uint32_t sourceIP = getIP(buf);
    uint32_t sequenceCount = getSequenceCount(buf);
    uint32_t command = getCommand(buf);
    uint8_t argsLength = buf[12];
    uint8_t response[UDP_FRAME_SIZE] = {0}
    uint8_t respLength = 0; //argsLength for our response
    uint8_t sendToADC = 0x00; //holder for hex code to pass from host to ADC (used for CD__ commands)
    
    //these three bytes are used for binary decomposition
    uint8_t loByte = 0;
    uint16_t tempByte = 0;
    uint8_t hiByte = 0;
    
    
    //security check on sourceIP and sequence Count
        //TODO
        
    
    //process the buffer according to command & argsLength
    //see 8/15 notes. hex code commands should "mean" something 
    //0x   _  _   _  _      _  _   _  _
    //AD__ read ADC # __, providing both converted value and raw ADC count
    //AD99 read all ADCS
    //CD__ a command to be passed on to an ADC (perhaps to set ADC mode)
    //00__ a command configuring the mbed in some way
    //FFFF software reset
    // -- EJR 8/18/2014
    switch (command){
            
        case 0xFFFFFFFF:    //soft reset
            g_in_buffer = {0}; 
            g_out_buffer = {0};
            objects = {0};
            respLength = 0; //no response
            //more resetting things
            break;
        case 0xAD000000:    //return AD requests start with 0xAD, followed by a number; 99 means all
            objSelect(0);
            objects[0] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[0]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[0]
            break;
        case 0xAD010000:
            objSelect(1);
            objects[1] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[1]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[1]
            break;
        case 0xAD020000:
            objSelect(2);
            objects[2] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[2]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[2]
            break;
        case 0xAD030000:
            objSelect(3);
            objects[3] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[3]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8; //down shift for masking
            hiByte = tempByte & 0x0011; //grab hi byte
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[3]
            break; 
        case 0xAD040000:
            objSelect(4);
            objects[4] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[4]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[4]
            break;
        case 0xAD050000:
            objSelect(5);
            objects[5] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[5]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[5] 
            break;
        case 0xAD060000:
            objSelect(6);
            objects[6] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[6]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[6]
            break;
        case 0xAD070000:
            objSelect(7);
            objects[7] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[7]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[7]
            break;
        case 0xAD080000:
            objSelect(8);
            objects[8] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[8]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[8]
            break;
        case 0xAD090000:
            objSelect(9);
            objects[9] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[9]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[9]
            break;
        case 0xAD100000:
            objSelect(10);
            objects[10] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[10]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[10]
            break; 
        case 0xAD110000:
            objSelect(11);
            objects[11] = readADC();
            respLength = 2; //expect a 2 byte response
            tempByte = objects[11]
            loByte = tempByte & 0x0011; //grab lo byte
            tempByte >> 8;
            hiByte = tempByte & 0x0011;
            response[13] = hiByte;
            response[14] = loByte;
            //return objects[11]
            break;
        case 0xAD990000:    //return all ADs
            for (i = 0; i < 12; i++){
                objSelect(i);
                objects[i] = readADC();
            }
            respLength = 22; //expect a 22 byte response (to be changed)
            //return objects
            break;
        
        case 0xCD000000:    //command AD requests start with 0xCD, followed by a number; 99 means all
            objSelect(0);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            //return what the ADC said during to every write
            CS = 1; //de-assert chip select
            break;
        case 0xCD010000:
            objSelect(1);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            //respLength probably 0
            break;
        case 0xCD020000:
            objSelect(2);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            break;
        case 0xCD030000:
            objSelect(3);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            break; 
        case 0xCD040000:
            objSelect(4);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            break;
        case 0xCD050000:
            objSelect(5);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            break;
        case 0xCD060000:
            objSelect(6);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            //return what the ADC said during to every write
            CS = 1; //de-assert chip select
            break;
        case 0xCD070000:
            objSelect(7);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            break;
        case 0xCD080000:
            objSelect(8);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            break;
        case 0xCD090000:
            objSelect(9);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            break;
        case 0xCD100000:
            objSelect(10);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            break; 
        case 0xCD110000:
            objSelect(11);
            CS = 0; //assert chip select
            respLength = argsLength;
            for ( i = 1, i <= argsLength, i++){
                sendToADC = buf[12+i];
                spi.write(sendToADC);
            }// end for
            CS = 1; //de-assert chip select
            //return what the ADC said during to every write
            break;
        case 0xCD990000:    //command all ADs
            respLength = argsLength;
            for (i = 0, i < 12, i++){
                objSelect(i);
                CS = 0; //assert chip select
                for ( i = 1, i <= argsLength, i++){
                    sendToADC = buf[12+i];
                    spi.write(sendToADC);
                }// end for
                CS = 1; //de-assert chip select    
            }
            //return what the ADC said during to every write
            break;
        case 0x00000000:    //mbed configuration commands start with 0x00__; currently none written
            break;    
        default:
            //unknown command; do some unknown command stuff   
    }//end switch(command) statement
    
    
    //somehow get own IP
    //response[0-3] stores own IP
    
    //give sequence count
    //response[4-7] stores sequence count
    //should i add on to the sequence count of the incoming message, or keep my own count?
    
    //no command in response - just echo what the given command was
    for (i = 8, i < 12, i ++){ 
        response[i] = buf[i]; //echo positions 8, 9, 10, 11 in buffer
    }
    response[12] = respLength;
    //arguments of response should have been filled in during the switch statement above
    
    //put the response in the global out buffer for sending
    g_out_buffer = response;
    //possibly sendTo statement to send g_out_buffer
}

int main(){
    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    //Analog Devices AD7788 uses falling edge of SCLK as active edge
    //  (CPOL = 1 CPHA = 0; this is format "2")
    //always double check the format!!
    spi.format(8,2);
    spi.frequency(1000000); //Hz; Max freq for AD7788 seems to be 5MHz
    
    //before beginning ethernet communication,  populate initial object states.
    for (i = 0; i < 12; i++){
        objSelect(i);
        objects[i] = readADC();
    }
    
    //ethernet and UDP socket
    eth.init(); //using no arguments in init -> use DHCP 
    eth.connect();
    commSocket.init();
    Endpoint destination; //where our socket will receiveFrom and sendTo
    //destination.set_address(host,port);
    //commSocket.sendto(destination, g_out_buffer, sizeof(g_out_buffer)); //host, buffer, length
    //commSocket.receiveFrom(destination, g_in_buffer, sizeof(g_in_buffer)); //host, buffer, length
    
    //do stuff.  
    
    while(1){ 
        //maybe do nothing, or update all the objects, as you like.
        /*
        for (i = 0; i < 12; i++){
        objSelect(i);
        objects[i] = readADC();
        }
        */
    }
    
    commSocket.close();
    eth.disconnect();
    return 0;
}