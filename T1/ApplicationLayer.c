#include "ApplicationLayer.h"
#include "utils.h"
#include "consts.h"

int sendFile(const char * path, char *filename){

    FILE *file = fopen (filename, "rb"); //opens file

    //checks if the file has been created
    if (!file){
        printf("Error opening file.\n");
        return 0;
    }
    else
    {
        printf("Success opening file.\n");
    }

    int fd = initiateConnection(path);  //start connection
    if(fd <= 0){
    	printf("Cant find a connection.");
    	return 0;
    }

    int check_cnct = llopen(fd, TRANSMITTER);
    if (check_cnct<=0) //checks for a connection error
    {
        return 0;
    }

    char fileSizeBuf[sizeof(int)*3+2];  //frame size

    int fileSize = getFileSize(file); //gets the size of the file

    snprintf(fileSizeBuf, sizeof fileSizeBuf, "%d", fileSize); //puts the file size into fileSizeBuf
    //snprintf redirects the result of printf to a variable


    if(!sendControlPackage(fd, START, fileSizeBuf, filename))  //send Start control package
    {
    	printf("Couldn't send Control Packet.\n");
        return 0;
    }

    //alocar espaco para buffer de arquivo
    char* filebuf = malloc(MAX_SIZE);

	// le pedacos de arquivo
	unsigned int readBytes = 0, writtenBytes = 0, i = 0;

	while ((readBytes = fread(filebuf, sizeof(char), MAX_SIZE, file)) > 0) {
        //createControlPacket(fd, (i++) % 255, fileBuf, readBytes)

		// envia os pedacos dentro do pacote de dados
		if (!sendDataPackage(fd, (i++) % 255, filebuf, readBytes)) {
			free(filebuf);
			return 0;
		}

		// reseta arquivo buffer
		filebuf = memset(filebuf, 0, MAX_SIZE);

		//incrementa numero de bytes escritos
		writtenBytes += readBytes;
	}
	printf("\n");

	free(filebuf);

	if (fclose(file) != 0) {
		printf("ERRO:impossivel fechar o arquivo.");
		printf("\n\n");
		return 0;
	}

	/* END */

	if (!sendControlPackage(fd, END, "0", ""))
    {
        return 0;
    }

	if (!llclose(fd, TRANSMITTER))
    {
        return 0;
	}

	printf("\n");
	printf("Arquivo transferido com sucesso.");
	printf("\n\n");

	return 1;
}
