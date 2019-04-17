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
    int local_msglen;
    char buf[BUFFER_MAX];
    while ((local_msglen = recv(local_sd, buf, sizeof(buf), 0))) {
        fputs(buf, stdout);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    struct hostent *hp;
    struct sockaddr_in sin;
    char buf[BUFFER_MAX];

    if(argc != 2) {
        fprintf(stderr, "Error: Usage: %s Server-IP\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    hp = gethostbyname(argv[1]);
    if (!hp) {
        fprintf(stderr, "[Chatroom] unknown host: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(DEFAULT_SERVER_PORT);

    if ((local_sd = socket(PF_INET,SOCK_STREAM,0)) < 0) {
        perror("[Chatroom] Creating socket failed: ");
        exit(EXIT_FAILURE);
    }
    if (connect(local_sd, (struct sockaddr *) &sin, sizeof(sin))  < 0 ) {
        perror("[Chatroom] Connect failed: "); 
        exit(EXIT_FAILURE);
    }

    pthread_t recvs;
    if(pthread_create(&recvs, NULL, recvHandle, NULL) != 0) {
        perror("[Chatroom] Create Thread failed: ");
        exit(EXIT_FAILURE);
    }
    pthread_detach(recvs);

    uint16_t net_msglen = 0;
    uint32_t local_msglen = 0;
    while (fgets(buf, sizeof(buf), stdin))  {
        buf[BUFFER_MAX-1] = '\0' ; 
        local_msglen = strlen(buf) + 1;
        net_msglen = htons(local_msglen);
        send(local_sd, &net_msglen, 2, 0);
        send(local_sd, buf, local_msglen, 0 ) ; 
    }
    return 0;
}