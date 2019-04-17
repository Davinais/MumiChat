#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include "packet.h"

#define DEFAULT_SERVER_PORT 9487

int local_sd;

void* recvHandle() {
    int local_msglen;
    packet_t rx_pkt;
    struct tm *rxtm;
    memset((char *)&rx_pkt, 0, sizeof(rx_pkt));
    while ((local_msglen = recv(local_sd, &rx_pkt, sizeof(rx_pkt), 0))) {
        rxtm = localtime(&(rx_pkt.timestamp));
        fprintf(stdout, "%02d:%02d:%02d | [%s] %s\n", 
            rxtm->tm_hour, rxtm->tm_min, rxtm->tm_sec, rx_pkt.username, rx_pkt.buf);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    struct hostent *hp;
    struct sockaddr_in sin;
    packet_t tx_pkt;
    memset((char *)&tx_pkt, 0, sizeof(tx_pkt));

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

    printf("Enter User Name: ");
    // TODO: Input Buffer Check
    fgets(tx_pkt.username, sizeof(tx_pkt.username), stdin);
    tx_pkt.username[strlen(tx_pkt.username)-1] = '\0';
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

    tx_pkt.pkt_type = SINGLE_PKT;
    tx_pkt.opt = SENDMSG;
    // TODO: Input Buffer Check
    while (fgets(tx_pkt.buf, sizeof(tx_pkt.buf), stdin)) {
        tx_pkt.buf[strlen(tx_pkt.buf)-1] = '\0';
        tx_pkt.timestamp = time(NULL);
        send(local_sd, &tx_pkt, sizeof(tx_pkt), 0 );
    }
    return 0;
}