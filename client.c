/*
 * Client program for Justin Penny 11126992
 * This program connects to a pserver.c program and is able to make requests for IP addresses and is also able to log off.
 */
#include <stdio.h>                      
#include <sys/socket.h>         
#include <netinet/in.h>
#include <netdb.h>
#include <error.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    #define h_addr h_addr_list[0]
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[1000];

	if (argc < 2)
	{
		printf("\nPort number is missing...\n");
		exit(0);
	}

	portno = atoi(argv[1]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error(EXIT_FAILURE, 0, "ERROR opening socket");
	server = gethostbyname("129.120.151.94"); //IP address of server MUST USE THIS OR IT WON'T CONNECT. SERVER MUST BE ON CSE01. CLIENT MUST BE ON CSE02
	//server = gethostbyname("localhost"); //Both in the same machine [IP address 127.0.0.1]
	
	if (server == NULL)
	{
		printf("\nERROR, no such host...\n");
		exit(0);
	}

	//Connecting with the server
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
		error(EXIT_FAILURE, 0, "ERROR connecting the server...");
        
    printf("\nINPUT INSTUCTIONS:\nAll URL entries must begin with 'www'");
    printf("\nTo blacklist a file, type 'blacklist' press enter\nthen submit the URL as normal");
    printf("\n'logout' can be entered to terminate the connection and end both programs\n");

    while(1) {
        //Sending the message to the server
        printf("\nEnter client's message: ");
        bzero(buffer,256);
        scanf("%s", buffer);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
        {
            error(EXIT_FAILURE, 0, "ERROR writing to socket");
        }
        
        //Receiving the message from the server
        bzero(buffer,1000);
        n = read(sockfd, buffer, 1000);
        if (n < 0)
            error(EXIT_FAILURE, 0, "ERROR reading from socket");
        
        // Terminates connection from server after a successful logout request is sent
        if(strncmp(buffer, "logout from server", 18)==0){
            printf("Logged out of server\n");
            return(1);
        }
        else
        {
            printf("Server has sent: %s\n", buffer);
        }
        
    }
	return 0;	
} 
