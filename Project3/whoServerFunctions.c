#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>

#include "extrafunctions.h"
#include "whoServerFunctions.h"

pthread_mutex_t lockmtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_obt = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mem = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_nonfull;

extern pthread_cond_t cond_nonempty;

void* thread_func(void* arg){

    if(pthread_mutex_lock(&lockmtx)){ printf("pthread_mutex_lock error");  exit(1);}    //lock to pass safely arguments

    int sock;
    pool_t* pool = (pool_t*)arg;

    if(pthread_mutex_unlock(&lockmtx)){ printf("pthread_mutex_lock error");  exit(1);}

    char* buf;
    while(1){

        if(pthread_mutex_lock(&mtx_obt)){ printf("pthread_mutex_lock error");  exit(1);}    //lock to obtain
        sock = Obtain(pool);    //obtain
        if(pthread_mutex_unlock(&mtx_obt)){ printf("pthread_mutex_lock error");  exit(1);}

        pthread_cond_signal(&cond_nonfull);


        buf = ReadMess(sock,BufferSize);

        if(strcmp(buf,"stats") == 0){   // uncomment the code to print statistics
            if(strcmp(buf,"END") != 0) printf("%s\n",buf );

            while(strcmp(buf,"END") != 0){
                free(buf);
                buf = ReadMess(sock,BufferSize);

                if(strcmp(buf,"END") != 0) printf("%s\n",buf );    // print statistics
            }
            free(buf);


            buf = ReadMess(sock,BufferSize);    //read worker port
            int port = atoi(buf);


            int wsock;
            struct sockaddr_in server;
            struct sockaddr *serverptr = (struct sockaddr*)&server;

            if((wsock = socket(PF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket");

            struct hostent *rem;
            if((rem = gethostbyname(pool->ip)) == NULL){
                herror("gethostbyname"); exit(1);
            }
            memcpy(&server.sin_addr, rem->h_addr, rem->h_length);

            server.sin_family = AF_INET;
            server.sin_port = htons(port);

            printf("Connecting to port %d\n", port);
            if(connect(wsock, serverptr, sizeof(server)) < 0) perror_exit("connect");

            pool->list = AddListnode(pool->list,wsock);

            // Printlist(pool->list);

            free(buf);
        }
        else{
            char* rest = NULL;

            if(pthread_mutex_lock(&mem)){ printf("pthread_mutex_lock error");  exit(1);}

            listnode* temp = pool->list;

            buf[strlen(buf)-1] = '\0';
            char* tempbuf = strdup(buf);
            printf("\n%s\n",tempbuf );


            strtok_r(tempbuf, " ", &rest);
            if(strcmp(tempbuf, "/topk-AgeRanges") == 0){

                bool success=0;
                temp = pool->list;
                while(temp != NULL){
                    WriteMess(buf,temp->sock,BufferSize);   //write query to workers

                    char* rd = NULL;
                    rd = ReadMess(temp->sock,BufferSize);

                    if(strcmp(rd,"-")!=0){
                        success = 1;
                        printf("%s\n",rd );
                    }

                    free(rd);
                    temp = temp->next;
                }
                if(!success) printf("No cases found\n");

            }
            else if(strcmp(tempbuf, "/diseaseFrequency") == 0){
                int stats = 0;
                temp = pool->list;
                while(temp != NULL){
                    WriteMess(buf,temp->sock,BufferSize);   //write query to workers

                    char* rd = NULL;
                    rd = ReadMess(temp->sock,BufferSize);

                    if(atoi(rd) > 0) stats += atoi(rd);

                    free(rd);
                    temp = temp->next;
                }

                if(stats == 0) printf("Not found\n");
                else printf("%d\n",stats );

            }
            else if(strcmp(tempbuf, "/searchPatientRecord") == 0){

                bool in = 0;
                temp = pool->list;
                while(temp != NULL){
                    WriteMess(buf,temp->sock,BufferSize);   //write query to workers

                    char* res = NULL;
                    res = ReadMess(temp->sock,BufferSize);

                    if(strcmp(res,"-") != 0){
                        in = 1;
                        printf("%s\n",res );
                    }
                    free(res);

                    temp = temp->next;
                }
                if(!in) printf("Record doesn't exist\n");

            }
            else if(strcmp(tempbuf, "/numPatientAdmissions") == 0){

                bool in = 0;
                temp = pool->list;
                while(temp != NULL){
                    WriteMess(buf,temp->sock,BufferSize);   //write query to workers

                    char* res = NULL;
                    res = ReadMess(temp->sock,BufferSize);

                    if(strcmp(res,"-") != 0){
                        in = 1;
                        printf("%s",res );
                    }
                    free(res);

                    temp = temp->next;
                }
                if(!in) printf("Didn't found results\n");
            }
            else if(strcmp(tempbuf, "/numPatientDischarges") == 0){
                bool in = 0;
                temp = pool->list;
                while(temp != NULL){
                    WriteMess(buf,temp->sock,BufferSize);   //write query to workers

                    char* res = NULL;
                    res = ReadMess(temp->sock,BufferSize);

                    if(strcmp(res,"-") != 0){
                        in = 1;
                        printf("%s",res );
                    }
                    free(res);

                    temp = temp->next;
                }
                if(!in) printf("Didn't found results\n");
            }
            else if(strcmp(tempbuf, "/exit") == 0){
                temp = pool->list;
                while(temp != NULL){
                    WriteMess(buf,temp->sock,BufferSize);

                    temp = temp->next;
                }

            }

            free(tempbuf);
            free(buf);

            if(pthread_mutex_unlock(&mem)){ printf("pthread_mutex_lock error");  exit(1);}
        }


    }

    listnode* trav = pool->list;
    while(trav != NULL){    //close socks
        close(trav->sock);
        trav = trav->next;
    }

    close(sock);

    pthread_exit(NULL);
}

void Initialize(pool_t* pool, int arraysize){
    pool->start = 0;
    pool->end = -1;
    pool->count = 0;
    pool->size = arraysize;
    pool->list = NULL;

    pool->fd = malloc(arraysize * sizeof(int));
}

void Place(pool_t* pool, int sock_fd){
    pthread_mutex_lock(&mtx);

    while(pool->count >= pool->size){
        pthread_cond_wait(&cond_nonfull, &mtx);
    }
    pool->end = (pool->end + 1) % pool->size;
    pool->fd[pool->end] = sock_fd;
    pool->count++;

    pthread_mutex_unlock(&mtx);
}

int Obtain(pool_t* pool){
    pthread_mutex_lock(&mtx);

    while(pool->count <= 0){
        pthread_cond_wait(&cond_nonempty, &mtx);
    }

    int sock_fd = 0;
    sock_fd = pool->fd[pool->start];
    pool->start = (pool->start + 1) % pool->size;
    pool->count--;

    pthread_mutex_unlock(&mtx);

    return sock_fd;
}

listnode* AddListnode(listnode* prev, int sock){
	listnode* node = (listnode*)malloc(sizeof(listnode));

    node->sock = sock;
    node->next = prev;

	return node;
}

void Printlist(listnode* root){
    while(root != NULL){
        printf("%d ",root->sock);
        root = root->next;
    }
    printf("\n" );
}


int AcceptConnections(int fds[], int times, struct sockaddr *addr, socklen_t *addrlen){
    fd_set readfds;
    int i,maxfd,fd;

    FD_ZERO(&readfds);
    maxfd = -1;
    for(i=0; i<times ;i++){ //find max fd
        FD_SET(fds[i], &readfds);

        if(fds[i] > maxfd) maxfd = fds[i];
    }

    if(select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) return -1;

    fd = -1;
    for(i=0; i<times ;i++){
        if(FD_ISSET(fds[i], &readfds)){
            fd = fds[i];
            break;
        }
    }

    if(fd == -1) return -1;
    else return accept(fd, addr, addrlen);
}
