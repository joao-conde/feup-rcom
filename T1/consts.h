#ifndef CONSTS_H_
#define CONSTS_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

//Macros used in both layers

#define RECEIVER 0 //if it's the receiver
#define TRANSMITTER 1 //if it's the transmitter

//Macros used in DataLinkLayer

#define FALSE 0
#define TRUE 1

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define NUM_TRIES 3 //max tries to transmit

#define MESSAGE_DATA_MAX_SIZE 512

#define CTRL_PKT_SIZE	5
#define DATA_PACKET_SIZE 	6*sizeof(char)

#define FLAG   0x7E
#define A      0x03
#define C0	   0x00
#define C1	   0x40
#define C_SET  0x03
#define C_UA   0x03
#define C_DISC 0x0B
#define C_RR0  0x05 //???????????????? ou 7 ??????
#define C_RR1  0x85
#define C_REJ0 0x01
#define C_REJ1 0x81
#define ESCAPE 0x7D




typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, DATA_RCV, STOP } State;

//Macros used in ApplicationLayer

#define DATA	1
#define STRT   2
#define END     3
#define T_FILE_SIZE 0
#define T_FILE_NAME 1

#define MAX_SIZE 256

#endif
