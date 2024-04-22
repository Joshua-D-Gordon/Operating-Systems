#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define PORT "9034"     // Port we're listening on
#define MAX_CLIENTS 10  // Maximum number of clients
#define MAXDATASIZE 100 // Max number of bytes we can get at once

// Structure to hold client information
typedef struct
{
    int sockfd;
    struct sockaddr_storage addr;
} client_info;

client_info client_connections[MAX_CLIENTS]; // Array to store client connections
int num_clients = 0;                         // Number of connected clients

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Function to handle communication with a client
void *client_handler(void *arg)
{
    printf("Client handler thread created.\n");
    client_info *info = (client_info *)arg;
    int sockfd = info->sockfd;
    struct sockaddr_storage addr = info->addr;

    char buf[MAXDATASIZE];
    ssize_t numbytes;
    // Receive and send data
    while (1)
    {
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv error");
            break;
        }

        if (numbytes == 0)
        {
            printf("Client disconnected.\n");
            break;
        }

        buf[numbytes] = '\0'; // Null-terminate the received string
        printf("Client: %s\n", buf);

        for (int i = 0; i < num_clients; i++)
        {
            if(client_connections[i].sockfd != sockfd){
                // Echo received data back to the client
                if (send(client_connections[i].sockfd, buf, numbytes, 0) == -1)
                {
                    perror("send");
                    break;
                }
            }
        }
    }
    free(info); // Free memory allocated for client_info

    // Close the socket
    close(sockfd);
    return NULL;
}

// Function to create a listening socket
int get_listener_socket(void)
{
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // Allow the port to be reused quickly after the program exits
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    // If we got here, it means we didn't get bound
    if (p == NULL)
    {
        return -1;
    }

    freeaddrinfo(ai); // Done with this

    // Listen
    if (listen(listener, 10) == -1)
    {
        return -1;
    }

    return listener;
}

// Main function to accept incoming clients and handle communication
void *accept_clients(void *arg)
{
    int listener = *((int *)arg);
    free(arg); // Free memory allocated for listener socket

    // Main loop to accept clients
    while (1)
    {
        struct sockaddr_storage remoteaddr; // Client address
        socklen_t addrlen = sizeof(remoteaddr);

        // Accept a new client
        int newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
        if (newfd == -1)
        {
            perror("accept");
            continue;
        }
        // Check if maximum number of clients has been reached
        if (num_clients >= MAX_CLIENTS)
        {
            printf("Maximum number of clients reached. Connection rejected.\n");
            close(newfd);
            continue;
        }

        // Save client connection
        client_connections[num_clients].sockfd = newfd;
        memcpy(&(client_connections[num_clients].addr), &remoteaddr, sizeof(remoteaddr));
        num_clients++;

        // // Create a thread to handle this client
        // client_info *info = (client_info *)malloc(sizeof(client_info));
        // info->sockfd = newfd;
        // memcpy(&(info->addr), &remoteaddr, sizeof(remoteaddr));

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, &client_connections[num_clients - 1]) != 0)
        {
            perror("pthread_create");
            close(newfd);
            // free(&client_connections[num_clients - 1]);
            continue;
        }

        char s[INET6_ADDRSTRLEN];
        inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), s, sizeof s);
        printf("Server: Got connection from %s\n", s);
    }

    return NULL;
}

int main(void)
{
    // Create a thread to accept incoming clients
    int listener = get_listener_socket();
    if (listener == -1)
    {
        fprintf(stderr, "Error getting listening socket\n");
        exit(1);
    }

    pthread_t accept_thread;
    int *listener_ptr = malloc(sizeof(int));
    *listener_ptr = listener;
    // if (pthread_create(&accept_thread, NULL, accept_clients, listener_ptr) != 0)
    // {
    //     perror("pthread_create");
    //     exit(1);
    // }

    // pthread_join(accept_thread, NULL); // Wait for the accept thread to finish
    accept_clients(listener_ptr);
    close(listener); // Close the listening socket

    free(listener_ptr); // Free the allocated memory for listener pointer

    return 0;
}
