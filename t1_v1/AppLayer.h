#include <stdio.h>
#include <termios.h>

int sendFile(const char* path, int file_fd, char *file_name);
int receiveFile(const char* path);

int sendControlPacket(int fd, int file_size, char* file_name, int control_field);
int receiveControlPacket(unsigned char * buffer, int control_field, unsigned int * file_size, char ** file_name);

int sendDataPacket(int file_size, int file_fd, int fd);
int receiveDataPacket(unsigned char* buffer, int file_fd);

void printProgressBar(float current, float total);
void showInitialInformation(int mode);
void printStatistics(int mode);
