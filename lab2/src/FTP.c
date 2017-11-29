#include "FTP.h"

static int connectSocket(const char* ip, int port) {
	int sockfd;
	struct sockaddr_in server_addr;

	// server address handling
	bzero((char*) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port); /*server TCP port must be network byte ordered */

	// open an TCP socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// connect to the server
	connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

	return sockfd;
}

int ftpConnect(ftp* ftp, const char* ip, int port) {
	int socketfd;
	char rd[1024];

	socketfd = connectSocket(ip, port));

	ftp->control_socket_fd = socketfd;
	ftp->data_socket_fd = 0;

	ftpRead(ftp, rd, sizeof(rd));

	return 0;
}

int ftpLogin(ftp* ftp, const char* user, const char* password) {
	char sd[1024];

	// username
	sprintf(sd, "USER %s\r\n", user);

	ftpSend(ftp, sd, strlen(sd));

	ftpRead(ftp, sd, sizeof(sd));

	// cleaning buffer
	memset(sd, 0, sizeof(sd));

	// password
	sprintf(sd, "PASS %s\r\n", password);
	ftpSend(ftp, sd, strlen(sd));

	ftpRead(ftp, sd, sizeof(sd));

	return 0;
}

int ftpCWD(ftp* ftp, const char* path) {
	char cwd[1024];

	sprintf(cwd, "CWD %s\r\n", path);
	ftpSend(ftp, cwd, strlen(cwd));

	ftpRead(ftp, cwd, sizeof(cwd));

	return 0;
}

int ftpPasv(ftp* ftp) {
	char pasv[1024] = "PASV\r\n";

	ftpSend(ftp, pasv, strlen(pasv));

	ftpRead(ftp, pasv, sizeof(pasv));

	// starting process information
	int ipPart1, ipPart2, ipPart3, ipPart4;
	int port1, port2;
	sscanf(pasv, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ipPart1,	&ipPart2, &ipPart3, &ipPart4, &port1, &port2);

	// cleaning buffer
	memset(pasv, 0, sizeof(pasv));

	// forming ip
	sprintf(pasv, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4);

	// calculating new port
	int portResult = port1 * 256 + port2;

	printf("IP: %s\n", pasv);
	printf("PORT: %d\n", portResult);

	ftp->data_socket_fd = connectSocket(pasv, portResult);

	return 0;
}

int ftpRetr(ftp* ftp, const char* filename) {
	char retr[1024];

	sprintf(retr, "RETR %s\r\n", filename);
	if (ftpSend(ftp, retr, strlen(retr))) {
		printf("ERROR: Cannot send filename.\n");
		return 1;
	}

	if (ftpRead(ftp, retr, sizeof(retr))) {
		printf("ERROR: None information received.\n");
		return 1;
	}

	return 0;
}

int ftpDownload(ftp* ftp, const char* filename) {
	FILE* file;
	int bytes;

	if (!(file = fopen(filename, "w"))) {
		printf("ERROR: Cannot open file.\n");
		return 1;
	}

	char buf[1024];
	while ((bytes = read(ftp->data_socket_fd, buf, sizeof(buf)))) {
		if (bytes < 0) {
			printf("ERROR: Nothing was received from data socket fd.\n");
			return 1;
		}

		if ((bytes = fwrite(buf, bytes, 1, file)) < 0) {
			printf("ERROR: Cannot write data in file.\n");
			return 1;
		}
	}

	fclose(file);
	close(ftp->data_socket_fd);

	return 0;
}

int ftpDisconnect(ftp* ftp) {
	char disc[1024];

	if (ftpRead(ftp, disc, sizeof(disc))) {
		printf("ERROR: Cannot disconnect account.\n");
		return 1;
	}

	sprintf(disc, "QUIT\r\n");
	if (ftpSend(ftp, disc, strlen(disc))) {
		printf("ERROR: Cannot send QUIT command.\n");
		return 1;
	}

	if (ftp->control_socket_fd)
		close(ftp->control_socket_fd);

	return 0;
}

int ftpSend(ftp* ftp, const char* str, size_t size) {
	int bytes;

	bytes = write(ftp->control_socket_fd, str, size);

	printf("Bytes send: %d\nInfo: %s\n", bytes, str);

	return 0;
}

int ftpRead(ftp* ftp, char* str, size_t size) {
	FILE* fp = fdopen(ftp->control_socket_fd, "r");

	do {
		memset(str, 0, size);
		str = fgets(str, size, fp);
		printf("%s", str);
	} while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

	return 0;
}
