#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

/**
 * tcpechoserver1 (port number)
 * port numberにコネクションを待ち、接続したクライアントから受信したデータをそのまま送り返す、を繰り返す
 * 複数スレッド版
 * 1. socket
 * 2. bind
 * 3. listen
 * 4. accept
 * 5. read / write
 * 6. close
 */

int NTHREAD = 40;

void *server(void *arg) {
    int s, ss = *(int *)arg, m;
    struct sockaddr_in senderaddr;
    socklen_t addrlen;
    char buf[2048];

    s = accept(ss, (struct sockaddr *)&senderaddr, &addrlen);
    while (1) {
        memset(buf, 0, sizeof(buf));
        m = read(s, buf, sizeof(buf));
        write(s, buf, m);
    }
    close(s);
}

int main(int argc, char **argv) {
    int sock;
    int i;
    struct sockaddr_in addr;
    pthread_t thread[NTHREAD];

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(sock, NTHREAD);
    for (i = 0; i < NTHREAD; i++)
        pthread_create(&thread[i], NULL, server, &sock);
    for (i = 0; i < NTHREAD; i++)
        pthread_join(thread[i], NULL);

    return 0;
}
