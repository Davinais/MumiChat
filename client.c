#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>

#define DEFAULT_SERVER_PORT 9487
#define BUFFER_MAX 2048

int local_sd;

void* recvHandle() {
    int len;
    char buf[BUFFER_MAX];
    while ((len = recv(local_sd, buf, sizeof(buf), 0))) {
        fputs(buf, stdout);
    }
    pthread_exit(NULL);
}

int main() {
    struct hostent *hp;
    struct sockaddr_in sin;
    char buf[BUFFER_MAX];
    int len;

    char* host = "127.0.0.1";
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "talk: unknown host: %s\n", host);
        exit(EXIT_FAILURE);
    }
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(DEFAULT_SERVER_PORT);

    if ((local_sd = socket(PF_INET,SOCK_STREAM,0)) < 0) {
        perror("talk: socket");
        exit(EXIT_FAILURE);
    }
    if (connect(local_sd, (struct sockaddr *) &sin, sizeof(sin))  < 0 ) {
        perror("talk: connect"); 
        exit(EXIT_FAILURE);
    }

    pthread_t recvs;
    if(pthread_create(&recvs, NULL, recvHandle, NULL) != 0) {
        perror("Create Thread failed: ");
        exit(EXIT_FAILURE);
    }
    pthread_detach(recvs);

    uint16_t msglen = 0;
    while ( fgets(buf, sizeof(buf), stdin) )  {
        buf[BUFFER_MAX-1] = '\0' ; 
        msglen = strlen(buf) + 1;
        len = msglen;
        msglen = htons(msglen);
        send(local_sd, &msglen, 2, 0);
        send(local_sd, buf, len, 0 ) ; 
    }
    return 0;
}