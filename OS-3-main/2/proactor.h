#ifndef PROACTOR_H
#define PROACTOR_H
#define MAX_CLIENTS 10
#include <netinet/in.h>
#include <pthread.h>


typedef struct {
    int socket;
    struct sockaddr_in address;
} client_t;

typedef struct {
    int server_socket;
    client_t clients[MAX_CLIENTS];
    pthread_t thread;
} proactor_t;

proactor_t *proactor_create(int port);
void proactor_destroy(proactor_t *proactor);

#endif 
