#include "URL.h"
#include "FTP.h"


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
	parseURL(&url,argv[1]);

	//get hostname IP
	getIpByHost(&url);

	//process FTP client
	ftp ftp;

	//connect
	ftpConnect(&ftp, url.ip, url.port);

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
	ftpLogin(&ftp,user,password);

	//change directory
	ftpCWD(&ftp,url.path);

	//enter passive mode
	ftpPasv(&ftp);

	//begin transmitting
	ftpRetr(&ftp,url.filename);

	//download filename
	ftpDownload(&ftp,url.filename);

	//disconnect from FTP
	ftpDisconnect(&ftp);

	return 0;
}

void printUsage(char* argv0) {
	printf("\nLogin Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n",	argv0);
	printf("Anonymous Usage: %s ftp://<host>/<url-path>\n\n", argv0);
}
