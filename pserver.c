/*
 * Proxy Server program for Justin Penny 11126992
 * This program fetches web data from requests recieved from pclient.c
 * The data is cached depending on the results of the request
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>

int hostname_to_ip(char * hostname , char* ip);
int fetch_response(char *site, int sockfd);
int read_in(char *file_choice, char array[5][100]);
int write_out(char *file_choice, char array[5][100]);
void initialize(char a1[5][100], char a2[5][100]);
int list_check(char *buffer, char a1[5][100]);

int main(int argc, char *argv[])
{
    // The first 88 lines of this code are all initiliation and setup. My list and blacklist files are read from
    // My arrays that represent those files are initilized and filled and the connection to server.c is estabished.
    #define h_addr h_addr_list[0]
    int sockfd, newsockfd[1], portno, clilen, n, option = 1, response, l;
    struct sockaddr_in serv_addr, cli_addr; 
    char buffer[256]; 
    char ip[100];
    int result, list_full = 0, blist_full = 0, l_count = 0, bl_count = 0;
    char list_array[5][100];
    char blist_array[5][100];

    initialize(list_array, blist_array);

    read_in("list.txt", list_array);
    read_in("blacklist.txt", blist_array);
    
    printf("\nlist.txt Contents:");
    for(int k = 0; k < 5; k++){
        printf("\n%s", list_array[k]);
    }
    
    printf("\n");
    
    printf("\nblacklist.txt Contents:");
    for(int k = 0; k < 5; k++){
        printf("\n%s", blist_array[k]);
    }
    
    printf("\n");
    
	if(argc < 2)
	{

		printf("\nPort number is missing...\n");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); // this sould let me reuse the socket
    
	if (sockfd < 0)
		error(EXIT_FAILURE, 0, "ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		error(EXIT_FAILURE, 0, "ERROR binding");

	printf("\nServer Started and listening to the port %d\n", portno);
    	listen(sockfd, 5);
        
        clilen=sizeof(cli_addr);
        
		sockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
		if (sockfd < 0){
				error(EXIT_FAILURE, 0, "ERROR on accept");
            }
		printf("\nClient is connected...\n");

        // Main while loop of the program
        while(1){

        // I understand that goto: is bad practice, but I couldn't think of a better way to return the server to a listening state
        // after a repeat entry was submitted by the client
        flag:

		bzero(buffer,256);
		n = read(sockfd,buffer,255);

	   	if(n < 0) {
			error(EXIT_FAILURE, 0, "ERROR reading from socket");
        exit(1);
        }
        
        // Checking for logout request, This process will send a terminate message to the client and close the server
        if(strncmp(buffer, "logout", 6)==0){
            printf("Logout request recieved\n");
            n = write(sockfd, "logout from server", strlen("logout from server"));
            close(sockfd);
            return (1);
        }
        
        // Adds a file to the blacklist, omiting blacklist command assumes url will be added to list
        else if(strncmp(buffer, "blacklist", 9)==0){
            printf("\nBlacklist request recieved");
            n = write(sockfd, "blackout confirm from server\nEnter URL to blacklist", strlen("blackout confirm from server\nEnter URL to blacklist"));
            n = read(sockfd,buffer,255);
            
            // if the list isn't full, add the next open spot
            if(blist_full != 1){
                for(l = 0; l < 5; l++){
                    if(strcmp(blist_array[l], "empty") == 0){
                        strcpy(blist_array[l], buffer);
                        break;
                    }
                    blist_full = 1;
                }
            }
        
            // if the list IS full, start replacing entries
            else{
                if(bl_count > 5){
                    bl_count = 0;
                }
                
                strcpy(blist_array[bl_count], buffer);
                bl_count++;
            }
            
            write_out("blacklist.txt", blist_array);
            n = write(sockfd, "\nblackout successful", strlen("blackout successful\n"));
            
        }
        
        
        // The following 2 else ifs check if the entry from the client is already in list or blackist
        else if(list_check(buffer, blist_array) == 1){
             n = write(sockfd, "ERROR: That URL is contained in blacklist.txt", strlen("ERROR: That URL is contained in blacklist.txt"));
             goto flag;
        }
        else if(list_check(buffer, list_array) == 1){
            n = write(sockfd, "ERROR: That URL is contained in list.txt", strlen("ERROR: That URL is contained in list.txt"));
             goto flag;
        }
        
        // If we make it this far, the message from server is not in either list nor is it a logout request
        else
        {
            printf("\nClient has sent: %s\n", buffer);
            
            response = fetch_response(buffer, sockfd);
            printf("\nResponse from site: %d", response);
            
            // If the server response is 200, add the filename to list.txt
            if(response == 1){
                // if the list isn't at 5 URLs, fill in the next empty spot
                if(list_full != 1){
                    for(l = 0; l < 5; l++){
                        if(strcmp(list_array[l], "empty") == 0){
                            strcpy(list_array[l], buffer);
                            break;
                        }
                        list_full = 1;
                    }
                }
                // if the list IS full, start replacing entries
                else{
                    if(l_count > 5){
                        l_count = 0;
                    }
                    // Deleting entries that are removed
                    remove(list_array[l_count]);
                    
                    strcpy(list_array[l_count], buffer);
                    l_count++;
                }
                
                write_out("list.txt", list_array);

            }
		}
    }
        
	return 0;
}









// Function that will convert a URL to an IP address
// Function source code pulled from http://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/ and redesigned to fit this assignment
int hostname_to_ip(char * hostname , char* ip){
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}

// This function sends a GET request for the index.html page of the URL sent by the client
int fetch_response(char* argv, int sockfd) {
     
    char arg[500];
    char request[1000];
    struct hostent *server;
    struct sockaddr_in serveraddr;
    int port = 80;
    strcpy(arg, argv);
    int i, n, result;
    int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    char *check = "HTTP/1.1 200 OK";
     
    if (tcpSocket < 0)
        printf("\nError opening socket");
    else
       // printf("\nSuccessfully opened socket");

    server = gethostbyname(argv);
     
    if (server == NULL)
    {
        printf("\ngethostbyname() failed\n");
    }
    else
    {
        printf("\n%s = ", server->h_name);
        unsigned int j = 0;
        while (server -> h_addr_list[j] != NULL)
        {
            printf("%s", inet_ntoa(*(struct in_addr*)(server -> h_addr_list[j])));
            j++;
        }
    }
     
    printf("\n");
 
    // This nasty chunk is the connection to port 80 and the submission of the GET request to the web server
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(port);
    if (connect(tcpSocket, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        printf("\nError Connecting");
    bzero(request, 1000);
    sprintf(request, "GET /index.html HTTP/1.1\r\nHost: %s\r\n\r\n", argv);
    if (send(tcpSocket, request, strlen(request), 0) < 0)
        printf("\nError with send()");
    bzero(request, 1000);
    recv(tcpSocket, request, 999, 0);
    
    // Checking if the HTTP request returned 200, if so create a file to cache the result to
    if(strstr(request, check) != NULL) {
        char *filename = argv;
        FILE *file = fopen(filename, "w");
        if(NULL == file)
        {
            fprintf(stderr, "Cannot open file: %s\n", filename);
        }
        fprintf (file, "%s", request);
        printf("\nCache successful");
        n = write(sockfd, request, strlen(request));
        fclose(file);
        result = 1;
    
    }
    // If the respnse is not 200, the entry will not be added to list.txt
    else{
    n = write(sockfd, request, strlen(request));
    result = 0;
    }
    
    close(tcpSocket);
    return (result);
}

// Function to read in our list and blist text files into an array
int read_in(char *file_choice, char array[5][100])
{
    char *filename = file_choice;
    FILE *file = fopen(filename, "r");
    if(NULL == file)
    {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return 1;
    }
    size_t buffer_size = 80;
    char *buffer = malloc(buffer_size * sizeof(char));

    // read each line
    int line_number = 0;
    while(-1 != getline(&buffer, &buffer_size, file))
    {
        if(strcmp(buffer, "empty") != 0){
            strcpy(array[line_number], buffer);
            strtok(array[line_number], "\n");
        }
        line_number++;
    }
    fflush(stdout);
    
    // finished
    fclose(file);
    free(buffer);

    return 0;
}

// Writing array to file
int write_out(char *file_choice, char array[5][100])
{
    char *filename = file_choice;
    FILE *file = fopen(filename, "w");
    if(NULL == file)
    {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return 1;
    }

    // write each line of array
    int line_number = 0;
    for(line_number; line_number < 5; line_number++)
    {
        //trying to remove any newline characters before I add mine
        strtok(array[line_number], "\n");
        fprintf (file, "%s\n", array[line_number]);
    }
    
    // finished
    fclose(file);

    return 0;
}

// Prefilling my array so it isn't garbage
void initialize(char a1[5][100], char a2[5][100]){
    for(int i = 0; i <=5; i++){
        strcpy(a1[i], "empty");
    }
    
    for(int i = 0; i <=5; i++){
        strcpy(a2[i], "empty");
    }
}

// Checking my list, 0 if there is no match, 1 if there is a match in the list
int list_check(char *buffer, char a1[5][100]){
    for(int i = 0; i < 5; i++){
        if(strcmp(buffer, a1[i]) == 0){
            return 1;
        }
    }
    return 0;
}
