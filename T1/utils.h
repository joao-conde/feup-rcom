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

#define RECEIVER 0 //if it's the receiver
#define TRANSMITTER 1 //if it's the transmitter

#define FALSE 0
#define TRUE 1

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define NUM_TRIES 3 //max tries to transmit

const unsigned int MESSAGE_DATA_MAX_SIZE = 512;

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


const unsigned char SET[] = {FLAG, A, C_SET, A^C_SET,FLAG};
const unsigned char UA[] = {FLAG, A, C_UA, A^C_UA, FLAG};
const unsigned char DISC[] = {FLAG, A, C_DISC, A^C_DISC, FLAG};
const unsigned char RR0[] = {FLAG, A, C_RR0, A^C_RR0, FLAG};
const unsigned char RR1[] = {FLAG, A, C_RR1, A^C_RR1, FLAG};
const unsigned char REJ0[] = {FLAG, A, C_REJ0, A^C_REJ0, FLAG};
const unsigned char REJ1[] = {FLAG, A, C_REJ1, A^C_REJ1, FLAG};

typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, DATA_RCV, STOP } State;
