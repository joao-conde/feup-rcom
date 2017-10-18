#ifndef DATALINK_H_
#define DATALINK_H_

#include "consts.h"


void alarm_answer(); // function that answers the alarm
int stateMachineControl(State *state, char ch);   //checks if a frame is well built and received
int initiateConnection(const char * path);   //starts the connection
int llopen(int fd, int type);
int llwrite(int fd, const unsigned char* buffer, int length);
int llread(int fd, unsigned char* buffer);
int llclose (int fd, int type);
unsigned char getBCC(const unsigned char* buffer, int length);
unsigned char * encapsulatePacket(const unsigned char * buffer, int length);
unsigned int stuffPacket(unsigned char** packet, int length);
int receivePacket(int fd, unsigned char * buffer, int buffSize);
unsigned int destuffPacket(unsigned char** packet, int length);
unsigned int verifyResponseCommand(unsigned char * buffer, const unsigned char * command);
int verifyDataReceived(unsigned char * buffer, int size);

#endif
