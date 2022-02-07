#pragma once

#include <pthread.h>
#include <unistd.h>

#define BufferSize 10   // buffersize for read and write

typedef struct info{    // struct to pass arguments to pthread_create();
    int port;
    char* query;
    char* serv_ip;
}info;

// function to pthread_create that connects a thread to a socket //
void* func(void* arg);
