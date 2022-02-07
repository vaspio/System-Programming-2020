#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

#include "extrafunctions.h"
#include "whoServerFunctions.h"

pool_t pool;
pthread_cond_t cond_nonempty;

int main(int argc, char *argv[]){
    int i,j,numThreads,buffer;
    int qport,statport;

    if(argc == 9){
        for(i=1; i<argc ;i++){
    		if(strcmp(argv[i],"-w") == 0) numThreads = atoi(argv[i+1]);
            if(strcmp(argv[i],"-b") == 0) buffer = atoi(argv[i+1]);

            if(strcmp(argv[i],"-s") == 0) statport = atoi(argv[i+1]);
            if(strcmp(argv[i],"-q") == 0) qport = atoi(argv[i+1]);
        }
        if(numThreads > 30 || numThreads <= 0){
            printf("Number of Threads should be a positive number < 30\n" );
            exit(EXIT_FAILURE);
        }

    }
    else{
        printf("Wrong input\n");
        exit(EXIT_FAILURE);
    }


    int sock1,sock2,cursock;
    socklen_t qclientlen;
    socklen_t statclientlen;
    struct sockaddr_in qserver, qclient, statserver, statclient;

    struct sockaddr* queryserver_ptr = (struct sockaddr*)&qserver;
    struct sockaddr* queryclient_ptr = (struct sockaddr*)&qclient;

    struct sockaddr* statserver_ptr = (struct sockaddr*)&statserver;
    struct sockaddr* statclient_ptr = (struct sockaddr*)&statclient;

    //  socket for queries  //
    if((sock1 = socket(PF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket1");
    qserver.sin_family = AF_INET;
    qserver.sin_addr.s_addr = htonl(INADDR_ANY);
    qserver.sin_port = htons(qport);

    qclientlen = sizeof(queryserver_ptr);

    int opt = 1;  //To enable the socket options
    if(setsockopt(sock1, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if(bind(sock1, queryserver_ptr, sizeof(qserver)) < 0) perror_exit("bind");

    if(listen(sock1, 5) < 0) perror_exit("listen");
    printf("Listening for queries to port %d\n", qport);

    //  socket for summary statistics  //
    if((sock2 = socket(PF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket1");
    statserver.sin_family = AF_INET;
    statserver.sin_addr.s_addr = htonl(INADDR_ANY);
    statserver.sin_port = htons(statport);

    statclientlen = sizeof(statserver_ptr);

    if(setsockopt(sock2, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if(bind(sock2, statserver_ptr, sizeof(statserver)) < 0) perror_exit("bind");

    if(listen(sock2, 5) < 0) perror_exit("listen");
    printf("Listening for statistics to port %d\n", statport);

    int fds[2];
    fds[0] = sock1;
    fds[1] = sock2;


    Initialize(&pool, buffer);   // initialize pool

    pthread_t* threadArr;
    threadArr = malloc(numThreads * sizeof(pthread_t));

    for(i=0; i<numThreads ;i++){
        pthread_create(threadArr+i, NULL, thread_func, (void*) &pool);
    }

    char workerIP[20];
    while(1){
        usleep(100);

    	if((cursock = AcceptConnections(fds, 2, queryserver_ptr, &qclientlen)) < 0) perror_exit("accept");
    	// printf("Accepted connection\n");

        getpeername(cursock, (struct sockaddr *)&qserver, &qclientlen);
        memset(workerIP, 0, sizeof(workerIP));
        strcpy(workerIP, inet_ntoa(qserver.sin_addr));


        pool.ip = strdup(workerIP);
        Place(&pool, cursock);

        pthread_cond_signal(&cond_nonempty);
    }

    return 0;
}
