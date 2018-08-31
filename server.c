/* A simple server in the internet domain using TCP
Made for COMP30023 Computer Systems
Student: Jonathan Dunne
ID: 836748
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define RESPONSE200 "HTTP/1.0 200 OK\n"
#define RESPONSE404 "HTTP/1.0 404 NOT FOUND\n"

int main(int argc, char **argv)
{
	int sockfd, newsockfd, portno;
	char buffer[256];
	char buffercopy[256];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	int n;
	char rootPath[100];
	char* request;
	char* filePath;
	char* fullPath;
	char* contentType = malloc(sizeof(char) * 50);


	if (argc < 3)
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}

	strcpy(rootPath, argv[2]);



	 /* Create TCP socket */

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{

		perror("ERROR opening socket");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	portno = atoi(argv[1]);

	/* Create address we're going to listen on (given port number)
	 - converted to network byte order & any IP address for
	 this machine */

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);  // store in machine-neutral format

	 /* Bind address to the socket */

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding");
		exit(1);
	}

	/* Listen on socket - means we're ready to accept connections -
	 incoming connection requests will be queued */


	listen(sockfd,5);

	clilen = sizeof(cli_addr);

	while(1){

		/* Accept a connection - block until a connection is ready to
		 be accepted. Get back a new file descriptor to communicate on. */

		newsockfd = accept(	sockfd, (struct sockaddr *) &cli_addr,
							&clilen);

		if (newsockfd < 0)
		{
			perror("ERROR on accept");
			exit(1);
		}

		bzero(buffer,256);

		/* Read characters from the connection,
			then process */

		n = read(newsockfd,buffer,255);

		if (n < 0)
		{
			perror("ERROR reading from socket");
			exit(1);
		}

		// save buffer, as strtok mutates the original variable
		strcpy(buffercopy, buffer);


		// Get full filepath from rootpath and filepath
		request = strtok(buffer, " ");
	 	filePath = strtok(NULL, " ");

		// save rootpath to stop mutation
		char rootPathCopy[100];
		strcpy(rootPathCopy, rootPath);

	 	fullPath = strcat(rootPathCopy, filePath);

		//find the content type from the filepath
		char* temp;
		temp = strtok(filePath, ".");
		temp = strtok(NULL, "");

		if(strcmp(temp, "html") == 0){
			strcpy(contentType, "text/html");
		}
		else if(strcmp(temp, "css") == 0){
			strcpy(contentType, "text/css");
		}
		else if(strcmp(temp, "js") == 0){
			strcpy(contentType, "text/javascript");
		}
		else if(strcmp(temp, "jpg") == 0){
			strcpy(contentType, "image/jpeg");
		}

		// create empty file
		FILE *contentFile;
		unsigned char* toSocketBuffer;
		int fileLength;

		// handle opening of images
		if(strcmp(contentType, "image/jpeg") == 0){
			contentFile = fopen(fullPath, "rb");
		}

		// handle opening of text files
		else{
			contentFile = fopen(fullPath, "r");
		}

		// handling non-null files
		if (contentFile != NULL) {
			fseek(contentFile, 0, SEEK_END);
			fileLength = ftell(contentFile);
			rewind(contentFile);

			toSocketBuffer = (char*) malloc(sizeof(char)*fileLength);
			size_t fileSize = fread(toSocketBuffer, 1, fileLength, contentFile);

			if(fileSize > 0){
				// send 200 response
				write(newsockfd, RESPONSE200, strlen(RESPONSE200));

				// write file content to the socket.
				write(newsockfd, "Content-Type: ", 14);
				write(newsockfd, contentType, strlen(contentType));
				write(newsockfd, "\n\n", 2);
				send(newsockfd, toSocketBuffer, fileSize, 0);
			}
		}

		// handling non-existent files
		else if (contentFile == NULL){
			write(newsockfd, RESPONSE404, strlen(RESPONSE404));
		}

	if (n < 0)
	{
		perror("ERROR writing to socket");
		exit(1);
	}

	// close socket
	close(newsockfd);
}
	close(sockfd);

	return 0;
}
