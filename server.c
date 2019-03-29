#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_SERVER_PORT 9487
#define MAX_CONNECT 20
#define MAX_PENDING 10

int main(int argc, char* argv[]) {
    
    struct sockaddr_in sa_local;
    memset(&sa_local, 0, sizeof(sa_local));
    sa_local.sin_family = AF_INET;
    sa_local.sin_addr.s_addr = INADDR_ANY;
    sa_local.sin_port = htons(DEFAULT_SERVER_PORT);

    int local_sd;
    int enable = 1;
    if((local_sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[Chatroom] Creating socket failed: ");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(local_sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("[Chatroom] Setsockopt failed: ");
        exit(EXIT_FAILURE);
    }

    if(bind(local_sd, (struct sockaddr*)&sa_local, sizeof(sa_local)) < 0) {
        perror("[Chatroom] Binding socket failed: ");
        exit(EXIT_FAILURE);
    }
    if(listen(local_sd, MAX_PENDING) < 0) {
        perror("[Chatroom] Listening failed: ");
        exit(EXIT_FAILURE);
    }

    int client_sd;
    struct sockaddr_in sa_client;

    return 0;
}
