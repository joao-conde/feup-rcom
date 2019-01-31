#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "AppLayer.h"

#define RECEIVER 0
#define TRANSMITTER 1

int printUsage(){
    printf("Usage:<port - /dev/ttyS0> <receive = 0 | send = 1 >  <filename>\n");
	return 1;
}

int main(int argc, char** argv) {

	/** check if args are ok **/
    if ( (argc != 4) ||
  	     ( (strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0) ) ||
		 ( atoi(argv[2]) != 0 && atoi(argv[2]) != 1 )
		 ) {
    	printUsage();
    	exit(1);
    }

    /** TRANSMITTER **/
	if(atoi(argv[2]) == TRANSMITTER){
		int file = open(argv[3], O_RDONLY);
		if (file == -1){
			printf("Error opening file.\n");
			return -1;
		}

		if (sendFile(argv[1], file, argv[3]) < 0){
			printf("Could not send file :(\n");
			return -1;
		} else printf("File sent successfully!\n");

	/** RECEIVER **/
	} else if (atoi(argv[2]) == RECEIVER)
		if (receiveFile(argv[1]) < 0){
			printf("File not received :(\n");
			return -1;
		}
		else printf("File received successfully!\n");

	else{
    	printUsage();
    	exit(1);
	}
	return 0;
}
