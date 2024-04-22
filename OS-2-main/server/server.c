/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <fcntl.h>
#include "../Base64.h"

#define PORT "3490" // the port users will be connecting to

#define BACKLOG 10 // how many pending connections queue will hold

#define MAXDATASIZE 100

#define MAX_PATH_LEN 4096

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0)
		;

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// get currrent directory
char *get_current_directory_with_files()
{
	char *cwd = malloc(MAX_PATH_LEN);
	if (!cwd)
	{
		perror("error in maloc max_path_len");
		exit(1);
	}

	if (!getcwd(cwd, MAX_PATH_LEN))
	{
		perror("Error getting current directory");
		exit(1);
	}

	return cwd;
}

// Send file contents over socket
void send_file_contents(int sockfd, char *filename)
{
	printf("filename:  %s\n", filename);
	FILE *filefd = fopen(filename, "r");
	printf("filename:  %s\n", filename);

	if (filefd == NULL)
	{
		perror("--open");
		send(sockfd, "404 FILE NOT FOUND\n\n", 19, 0);
		return;
	}

	// acquire a shared lock on the file for reading
	struct flock fl;
	fl.l_type = F_RDLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;

	// check lock aquired
	if (fcntl(fileno(filefd), F_SETLK, &fl) == -1)
	{
		perror("fcntl");
		fclose(filefd);
		return;
	}
	// send 200 OK
	send(sockfd, "200 OK\n", 7, 0);
	// wait for ack
	char ack[4];
	if (recv(sockfd, ack, 4, 0) == -1)
	{
		perror("recv");
		fclose(filefd);
		return;
	}

	// send file contents
	char buffer[MAXDATASIZE];
	while (fgets(buffer, MAXDATASIZE, filefd) != NULL)
	{
		// encode base64
		char *base64EncodeOutput;
		Base64Encode(buffer, &base64EncodeOutput);
		printf("converting to base64 - %s\n", buffer);
		printf("Sent (base64): %s\n", base64EncodeOutput);

		send(sockfd, base64EncodeOutput, strlen(base64EncodeOutput), 0);
	}

	// relase the file lock
	fl.l_type = F_UNLCK;
	if (fcntl(fileno(filefd), F_SETLK, &fl) == -1)
	{
		perror("fcntl");
		fclose(filefd);
		return;
	}

	fclose(filefd);
}

int fileExist(char *fullpath)
{
	if (access(fullpath, F_OK) != -1)
	{
		printf("file exists");
	}
	else
	{
		printf("file not exitss");
	}
	return access(fullpath, F_OK) != -1;
}

void createdir(char *dirpath)
{
	struct stat st = {0};

	if (stat(dirpath, &st) == -1)
	{
		mkdir(dirpath, 0700);
	}
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	if (argc != 2)
	{
		fprintf(stderr, "server root directory not found\n");
		exit(1);
	}

	// deifning REMOTEPATH where files are stored
	char *REMOTE_PATH = get_current_directory_with_files();
	printf("remote path is: %s\n", REMOTE_PATH);
	int rp_len = strlen(REMOTE_PATH);
	int tkone_len = strlen(argv[1]);
	char folder_path[rp_len + tkone_len + 1];

	strcpy(folder_path, REMOTE_PATH);
	strcat(folder_path, "/");
	strcat(folder_path, argv[1]);
	createdir(folder_path);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
					   sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while (1)
	{ // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
				  get_in_addr((struct sockaddr *)&their_addr),
				  s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork())
		{ // this is the child process

			// ADDED
			char buf[MAXDATASIZE];
			int numbytes;
			if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1)
			{
				perror("recv error");
				exit(1);
			}

			buf[numbytes] = '\0';
			// PRINT WHAT SENT
			if (strcmp(buf, "close") == 0)
			{
				printf("Received close command. Closing connection.\n");
				close(new_fd);
				exit(0); // Exit the child process
			}
			// need to makesubstring of first three - four chars or after " "
			char copy[MAXDATASIZE];
			strcpy(copy, buf);
			printf("\n------%s---------\n", copy);
			char *tokens[2]; // Array to store the split strings // GET/POST somthing
			char *token;

			// Split the string using space as delimiter
			token = strtok(copy, " ");
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
				// for clientb if : GET f.list
				if (strstr(tokens[1], ".list") != NULL)
				{ // GET f.list
					printf("**GET .LIST**\n");

					int rp_len = strlen(REMOTE_PATH);
					int tkone_len = strlen(tokens[1]);

					char fullpath[rp_len + tkone_len + 1];
					// copy to full path remote path "/" tokens[1]
					strcpy(fullpath, REMOTE_PATH);
					strcat(fullpath, "/");
					strcat(fullpath, tokens[1]);
					send_file_contents(new_fd, fullpath);
				}
				else
				{
					printf("**GET**\n");
					// tokens[1] == files/a.txt

					int rp_len = strlen(REMOTE_PATH);
					int tkone_len = strlen(tokens[1]);

					char fullpath[rp_len + tkone_len + 1];
					// copy to full path remote path "/" tokens[1]
					strcpy(fullpath, REMOTE_PATH);
					strcat(fullpath, "/");
					strcat(fullpath, tokens[1]);
					send_file_contents(new_fd, fullpath);
				}
			}
			else if (strcmp(tokens[0], "POST") == 0)
			{
				printf("**POST**\n");

				char *newline_index = strchr(buf, '\n');

				if (newline_index == NULL)
				{
					// If '\n' is not found, return -1 to indicate failure
					return -1;
				}

				// Calculate the index of the first '\n'
				int index = newline_index - buf;

				// Assign the substring after the first '\n'
				char *contents = newline_index + 1;

				printf("%s\n", contents);

				// copy to full path remote path "/" tokens[1]
				int rp_len = strlen(folder_path);
				int tkone_len = strlen(tokens[1]);
				printf("\nfolder_path:   -%s-\n", folder_path);

				char fullpath[rp_len + tkone_len + 1];
				strcpy(fullpath, folder_path);
				strcat(fullpath, "/");
				strcat(fullpath, tokens[1]);
				printf("\nfullpath:   -%s-\n", fullpath);
				// open file for writing
				FILE *file = fopen(fullpath, "w");
				if (file == NULL)
				{
					perror("fopen");
					// handel error
					if (send(new_fd, "500 INTERNAL ERROR\n\n", 7, 0) == -1)
					{
						perror("send");
						exit(1);
					}
					exit(1);
				}

				// lock file with fcntl
				struct flock fl;
				fl.l_type = F_WRLCK; // write lock
				fl.l_whence = SEEK_SET;
				fl.l_start = 0;
				fl.l_len = 0;

				// check for error locking
				if (fcntl(fileno(file), F_SETLK, &fl) == -1)
				{
					perror("fcntl");
					fclose(file);
					exit(1);
				}
				// write to file
				if (fputs(contents, file) == EOF)
				{
					perror("fputs");
					fclose(file);
					exit(1);
				}

				// release the lock
				fl.l_type = F_UNLCK;
				if (fcntl(fileno(file), F_SETLK, &fl) == -1)
				{
					perror("fcntl");
					fclose(file);
					exit(1);
				}

				fclose(file);

				if (send(new_fd, "200 OK\n", 7, 0) == -1)
				{
					perror("send");
					exit(1);
				}
				// if file already exists error 500
				// other errros 500
			}
			else
			{
				printf("**ERROR**\n");
				// send error 500
			}

			close(sockfd); // child doesn't need the listener

			close(new_fd);
			exit(0);
		}
		close(new_fd); // parent doesn't need this
	}

	return 0;
}