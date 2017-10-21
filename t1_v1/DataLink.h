int startConnection(const char * path);
int llopen(int fd, int type);
int llwrite(int fd, const unsigned char* buffer, int length);
int llread(int fd, unsigned char* buffer);
int llclose (int fd, int type);
int sendPacket(int fd, char * buffer, int length);
int receivePacket(int fd, unsigned char ** buffer, unsigned int * buffSize);
