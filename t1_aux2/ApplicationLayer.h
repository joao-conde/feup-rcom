#ifndef APPLICATIONLAYER_H_
#define APPLICATIONLAYER_H_



int sendFile(const char * path, char *filename);
int receiveFile(const char * path);
int sendControlPackage(int fd, const int C, char* fileSize, char* fileName);
int receiveControlPackage(int fd, int* controlPackageType, int* fileLength, char** fileName);
int sendDataPackage(int fd, int N, const char* buf, int length);
int receiveDataPackage(int fd, int* N, char** buf, int* length);

#endif
