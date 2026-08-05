#ifndef NHEADERS
#define NHEADERS
#include<pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#define BUFFERSIZE 1024
#define KMAX 5

typedef struct message {
    int head;
    int sc_code;
    char msg[BUFFERSIZE];
}message;

void msgcpy(message *dest,message *src) {
    dest->head = src->head;
    strcpy(dest->msg,src->msg);
}
#define UDP_SOCKET(x) if( (x = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0) {\
                        printf("Error creating socket\n");\
                        return -1;\
                        }

#define BIND(x,y,z) if( (x = bind(y,(struct sockaddr *)&z,sizeof(z))) < 0) {\
                        printf("Could not bind the socket\n");\
                        return -1;\
                        } 

#define BIND_THREAD_COMPAT(x,y,z) if( (x = bind(y,(struct sockaddr *)&z,sizeof(z))) < 0) {\
                        printf("Could not bind the socket\n");\
                        return NULL;\
                        } 

#define TCP_SOCKET(x) if( (x = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0) {\
                        printf("Error creating socket\n");\
                        return -1;\
                        }   

#define TCP_SOCKET_THREAD_COMPAT(x) if( (x = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0) {\
                        printf("Error creating socket\n");\
                        return NULL;\
                        } 

#define TRY_CATCH(x,msg) if ((x) < 0) {\
                        printf(msg);\
                        return -1;\
                        }   

#define TRY_CATCH_THREAD_COMPAT(x,msg) if ((x) < 0) {\
                        printf(msg);\
                        return NULL;\
                        }  

#define BUFFERSIZE 1024
#define KMAX 5

typedef struct q_elem{
    struct message msg;
    int conn_sock;
} q_elem;

typedef struct que {
    q_elem pque[KMAX+1];
    int head;
    int tail;
    pthread_mutex_t m_mutex;
} que;

int init_q(que *q) {
    q->head = 1;
    q->tail = 0;
    if(!pthread_mutex_init(&q->m_mutex,NULL)) {
        return 0;
    }
    return -1;
}

int addelem(que *q,q_elem qe) {
    if(!pthread_mutex_lock(&q->m_mutex)) {
        if(q->head == q->tail) {
            pthread_mutex_unlock(&q->m_mutex);
            return -1;
        }
        msgcpy(&q->pque[q->head].msg,&qe.msg);
        q->pque[q->head].conn_sock = qe.conn_sock;
        q->head = (q->head + 1)%(KMAX + 1);
        pthread_mutex_unlock(&q->m_mutex);
        return 0;
    } 
    return -2;
}

int delelem(que *q,q_elem *elem) {
    if(!pthread_mutex_lock(&q->m_mutex)) {
        if(q->head == ((q->tail+1)%(KMAX + 1))) {
            pthread_mutex_unlock(&q->m_mutex);
            return -1;
        }
        q->tail = (q->tail + 1)%(KMAX + 1);
        elem->conn_sock = q->pque[q->tail].conn_sock;
        msgcpy(&elem->msg,&q->pque[q->tail].msg);
        pthread_mutex_unlock(&q->m_mutex);
        return 0;
    } 
    return -2;
}    

int isQEmpty(que q) {
    //printf("In isQEmpty\n");
    //if(!pthread_mutex_lock(&q.m_mutex)) {
        if(q.head == ((q.tail+1)%(KMAX + 1))) {
            //pthread_mutex_unlock(&q.m_mutex);
            return 1;
        } else {
            //pthread_mutex_unlock(&q.m_mutex);
            return 0;
        }
    //}
}

#endif