#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_SERVER_PORT 9487
#define MAX_CONNECT 20
#define MAX_PENDING 10
#define BUFFER_MAX  2048

#define MESSAGE_STORE_FILE "chatmsg.txt"

int main(int argc, char* argv[]) {
    
    struct sockaddr_in sa_local;
    memset(&sa_local, 0, sizeof(sa_local));
    sa_local.sin_family = AF_INET;
    sa_local.sin_addr.s_addr = INADDR_ANY;
    sa_local.sin_port = htons(DEFAULT_SERVER_PORT);

    int local_sd;
    int enable = 1, i = 0, j = 0;
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
    uint32_t addrlen, recvbytes;
    uint16_t msgread;
    struct sockaddr_in sa_client;

    int fd_max;
    struct timeval timeout;
    fd_set master_fds, read_fds;
    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);

    FD_SET(local_sd, &master_fds);
    fd_max = local_sd;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    FILE *msg_file;
    if(!(msg_file = fopen(MESSAGE_STORE_FILE, "w"))) {
        perror("[Chatroom] Cannot open file: ");
        exit(EXIT_FAILURE);
    }
    char readbuf[BUFFER_MAX] = { 0 };
    while(1) {
        read_fds = master_fds;

        if(select(fd_max+1, &read_fds, NULL, NULL, &timeout) < 0) {
            perror("[Chatroom] Select Failed: ");
            exit(EXIT_FAILURE);
        }

        for(i = 0; i <= fd_max; i++) {
            if(FD_ISSET(i, &read_fds)) {
                if(i == local_sd) {
                    addrlen = sizeof(sa_client);
                    if((client_sd = accept(local_sd, (struct sockaddr *)&sa_client, &addrlen)) < 0) {
                        perror("[Chatroom] Accept Failed: ");
                    }
                    else {
                        FD_SET(client_sd, &master_fds);
                        if(client_sd > fd_max) {
                            fd_max = client_sd;
                        }
                        printf("[Chatroom] New Client: %d\n", client_sd);
                    }
                }
                else {
                    uint16_t msglen = 0;
                    if((recvbytes = recv(i, &msglen, 2, 0)) <= 0) {
                        if(recvbytes == 0) {
                            printf("[Chatroom] Client Disconnected: %d\n", i);
                        }
                        else {
                            perror("[Chatroom] recv Failed: ");
                        }
                        close(i);
                        FD_CLR(i, &master_fds);
                    }
                    else {
                        msglen = ntohs(msglen);
                        for(msgread = 0; msglen > 0;) {
                            if((recvbytes = recv(i, readbuf+msgread,
                                ((msglen<=BUFFER_MAX)?msglen:BUFFER_MAX), 0)) <= 0) {
                                perror("[Chatroom] recv Failed: ");
                                close(i);
                                FD_CLR(i, &master_fds);
                                break;
                            }
                            msglen -= recvbytes;
                            msgread += recvbytes;
                        }
                        printf("[New Mesg] %s", readbuf);
                        fwrite(readbuf, sizeof(char), msgread-1, msg_file); // -1 for the last '\0'
                        fputs("", msg_file);
                        fflush(msg_file);
                        for(j = 0; j <= fd_max; j++) {
                            if(FD_ISSET(j, &master_fds)) {
                                if((j != local_sd) && (j != i)) {
                                    int msgsend = 0, tmp = 0;
                                    while(msgsend < msgread) {
                                        if((tmp = send(j, readbuf+msgsend, msgread, 0)) < 0) {
                                            perror("[Chatroom] send Failed: ");
                                        }
                                        msgsend += tmp;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    fclose(msg_file);
    return 0;
}
