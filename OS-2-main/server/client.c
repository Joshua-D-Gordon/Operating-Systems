/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include "../Base64.h"

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

//main 
int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	//max size for buffer
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	//need to in terminal right host name and use host name
	// example : ./client jodogo-VirtualBox

	if (argc != 2)
	{
		fprintf(stderr, "usage: client hostname\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			  s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	char command[MAXDATASIZE];
	char accumulated_input[MAXDATASIZE * 10]; // Accumulate multiple lines of input

	accumulated_input[0] = '\0'; // Initialize accumulated input as an empty string

	printf("Please enter a command (press Enter twice to stop): ");
	while (1)
	{
		fgets(command, MAXDATASIZE, stdin);

		// Remove the newline character from the end of the string
		command[strcspn(command, "\n")] = '\0';

		if (strlen(command) == 0)
		{
			// If accumulated input is not empty, send it
			if (strlen(accumulated_input) > 0)
			{
				if (send(sockfd, accumulated_input, strlen(accumulated_input), 0) == -1)
				{
					perror("send");
					break;
				}
				// Clear accumulated input
				accumulated_input[0] = '\0';
			}
			else
			{
				printf("No input to send.\n");
			}
			break; // Exit the loop when the user presses Enter twice
		}

		// Concatenate the command to accumulated input
		strcat(accumulated_input, command);
		strcat(accumulated_input, "\n");
	}

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
	{
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received: %s\n", buf);

	//send ack
    if(send(sockfd, "ACK", 3, 0) == -1){
        perror("send");
        exit(1);
    }

	//printing the text
	while((numbytes = recv(sockfd, buf, MAXDATASIZE -1, 0))> 0){
		buf[numbytes] = '\0';
		//printf("client: received '%s'\n", buf);

		//Decode From Base64
  		char* base64DecodeOutput;
  		Base64Decode(buf, &base64DecodeOutput);
		//print output
  		printf("Output: %s\n", base64DecodeOutput);
		
	}

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
	{
		perror("recv");
		exit(1);
	}

	close(sockfd);

	return 0;
}
