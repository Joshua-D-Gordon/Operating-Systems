#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT "9034"     // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

// Global variables for socket descriptor and thread synchronization
int sockfd;
pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Function to handle sending data
void *send_thread(void *arg)
{
    char buf[MAXDATASIZE];
    ssize_t numbytes;

    while (1)
    {
        pthread_mutex_lock(&send_mutex); // Lock mutex before sending
        fgets(buf, MAXDATASIZE, stdin);
        numbytes = strlen(buf);
        buf[numbytes] = '\0'; // Null-terminate the string

        if (send(sockfd, buf, numbytes, 0) == -1)
        {
            perror("send error");
            continue;
        }
        pthread_mutex_unlock(&send_mutex); // Unlock mutex after sending
    }
}

// Function to handle receiving data
void *recv_thread(void *arg)
{
    char buf[MAXDATASIZE];
    ssize_t numbytes;

    while (1)
    {
        pthread_mutex_lock(&recv_mutex); // Lock mutex before receiving
        numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
        pthread_mutex_unlock(&recv_mutex); // Unlock mutex after receiving

        if (numbytes <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        buf[numbytes] = '\0'; // Null-terminate the received string
        printf("server: %s\n", buf);
    }
}

int main(int argc, char *argv[])
{
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    pthread_t send_thread_id, recv_thread_id;

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
            close(sockfd);
            perror("client: connect");
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

    // Create send and receive threads
    if (pthread_create(&send_thread_id, NULL, send_thread, NULL) != 0)
    {
        perror("pthread_create");
        return 1;
    }

    if (pthread_create(&recv_thread_id, NULL, recv_thread, NULL) != 0)
    {
        perror("pthread_create");
        return 1;
    }

    // Wait for threads to finish
    pthread_join(send_thread_id, NULL);
    pthread_join(recv_thread_id, NULL);

    close(sockfd);

    return 0;
}
