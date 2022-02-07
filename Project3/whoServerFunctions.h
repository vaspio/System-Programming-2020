#pragma once

#include <pthread.h>
#include <unistd.h>

#define BufferSize 10

typedef struct listnode{
    int sock;
    struct listnode* next;
}listnode;

typedef struct {
    int* fd;    // fd buffer
    int size;   // size of buffer
    int count;  //current
    int start;
    int end;
    char* ip;   // ip to connect to sockets

    listnode* list; //list of sockets
}pool_t;


// circular buffer functions //
void Initialize(pool_t* pool, int arraysize);
void Place(pool_t* pool, int sock_fd);
int Obtain(pool_t* pool);

// add new node to list //
listnode* AddListnode(listnode* prev, int sock);
// print the list //
void Printlist(listnode* root);

// function that reads statistics and queries and prints results //
void* thread_func(void* arg);

// accept a socket connection with select //
int AcceptConnections(int fds[], int times, struct sockaddr *addr, socklen_t *addrlen);
