#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    int sock;
    struct sockaddr_in addr;
    char buff[2048];
    char inbuff[2048];
    socklen_t addrlen;

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    memset(buff, 0, sizeof(buff));
    printf("> ");

    while (scanf("%s", buff) != EOF) {
        addrlen = sizeof(addr);
        connect(sock, (struct sockaddr *)&addr, addrlen);
        write(sock, buff, strlen(buff)+1);

        memset(inbuff, 0, sizeof(inbuff));
        read(sock, inbuff, 2048);

        printf("%s\n> ", inbuff);
        memset(buff, 0, sizeof(buff));
    }
    close(sock);
    return 0;
}
