#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <poll.h>

/**
 * tcpechoserver1 (port number)
 * port numberにコネクションを待ち、接続したクライアントから受信したデータをそのまま送り返す、を繰り返す
 * select版
 * 1. socket
 * 2. bind
 * 3. listen
 * 4. accept
 * 5. read / write
 * 6. close
 */

int NTHREAD = 2000;

int main(int argc, char **argv) {
    int sock;
    int len;
    int sockoptval = 1;
    struct sockaddr_in addr;
    fd_set fds, readfds;
    char buf[2048];

    // sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(sockoptval)) == -1) {
        perror("setsockopt");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(sock, SOMAXCONN) == -1) {
        perror("listen");
        return 1;
    }

    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    while (1) {
        int i;
        memcpy(&fds, &readfds, sizeof(readfds));
        select(FD_SETSIZE, &fds, NULL, NULL, NULL);

        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &fds)) {
                printf("%d readable\n", i);
                if (i == sock) {
                    int new_sock;
                    len = sizeof(addr);
                    new_sock = accept(i, (struct sockaddr *)&addr, (socklen_t *)&len);
                    printf("accept %d\n", i);
                    if (new_sock == -1) {
                        perror("accept");
                        return 1;
                    }
                    if (new_sock <= FD_SETSIZE - 1) FD_SET(new_sock, &readfds);
                } else {
                    int read_size;
                    read_size = read(i, buf, sizeof(buf));
                    if (read_size == 0 || read_size == -1) {
                        printf("接続切れ%d\n",i);
                        close(i);
                    } else {
                        write(i, buf, read_size);
                    }
                    if (read_size == -1 || read_size == 0) FD_CLR(i, &readfds);
                }
            }
        }
    }

    close(sock);

    return 0;
}
