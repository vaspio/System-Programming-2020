#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>

#include "extrafunctions.h"
#include "whoClientFunctions.h"

#define BufferSize 10


pthread_cond_t cvar;
pthread_mutex_t mtx_arg;

int main(int argc, char *argv[]){
    int i,numThreads,server_port;
    char queryFile[30];
    char server_ip[100];

    if(argc == 9){
        for(i=1; i<argc ;i++){
    		if(strcmp(argv[i],"-q") == 0) strcpy(queryFile, argv[i+1]);
            if(strcmp(argv[i],"-sip") == 0) strcpy(server_ip, argv[i+1]);

            if(strcmp(argv[i],"-w") == 0) numThreads = atoi(argv[i+1]);
            if(strcmp(argv[i],"-sp") == 0) server_port = atoi(argv[i+1]);
        }

        if(numThreads > 50 || numThreads <= 0){
            printf("Number of Threads should be a positive number < 50\n" );
            exit(EXIT_FAILURE);
        }
    }
    else{
        printf("Wrong input\n");
        exit(EXIT_FAILURE);
    }
    printf("file %s|\n",queryFile );
    FILE* queries = fopen(queryFile,"r");
    if(!queries){
        fprintf(stderr, "Error opening file '%s'\n", queryFile);
        exit(EXIT_FAILURE);
    }

    pthread_t* threadArr;
    threadArr = malloc(numThreads * sizeof(pthread_t));

    pthread_mutex_init(&mtx_arg, 0);
    pthread_cond_init(&cvar, 0);

    info* arguments;
    char str[500];
    int t=0;

    printf("Creating threads and sending queries..\n" );

    arguments = malloc(sizeof(arguments));
    while(fgets(str, sizeof(str), queries) != NULL){
        // lock to pass safely the arguments //
        if(pthread_mutex_lock(&mtx_arg)){ printf("pthread_mutex_unlock arguments ERROR");  exit(1);}

        arguments->port = server_port;
        arguments->query = strdup(str);
        arguments->serv_ip = strdup(server_ip);

        if(t == numThreads){    // all threads have a query
            sleep(1);
            pthread_cond_broadcast(&cvar);

            for(i=0; i<numThreads ;i++){
                if(pthread_join(*(threadArr+i), NULL)){
                    printf("pthread_join ERROR\n"); exit(1);
                }

            }
            free(threadArr);    //free old threads

            threadArr = malloc(numThreads * sizeof(pthread_t));

            t=0;    //initialize
        }

        pthread_create(threadArr+t, NULL, func, (void *) arguments);

        t++;

    }

    sleep(2);
    pthread_cond_broadcast(&cvar);

    for(i=0; i<t ;i++){
        if(pthread_join(*(threadArr+i), NULL)){
            printf("pthread_join ERROR\n"); exit(1);
        }

    }
    printf("\nDone\n" );

    free(threadArr);

    free(arguments->serv_ip);
    free(arguments->query);
    free(arguments);

    pthread_mutex_destroy(&mtx_arg);
    pthread_cond_destroy(&cvar);

    fclose(queries);
}
