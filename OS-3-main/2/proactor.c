#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "proactor.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void *proactor_thread(void *arg)
{
    proactor_t *proactor = (proactor_t *)arg;
    fd_set read_fds;
    int max_fd = proactor->server_socket;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(proactor->server_socket, &read_fds);

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (proactor->clients[i].socket > 0)
            {
                FD_SET(proactor->clients[i].socket, &read_fds);
                if (proactor->clients[i].socket > max_fd)
                {
                    max_fd = proactor->clients[i].socket;
                }
            }
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(proactor->server_socket, &read_fds))
        {
            struct sockaddr_in client_address;
            socklen_t client_address_len = sizeof(client_address);
            int client_socket = accept(proactor->server_socket, (struct sockaddr *)&client_address, &client_address_len);
            if (client_socket < 0)
            {
                perror("accept");
                continue;
            }

            printf("Client connected\n");

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (proactor->clients[i].socket == 0)
                {
                    proactor->clients[i].socket = client_socket;
                    proactor->clients[i].address = client_address;
                    break;
                }
            }
        }

        char buffer[BUFFER_SIZE];
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (proactor->clients[i].socket > 0 && FD_ISSET(proactor->clients[i].socket, &read_fds))
            {
                int bytes_received = recv(proactor->clients[i].socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received <= 0)
                {
                    close(proactor->clients[i].socket);
                    proactor->clients[i].socket = 0;
                }
                else
                {
                    buffer[bytes_received] = '\0';
                    printf("Received message from client: %s\n", buffer);
                    for (int j = 0; j < MAX_CLIENTS; j++)
                    {
                        if (proactor->clients[j].socket > 0 && proactor->clients[j].socket != proactor->clients[i].socket)
                        {
                            send(proactor->clients[j].socket, buffer, bytes_received, 0);
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

proactor_t *proactor_create(int port)
{
    proactor_t *proactor = (proactor_t *)malloc(sizeof(proactor_t));
    if (proactor == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    proactor->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (proactor->server_socket < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(proactor->server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(proactor->server_socket, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    if (pthread_create(&proactor->thread, NULL, proactor_thread, proactor) != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    printf("Proactor created\n");

    return proactor;
}

void proactor_destroy(proactor_t *proactor)
{
    close(proactor->server_socket);
    pthread_cancel(proactor->thread);
    free(proactor);
}
