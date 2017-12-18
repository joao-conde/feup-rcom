#include "URL.h"
#include "FTP.h"

void printUsage(char* argv0) {
	printf("\nLogin Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n",	argv0);
	printf("Anonymous Usage: %s ftp://<host>/<url-path>\n\n", argv0);
}

int main(int argc, char** argv){

/*

 In order to connect to FTP service and download file we must:
	1-proccess the URL (given as argument): retrieve useful information from the URL and find out the host IP
	2-process FTP client:
		-connect to FTP
		-login in FTP (or anonymous)
		-change directory
		-set passive mode
		-begin transmitting
		-file download
		-disconnect from FTP
*/


	if(argc != 2){
		printf("Wrong number of arguments\n");
		printUsage(*argv);
		return 1;
	}


	//url process
	url url;
	initURL(&url);

	if(parseURL(&url,argv[1]) != 0){
		perror("Error parsing URL\n");
		return 1;
	}

	//get hostname IP
	getIpByHost(&url);

	//process FTP client
	ftp ftp;

	//connect
	if(ftpConnect(&ftp, url.ip, url.port) != 0){
		perror("Error connecting to FTP\n");
		return 1;
	}

	//user and pw
	const char* user = strlen(url.user) ? url.user : "anonymous";
	char* password;
	if (strlen(url.password)) {
		password = url.password;
	} else {
		char buf[100];
		printf("You are now entering in anonymous mode.\n");
		printf("Please insert your college email as password: ");
		while (strlen(fgets(buf, 100, stdin)) < 14)
			printf("\nIncorrect input, please try again: ");
		password = (char*) malloc(strlen(buf));
		strncat(password, buf, strlen(buf) - 1);
	}


	//login
	if(ftpLogin(&ftp,user,password) != 0){
		perror("Error logging in ftp\n");
		return 1;
	}

	//enter passive mode
	if(ftpPasv(&ftp) != 0){
		perror("Error entering PASV mode\n");
		return 1;
	}

	//change directory
	if(ftpCWD(&ftp,url.path) != 0){
		perror("Error changing directory\n");
		return 1;
	}

	//begin transmitting
	if(ftpRetr(&ftp,url.filename)!= 0){
		perror("Error retrieving file\n");
		return 1;
	}

	//download filename
	if(ftpDownload(&ftp,url.filename)!= 0){
		perror("Error downloading file\n");
		return 1;
	}

	//disconnect from FTP
	if(ftpDisconnect(&ftp)!= 0){
		perror("Error disconnecting from ftp\n");
		return 1;
	}

	return 0;
}
