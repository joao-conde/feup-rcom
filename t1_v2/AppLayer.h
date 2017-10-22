#include <stdio.h>
#include <termios.h>

int sendFile(const char* path, int file_fd, char *file_name);
int receiveFile(const char* path);

int sendControlPacket(int fd, int file_size, char* file_name, int control_field);
int receiveControlPacket(unsigned char * buffer, int control_field, unsigned int * file_size, char ** file_name);

int sendDataPackage(int fd, int N, const char* buf, int length);
int receiveDataPackage(int fd, int* N, char** buf, int* length);
