#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

int main(int argc, char **argv) {
    int sock;
    struct sockaddr_in addr;
    struct timeval tv, gtv;
    char *buff;
    socklen_t addrlen;

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    // datasize time スループットMbps

    for (int i=0; i < 1024*100; i++) {
        *(buff + i) = 'a';
    }

    char *endp = buff + 1024 * 100;

    addrlen = sizeof(addr);
    connect(sock, (struct sockaddr *)&addr, addrlen);
    gettimeofday(&tv, NULL);

    while (buff < endp) {
        size_t cn = write(sock, buff, endp-buff);
        if (cn == -1) {
            perror("writing error");
            return 1;
        }
        buff += cn;
    }

    gettimeofday(&gtv, NULL);
    read(sock, buff, 2048);
    printf("%d %ld %f\n", 102400, gtv.tv_sec - tv.tv_sec, 10.0 * 8 / (gtv.tv_sec-tv.tv_sec));

    close(sock);
    return 0;
}
