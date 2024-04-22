/*
** clientb.c --
*/

#include <stdio.h>
#include <sys/poll.h>
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

// Function to send a request to the server to download a file asynchronously
void send_file_async(int sockfd, const char *filename)
{
	printf("\nsend_file_async--->%s\n", filename);
	char request[MAXDATASIZE];
	sprintf(request, "GET %s\n\n", filename);

	printf("request:  --%s--\n", request);
	if (send(sockfd, request, strlen(request), 0) == -1)
	{
		printf("no good");
		perror("send");
	}
}

void save_to_file(const char *filename, const char *data, const char *order)
{
	// Extract the file name without the ".." prefix
	const char *file_name_without_prefix = filename + 3;
	printf("file to write to is: %s", file_name_without_prefix);

	FILE *file = fopen(file_name_without_prefix, order); // Open file for appending in text mode
	if (file == NULL)
	{
		perror("Error opening file");
		exit(1);
	}
	printf("dtata is - %s\n", data);
	size_t data_len = strlen(data);
	size_t bytes_written = fwrite(data, sizeof(char), data_len, file);
	if (bytes_written != data_len)
	{
		perror("Error writing to file");
		fclose(file);
		exit(1);
	}

	fclose(file);
}

// main
int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	// max size for buffer
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	// need to in terminal right host name and use host name
	//  example : ./client jodogo-VirtualBox

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

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	char command[MAXDATASIZE];
	char accumulated_input[MAXDATASIZE * 10]; // Accumulate multiple lines of input

	accumulated_input[0] = '\0'; // Initialize accumulated input as an empty string

	char *current_command;
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
			}
			else
			{
				printf("No input to send.\n");
			}
			break; // Exit the loop when the user presses Enter twice
		}

		strcat(accumulated_input, command);
		strcat(accumulated_input, "\n");
	}

	char *tokens[2]; // Array to store the split strings // GET/POST somthing
	char *token;

	// Split the string using space as delimiter
	token = strtok(accumulated_input, " ");
	int i = 0;
	// Store the tokens into the array
	while (token != NULL && i < 2)
	{
		tokens[i++] = token;
		token = strtok(NULL, " ");
	}
	tokens[1] = strtok(tokens[1], "\n");
	if (strcmp(tokens[0], "GET") == 0)
	{
		if (strstr(tokens[1], ".list") != NULL)
		{ // GET f.list
			strcat(current_command, "GETL");
		}
		else
		{
			strcat(current_command, "GET");
		}
	}
	else if (strcmp(tokens[0], "POST") == 0)
	{
		strcat(current_command, "POST");
	}
	else
	{
		strcat(current_command, "ERRO");
	}
	accumulated_input[0] = '\0';

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
	{
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received: %s\n", buf);

	// send ack
	if (send(sockfd, "ACK", 3, 0) == -1)
	{
		perror("send");
		exit(1);
	}

	// pollfd for downloading multiple files
	struct pollfd fds1[1];
	fds1[0].fd = sockfd;
	fds1[0].events = POLLIN;
	char *name_file_tokens[MAXDATASIZE]; // Array to store the split strings // GET/POST somthing
	int size;
	while (1)
	{
		int ret = poll(fds1, 1, -1);
		if (ret == -1)
		{
			perror("pool");
			exit(1);
		}
		else if (ret == 0)
		{
			continue;
		}
		else
		{
			if (fds1[0].revents & POLLIN)
			{
				numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
				if (numbytes == -1)
				{
					perror("recv");
					exit(1);
				}
				else if (numbytes == 0)
				{
					break;
				}
				else
				{
					buf[numbytes] = '\0';
					// printf("client: received '%s'\n", buf);

					// Decode From Base64
					char *base64DecodeOutput;
					Base64Decode(buf, &base64DecodeOutput);
					// print output
					//  TODO : save to array [a.txt, b.txt ...] instead of printing
					if (strcmp(current_command, "GETL") == 0)
					{
						// char filesList[MAXDATASIZE];
						char *name_file_tokens[MAXDATASIZE]; // Array to store the split strings // GET/POST somthing
						char *token;

						// Split the string using space as delimiter
						token = strtok(base64DecodeOutput, "\n");
						int i = 0;
						// Store the name_file_tokens into the array
						while (token != NULL && i < MAXDATASIZE)
						{
							name_file_tokens[i++] = token;
							token = strtok(NULL, "\n");
						}

						size = i;
					}
					else if (strcmp(current_command, "GET") == 0)
					{
						save_to_file(tokens[1], base64DecodeOutput, "a");
					}
					else
					{
						printf("Output: %s\n", base64DecodeOutput);
					}
				}
			}
		}
	}

	if (strcmp(current_command, "GETL") == 0)
	{
		struct pollfd fds[size];
		for (int j = 0; j < size; j++)
		{
			printf("line %d:  %s \n", j, name_file_tokens[j]);
		}
		// Create a new socket for each file
		int file_sockets[size];
		for (int j = 0; j < size; j++)
		{

			if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
			{
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
				return 1;
			}

			// loop through all the results and connect to the first we can
			for (p = servinfo; p != NULL; p = p->ai_next)
			{
				if ((file_sockets[j] = socket(p->ai_family, p->ai_socktype,
											  p->ai_protocol)) == -1)
				{
					perror("client: socket");
					continue;
				}

				if (connect(file_sockets[j], p->ai_addr, p->ai_addrlen) == -1)
				{
					perror("client: connect");
					close(file_sockets[j]);
					continue;
				}

				break;
			}

			if (p == NULL)
			{
				fprintf(stderr, "client: failed to connect\n");
				return 2;
			}

			inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
			printf("client: connecting to %s\n", s);

			// freeaddrinfo(servinfo); // all done with this structure

			send_file_async(file_sockets[j], name_file_tokens[j]);

			if ((numbytes = recv(file_sockets[j], buf, MAXDATASIZE - 1, 0)) == -1)
			{
				perror("recv");
				exit(1);
			}

			buf[numbytes] = '\0';

			printf("client: received: %s\n", buf);

			// send ack
			if (send(file_sockets[j], "ACK", 3, 0) == -1)
			{
				perror("send");
				exit(1);
			}
			// Add the socket to the fds array
			fds[j].fd = file_sockets[j];
			fds[j].events = POLLIN;
		}

		int count = 0;
		// Wait for events on all sockets

		while (1)
		{

			int ret = poll(fds, size, -1);
			if (ret == -1)
			{
				perror("poll");
				exit(1);
			}

			// Handle events on sockets
			for (int i = 0; i < size; i++)
			{
				if (fds[i].revents & POLLIN)
				{
					// Receive data from the server
					if ((numbytes = recv(fds[i].fd, buf, MAXDATASIZE - 1, 0)) == -1)
					{
						perror("recv");
						close(fds[i].fd);
						exit(1);
					}
					else if (numbytes == 0)
					{
						// Connection closed
						printf("Connection closed for file %s\n", name_file_tokens[i]);
						close(fds[i].fd);
						count++;
						break;
					}
					buf[numbytes] = '\0';

					// Decode data from base64 and save to file
					char *decoded_data;
					Base64Decode(buf, &decoded_data);
					printf("decoded_data, %s\n", decoded_data);
					// Save to file (implement this function)

					save_to_file(name_file_tokens[i], decoded_data, "a");
				}
			}

			if (count == size)
			{
				break;
			}
		}
	}

	close(sockfd);

	return 0;
}