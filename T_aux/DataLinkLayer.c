#include "DataLinkLayer.h"


struct termios oldtio,newtio;

int flag=0, count=0;

const unsigned char SET[] = {FLAG, A, C_SET, A^C_SET, FLAG};
const unsigned char UA[] = {FLAG, A, C_UA, A^C_UA, FLAG};
const unsigned char DISC[] = {FLAG, A, C_DISC, A^C_DISC, FLAG};
const unsigned char RR0[] = {FLAG, A, C_RR0, A^C_RR0, FLAG};
const unsigned char RR1[] = {FLAG, A, C_RR1, A^C_RR1, FLAG};
const unsigned char REJ0[] = {FLAG, A, C_REJ0, A^C_REJ0, FLAG};
const unsigned char REJ1[] = {FLAG, A, C_REJ1, A^C_REJ1, FLAG};

volatile static int control = 0;

void alarm_answer(){    // answers alarm
  printf("alarm # %d\n", count);
  flag=1;
  count++;
}

int stateMachineControl(State *state, char ch){
  int ret = -1;

  switch(*state) {
    case START:
      if(ch == FLAG){
        ret = ch;
        *state = FLAG_RCV;
      }
      break;

    case FLAG_RCV:
      if(ch == A){
        ret = ch;
        *state = A_RCV;
      }
      else if(ch == FLAG) ;
      else *state = START;
      break;

    case A_RCV:
      if(ch != FLAG){
        ret = ch;
        *state = C_RCV;
      }
      else if(ch == FLAG)
        *state = FLAG_RCV;
      else
        *state = START;
      break;

    case C_RCV:
      if(ch == (A^C_UA) || ch == (A^C_SET) || ch == (A^C_DISC) ||ch == (A^C_RR0)
      ||ch == (A^C_RR1) || ch == (A^C_REJ0) || ch == (A^C_REJ1) || ch == (A^C0) || ch == (A^C1)){
        ret = ch;
        *state = BCC_OK;
      }
      else if(ch == FLAG)
        *state = FLAG_RCV;
      else
        *state = START;
      break;

    case BCC_OK:
      if(ch == FLAG){			//succeeded receiving command
        ret = ch;
        *state = STOP;
      }
      else if(ch != FLAG){ 	//succeeded receiving data
        ret = ch;
        *state = DATA_RCV;
      }
      break;

    default:
      break;

  }

  return ret;
}

int initiateConnection(const char * path){
  (void) signal(SIGALRM, alarm_answer);

  int fd = open(path, O_RDWR | O_NOCTTY);

  if (fd <0) {printf("ERROR with path"); exit(-1); }

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
    newtio.c_cc[VMIN]     = 0;   /* not blocking read */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    return fd;


}

int llopen(int fd, int type){

	State state = START;
	int i = 0, tries = 0, ret = -1;
	unsigned char ch;


  if(type == TRANSMITTER){
    while(tries < NUM_TRIES) {
    		if(write(fd,SET,sizeof(SET))){		// sends SET frame
    			sleep(1);
    			tries++;
    		}else
				printf("ERROR writing SET");

    		for(i=0; i < sizeof(UA); i++){				// receives UA frame sent by the receiver
    			read(fd, &ch, 1);
    			ret = stateMachineControl(&state, ch);
    		}

    		if(state == STOP){
    			printf("UA received successfully.\n");	// reading was successful and connection was established
    			break;
    		} 
    	}


    	if(tries == NUM_TRIES)						// made the max number of tries
    		return -1;

  }else if(type == RECEIVER){
    while(TRUE) {
     		for(i=0; i < sizeof(SET); i++){			// reads SET frame sent by the transmitter
    			read(fd, &ch, 1);
    			ret = stateMachineControl(&state, ch);
    		}

    		if(state == STOP && ret == C_SET){
    			printf("SET received succesfully\n");
    			break;
    		} 
    	}

    	


    	if(write(fd,UA,sizeof(UA)) < 0){  //sends UA frame
    		printf("Error writing UA.\n");
    		return -1;
    	} else
          printf("UA sent successfully.\n");


    }

    return fd;
}

int llwrite(int fd, const unsigned char* buffer, int length){
    int transfer = TRUE;
    int tries = 0;

    unsigned char* packet;
    unsigned int packetSize;

    unsigned char* rcv_response = malloc(MESSAGE_DATA_MAX_SIZE);
    unsigned int rcv_response_size = MESSAGE_DATA_MAX_SIZE;

    while(transfer){
        if(tries == 0){
            if(tries >= NUM_TRIES){
                printf("llwrite: Message not sent.\n");
                return 0;
            }

            packet = encapsulatePacket(buffer, length);
            packetSize = stuffPacket(&packet, DATA_PACKET_SIZE + length);

			for(unsigned int i = 0; i< sizeof(packet);i++){
				printf("BUFFER[%d] = %x\n", i, packet[i]);
			}

            write(fd, packet, packetSize); //send Packet
			sleep(100);
            tries++;
            if(tries == 1){
                //setalarm
            	printf("llwrite: packet sent\n");
            }
        }


        if(receivePacket(fd, rcv_response, rcv_response_size)){
        	printf("llwrite: packet received\n");

        	rcv_response_size = destuffPacket(&rcv_response, rcv_response_size);
        	printf("llwrite: packet destuffed\n");

        	if((verifyResponseCommand(rcv_response, RR0)  && control) || (verifyResponseCommand(rcv_response, RR1) && !control)){
            	printf("llwrite: received RR command\n");
            	control = !control;
        		  //stopalarm
				      transfer = FALSE;
        	}else if((verifyResponseCommand(rcv_response, REJ0) && control) || (verifyResponseCommand(rcv_response, REJ1) && !control )){
            	printf("llwrite: received REJ command \n");
				      //stopAlarm();
				      tries = 0;
			   }
    	  }
    }

    free(rcv_response);
    //stopalarm

	  printf("llwrite: packet sent & received confirmation packet\n");

    return 1;
}

int llread(int fd, unsigned char* buffer){
  unsigned char* packet = malloc(MESSAGE_DATA_MAX_SIZE);
  unsigned int packetSize = MESSAGE_DATA_MAX_SIZE;

	int received = FALSE;
	while(!received){
		if(receivePacket(fd, packet, packetSize)){
			printf("llread: received a packet\n");

			packetSize = destuffPacket(&packet, packetSize);
			printf("llread: destuffed packet\n");

			if(!verifyDataReceived(packet, packetSize)){
				printf("llread: Packet is not data or has header errors\n");
				return 0;
			} else {
				    if((packet[2] == C0 && !control) || (packet[2] == C1 && control)){ //Received packet successfully, end receiving
					         control = !control;

					         memcpy(buffer, packet, packetSize); //save packet in buffer
					         free(packet);

					         if(control)
						          write(fd,RR1,sizeof(RR1));
					         else
						          write(fd,RR0,sizeof(RR0));

					         received = TRUE;
				     } else {
					          return 0;
				     }
			 }
		 }
	 }

	 //stopalarm

	 return 1;
}

int llclose (int fd, int type){
	State state = START;
	int i = 0, tries = 0, cf = -1;
  int ret;
	unsigned char ch;

	if(type == TRANSMITTER){
		while(tries < NUM_TRIES){
			if(write(fd, DISC, sizeof(DISC)) <= 0){
				tries++;
				continue;
			} else{
				printf("disc sent successfully.\n");
        //break;
			}

			for(i=0; i < sizeof(DISC); i++){
    			read(fd, &ch, 1);
    			ret = stateMachineControl(&state, ch);

    			if(ret != -1)
    				cf = ret;
    		}

    		if(state == STOP && cf == C_DISC){
    			printf("disc received successfully.\n");
    			break;
    		} else return -1;

    		if(write(fd,UA, sizeof(UA)) <= 0){
    			printf("Error writing UA.\n");
    			return -1;
    		}

    	}
	} else if(type == RECEIVER){
   		while(tries < NUM_TRIES) {
     		for(i=0; i < sizeof(DISC); i++){			// reads DISC frame sent by the transmitter
    			read(fd, &ch, 1);
    			ret = stateMachineControl(&state, ch);

    			if(ret != -1)
    				cf = ret;
    		}

    		if(state == STOP && cf == C_DISC){
    			printf("disc receiverd successfully.\n");
    			break;
    		} else tries++;
    	}

    	if(tries == NUM_TRIES)						// exceeded max num of tries
    		return -1;


    	if(write(fd,DISC,sizeof(DISC)) < 0){		// writes frame DISC and checks if it was successful
    		printf("Error writing DISC.\n");
    		return -1;
    	}

		  for(i=0; i < sizeof(UA); i++){
			  read(fd, &ch, 1);
			  ret = stateMachineControl(&state, ch);

			  if(ret != -1)
				  cf = ret;
		   }

		   if(state == STOP && ret == C_UA){
			   printf("UA received succesfully.\n");
		   }

	  }

    if (tcsetattr(fd,TCSANOW, &oldtio)== -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
    return(0);
}

unsigned char getBCC(const unsigned char* buffer, int length) {
	unsigned char bcc = 0;

	for(unsigned int i = 0; i < length; i++)
		bcc ^= buffer[i];

	return bcc;
}

unsigned char * encapsulatePacket(const unsigned char * buffer, int length){
	unsigned char* packet = malloc(DATA_PACKET_SIZE + length);

	packet[0] = FLAG;
	packet[1] = A;

	unsigned char BCC1;
	if(control){
		packet[2] = C1;
		BCC1 = A ^ C1;
	} else {
		packet[2] = C0;
		BCC1 = A ^ C0;
	}

	packet[3] = BCC1;

	memcpy(&packet[4], buffer, length);

  unsigned char BCC2 = getBCC(buffer, length);

	packet[4 + length] = BCC2;
	packet[5 + length] = FLAG;

	return packet;
}

unsigned int stuffPacket(unsigned char** packet, int length){
	unsigned int newPacketSize = length;

	int i;
	for (i = 1; i < length - 1; i++)
		if ((*packet)[i] == FLAG || (*packet)[i] == ESCAPE)
			newPacketSize++;

	*packet = (unsigned char*) realloc(*packet, newPacketSize);

	for (i = 1; i < length - 1; i++) {
		if ((*packet)[i] == FLAG || (*packet)[i] == ESCAPE) {
			memmove(*packet + i + 1, *packet + i, length - i);

			length++;

			(*packet)[i] = ESCAPE;
			(*packet)[i + 1] ^= 0x20;
		}
	}

	return newPacketSize;
}

int receivePacket(int fd, unsigned char * buffer, int buffSize){
	State state = START;
  volatile int received = FALSE;
	int cRcv;
	int length = 0;

  while(!received){
    unsigned char ch;

    if(state != STOP){
      cRcv = read(fd, &ch, 1);
      
    }
printf("CHAR %x\n", ch);
	printf("STATE %d\n", state);
    ch = stateMachineControl(&state,ch);
	printf("STATE %d\n", state);
    if(state == DATA_RCV){
      if (length % buffSize == 0) {
				int factor = length / buffSize + 1;
				buffer = (unsigned char*) realloc(buffer, factor * buffSize);
			}

      state = BCC_OK;
    }

    buffer[length++] = ch;

    if(state == STOP){
      buffer[length] = 0;
      received = TRUE;
    }
  }

	return received;
}

unsigned int destuffPacket(unsigned char** packet, int length){
	int i;
	for (i = 1; i < length - 1; ++i) {
		if ((*packet)[i] == ESCAPE) {
			memmove(*packet + i, *packet + i + 1, length - i - 1);

			length--;

			(*packet)[i] ^= 0x20;
		}
	}

	*packet = (unsigned char*) realloc(*packet, length);

	return length;
}

unsigned int verifyResponseCommand(unsigned char * buffer, const unsigned char * command){
	unsigned char a = buffer[1];
	unsigned char c = buffer[2];
	unsigned char bcc1 = buffer[3];

	if(bcc1 == (a^c) && c == command[2]){
		return TRUE;
	} else
		return FALSE;
}

int verifyDataReceived(unsigned char * buffer, int size){
	unsigned char a = buffer[1];
	unsigned char c = buffer[2];
	unsigned char bcc1 = buffer[3];

	if(bcc1 == (a^c) && (c == C0 || c == C1)){
		unsigned char BCC2Rcvd = getBCC(buffer, size); //bcc2 received
		unsigned char bcc2 = buffer[size - 1];

		if(bcc2 != BCC2Rcvd)
			return FALSE;
	} else if(c != C0 && c != C1){
		return FALSE;
	}

	return TRUE;
}
