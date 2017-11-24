#include "url.h"

void initURL(url* url){
  memset(url->user, 0, sizeof(url_content));
	memset(url->password, 0, sizeof(url_content));
	memset(url->host, 0, sizeof(url_content));
	memset(url->path, 0, sizeof(url_content));
	memset(url->filename, 0, sizeof(url_content));
	url->port = 21;
}

const char* regExpression =
		"ftp://([([A-Za-z0-9])*:([A-Za-z0-9])*@])*([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

const char* regExprAnony = "ftp://([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

int parseURL(url* url, const char* urlStr){

}
