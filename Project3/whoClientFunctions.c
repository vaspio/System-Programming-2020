#include <stdio.h>
#include <stdlib.h>	         /* exit */
#include <string.h>
#include <sys/wait.h>	     /* sockets */
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <pthread.h>
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */

#include "extrafunctions.h"
#include "whoClientFunctions.h"

pthread_mutex_t mtx;
pthread_mutex_t mem;
extern pthread_cond_t cvar;
extern pthread_mutex_t mtx_arg;

void* func(void* arg){

    info* arguments = (info*)arg;
    // printf("\nThread port %d ip %s query %s ", arguments->port,arguments->serv_ip, arguments->query);
    char* query = strdup(arguments->query);
    if(pthread_mutex_unlock(&mtx_arg)){ printf("pthread_mutex_unlock arguments ERROR");  exit(1);}  // unlock now we passed the arguments

    // Synchronize all threads to connect at once //
    pthread_mutex_init(&mtx, 0);
    if(pthread_mutex_lock(&mtx)){ printf("pthread_mutex_unlock ERROR");  exit(1);}

    pthread_cond_wait(&cvar, &mtx); //wait for signal

    if(pthread_mutex_unlock(&mtx)){ printf("pthread_mutex_unlock ERROR");  exit(1);}


    // create and connect //
    int sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;

    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket");

    struct hostent *rem;
    if((rem = gethostbyname(arguments->serv_ip)) == NULL) {
       herror("gethostbyname"); exit(1);
    }
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);

    server.sin_family = AF_INET;
    server.sin_port = htons(arguments->port);

    if(connect(sock, serverptr, sizeof(server)) < 0) perror_exit("connect");


    if(pthread_mutex_lock(&mem)){ printf("pthread_mutex_unlock ERROR");  exit(1);}

    WriteMess(query,sock,BufferSize);

    if(pthread_mutex_unlock(&mem)){ printf("pthread_mutex_unlock ERROR");  exit(1);}

    free(query);
    // pthread_mutex_destroy(&mtx);

    pthread_exit(NULL);

}
