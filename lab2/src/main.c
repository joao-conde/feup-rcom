#include "url.h"


int main(int argc, char** argv){

  if(argc != 2){
    printf("WRONG NUMBER OF ARGUMENTS\n");
    printUsage(argv[0]);
  }

  	///////////// URL PROCESS /////////////
    url url;
    initURL(&url);


    // start parsing argv[1] to URL components
    if (parseURL(&url, argv[1]))
      return -1;
}


void printUsage(char* argv0) {
	printf("\nUsage1 Normal: %s ftp://[<user>:<password>@]<host>/<url-path>\n",	argv0);
	printf("Usage2 Anonymous: %s ftp://<host>/<url-path>\n\n", argv0);
}
