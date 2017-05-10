#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * udpechoclient (host) (port number)
 * 標準入力があったら送り、サーバーからの返答を標準出力に返す。EOFで終了。
 * 1. socket
 * 2. (bind)
 * 3. sendto / recvfrom
 * 4. close
 */

int main(int argc, char **argv) {
    int sock;
    struct sockaddr_in addr;
    char buff[2048];
    char inbuff[2048];
    socklen_t addrlen;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    printf("> ");

    while (scanf("%s", buff) != EOF) {
        addrlen = sizeof(addr);

        sendto(sock, buff, strlen(buff), 0, (struct sockaddr *)&addr, addrlen);

        memset(inbuff, 0, sizeof(inbuff));
        recvfrom(sock, inbuff, strlen(buff), 0, (struct sockaddr *)&addr, &addrlen);
        printf("%s\n> ", inbuff);
    }
    close(sock);
    return 0;
}
