#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "AppLayer.h"
#include "DataLink.h"

#define DATA	1
#define START   2
#define END     3
#define T_FILE_SIZE 0
#define T_FILE_NAME 1

#define TRANSMITTER 1
#define RECEIVER 0

#define MAX_DATA_SIZE 1024

int sendControlPacket(int fd, int file_size, char* file_name, int control_field){
	int p = 0;
	int i = 0;
	unsigned char packet[5 + strlen(file_name) + sizeof(file_size)];

	packet[p++] = control_field;
	packet[p++] = T_FILE_NAME;				// type of filename
	packet[p++] = strlen(file_name);			// length of filename

	for (i = 0; i < strlen(file_name); i++){
		packet[p++] = file_name[i];			// value of filename
	}

	unsigned char size[sizeof(file_size)];

	for(i = 0; i < sizeof(file_size); i++){			// converts int to byte array
		size[i] = (file_size >> 8*i) & 0x00FF;
  	}

	packet[p++] = T_FILE_SIZE;
	packet[p++] = strlen((char *)size);

	for (i = 0; i < strlen((char *)size) ; i++){
		packet[p++] = size[i];				// value of filename
	}

	int packet_size = p;

	if (llwrite(fd, packet, packet_size) < 0){
		printf("llwrite failure :(\n");
		return -1;
	}else printf("llwrite success.\n");


	return 0;
}

int sendDataPacket(int file_fd, int fd) {
	int received = 0;
	int bytes_read = 0;
	char buffer[MAX_DATA_SIZE];
	unsigned char packet[4 + MAX_DATA_SIZE];
	int n_seq = 0;

	while(!received){
    bytes_read = 0; 
		bytes_read = read(file_fd, &buffer, MAX_DATA_SIZE);

		if (bytes_read < 0){
			printf("APPLAYER error reading data :(\n");
			return -1;
		} else if (bytes_read < MAX_DATA_SIZE){
			printf("Reached end of data reading!\n");
			received = 1;
		}

		packet[0] = DATA;
		packet[1] = n_seq % 255;
		packet[2] = (0xFF00 & bytes_read) >> 8;		// SE DER ERRO E POR CAUSA DISTO!!!!!!!!!!!!!!!!!!!!!!!!!
		packet[3] = 0x00FF & bytes_read;

		memcpy(&packet[4], buffer, bytes_read);

		int packet_size = 4 + MAX_DATA_SIZE;

		if(llwrite(fd, packet, packet_size)<0){
			printf("APPLAYER llwrite error.\n");
			return -1;
		} else printf("APPLAYER llwrite success.\n");


		n_seq++;
	}

	return 0;
}

int sendFile(const char* path, int file_fd, char *file_name) {

	int fd = startConnection(path);

	if(llopen(fd, TRANSMITTER) < 0)
		printf("APPLAYER: llopen failure :(\n");
	else printf("APPLAYER: llopen success.\n");

	struct stat stat_buf;

  	if (fstat(file_fd,&stat_buf) < 0){
		perror("fstat ERROR");
		return -1;
  	}

	int file_size = stat_buf.st_size;

	//SEND START PACKET
	if (sendControlPacket(fd, file_size, file_name, START) < 0){
		printf("APPLAYER: Didn't send START packet :(\n");
		return -1;
	} else printf("APPLAYER: Sent START packet.\n");

	//SEND DATA PACKET
	if (sendDataPacket(file_fd, fd) < 0){
		printf("APPLAYER: Didn't send DATA packet :(\n");
		return -1;
	} else printf("APPLAYER: Sent DATA packet.\n");

	//SEND END PACKET
	if (sendControlPacket(fd, file_size, file_name, END) < 0){
		printf("APPLAYER: Didn't send END packet :(\n");
		return -1;
	} else printf("APPLAYER: Sent END packet.\n");

	if(llclose(fd, TRANSMITTER) < 0)
		printf("APPLAYER: llclose failure :(\n");
	else printf("APPLAYER: llclose success.\n");

	return 0;
}

int receiveControlPacket(unsigned char * buffer, int control_field, unsigned int * file_size, char ** file_name){
	int length;
	int i = 0;

	if(buffer[0] != control_field) {
		printf("APPLAYER: Received wrong control field.\n");
		return -1;

	} else if(buffer[0] == START) {
		int pos = 1;

		for(i = 0; i < 2; i++){
			if(buffer[pos] == T_FILE_NAME){
				pos++;
				length = buffer[pos];
				pos++;

				memcpy(*file_name, &buffer[pos], length);
				pos = pos + length;
			} else if(buffer[pos] == T_FILE_SIZE){
				pos++;
				length = buffer[pos];
				pos++;

				unsigned int tmp = 0;
				int j = 0;
				for(j = 0; j < length; j++){
					tmp += (unsigned char)buffer[pos+j] << 8*j;
				}

				*file_size = tmp;
				pos = pos + length;
			} else{
				printf("APPLAYER: Control packet is corrupted.\n");
				return -1;
			}
		}

	} else if(buffer[0] == END) {
		printf("APPLAYER: Received END control packet.\n");

	} else {
		printf("APPLAYER: Critical Error. Impossible state reached.");
		return -1;
	}

	return 0;

}


int receiveDataPacket(unsigned char* buffer, int file_fd) {

	int size = buffer[1]*256 + buffer[2];

	if (write(file_fd, buffer+3, size) < 0){
		printf("APPLAYER: Error writing file part.\n");
		return -1;
	}

	return 0;

}

int receiveFile(const char* path){

	char* file_name = malloc(MAX_DATA_SIZE);
	unsigned int file_size;
	int file_fd = 0;
	int received = 0;

	int fd = startConnection(path);

	if(llopen(fd, RECEIVER) < 0)
		printf("APPLAYER RECEIVER: llopen failure :(\n");
	else printf("APPLAYER RECEIVER: llopen success.\n");

	while(!received){
		unsigned char buffer[MAX_DATA_SIZE];

		if (llread(fd, buffer) < 0)
			printf("APPLAYER: llread failure :(\n");
		else {
			printf("APPLAYER: llread success.\n");

			switch(buffer[0]){
				case 1:
					//RECEIVE DATA PACKET

					if (receiveDataPacket(&buffer[1], file_fd) < 0){
						printf("APPLAYER: Didn't receive DATA packet :(\n");
						return -1;
					} else printf("APPLAYER: Received DATA packet.\n");

					break;

				case 2:
							//RECEIVE START PACKET

					if (receiveControlPacket(buffer, START, &file_size, &file_name) < 0) {
						printf("APPLAYER: Didn't receive START packet :(\n");
						return -1;
					} else printf("APPLAYER: Received START packet.\n");

					printf("FILE SIZE: %d.\n", file_size);
					printf("FILE NAME: %s.\n", file_name);

					file_fd = open(file_name, O_CREAT|O_WRONLY, 0666);
					break;

				case 3:
					//RECEIVE END PACKET

					if (receiveControlPacket(buffer, END, &file_size, &file_name) < 0) {
						printf("APPLAYER: Didn't receive END packet :(\n");
						return -1;
					} else printf("APPLAYER: Received END packet.\n");

					received = 1;
					break;
			}
		}

	}

	free(file_name);
	close(file_fd);

	if(llclose(fd, RECEIVER) < 0){
		printf("APPLAYER: llclose failure :(\n");
		return -1;
	}
	else printf("APPLAYER: llclose success.\n");

	return 0;
}
