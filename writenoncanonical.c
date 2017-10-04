/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <flags.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255], aux_buf[255];
    int i, sum = 0, speed = 0, counter;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


    //send SET to receiver



    for (i = 0; i < 255; i++) {
      buf[i] = '0';
    }

	gets(&buf);



	for(counter = 0; counter < strlen(buf); counter++){
		if(buf[counter] == '\0')
			break;
	}

    res = write(fd,buf,counter);
    printf("%d bytes written\n", res);

    int aux;
    aux = read(fd, aux_buf, 255);
    if(!strcmp(buf, aux_buf))
      printf("equal!\n");
    else
      printf("ERROR\n");



  /*
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar
    o indicado no gui�o
  */




	sleep(1);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}

int frread(int fd, unsigned char* buf, int maxlen){
  int n= 0;
  int ch;
  while(1){
    if((ch = read(fd, buf + n, 1)) <= 0)
      return ch; //error...
    if(n==0 && buf[n] != FRFLAG)
      continue;
    if(n == 1 && buf[n] == FRFLAG)
      continue;
    n++;
    if(buf[n-1] != FRFLAG && n == maxlen){
      n = 0;
      continue;
    }

    if(buf[n-1] == FRFLAG && n > 2){
      processframe(buf,n);
      return n;
    }
  }
}

char* create_frame(char* type){

  if(!strcmp(type, 'SET'))
    printf("WUT\n");
}
