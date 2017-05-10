#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

int main(int argc, char **argv) {
    int sock;
    struct sockaddr_in addr;
    struct timeval tv, gtv;
    char temp[1024];
    socklen_t addrlen;

    if (argc != 3) {
        perror("argument error");
        return 1;
    }

    puts("socket preparing...");

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    // datasize time スループットMbps

    for (int i=0; i < 1024; i++) {
        temp[1024] = 'a';
    }

    char *endp = temp + 1024;

    addrlen = sizeof(addr);
    if (connect(sock, (struct sockaddr *)&addr, addrlen) == -1) {
        perror("connect error");
        return 1;
    }
    puts("connected!");
    int count = 100000;
    gettimeofday(&tv, NULL);
    for (int i=0; i<count; i++) {
        char *buff = temp;
        while (buff < endp) {
            size_t cn = write(sock, buff, endp-buff);
            if (cn == -1) {
                perror("writing error");
                return 1;
            }
            buff += cn;
        }
    }
    gettimeofday(&gtv, NULL);
    puts("done!");
    double tim = gtv.tv_sec - tv.tv_sec + (gtv.tv_usec - tv.tv_usec) / 1000000.0;
    printf("%d byte %f sec %f Mbps\n", 1024*count, tim, count/1000.0*8.0/tim);

    close(sock);
    return 0;
}
