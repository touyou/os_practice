#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * udpechoserver (port number)
 * 受信し、そのまま送り返すを繰り返す
 * 1. socket
 * 2. bind
 * 3. recvfrom / sendto
 * 4. close
 */

int main(int argc, char **argv) {
    int sock;
    struct sockaddr_in addr;
    struct sockaddr_in senderinfo;
    socklen_t addrlen;
    char buf[2048];
    char senderstr[16];
    int n;

    // make socket
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // socket information
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    while (1) {
        memset(buf, 0, sizeof(buf));
        addrlen = sizeof(senderinfo);

        n = recvfrom(sock, buf, sizeof(buf)-1, 0, (struct sockaddr *)&senderinfo, &addrlen);

        sendto(sock, buf, n, 0, (struct sockaddr *)&senderinfo, addrlen);
    }
    close(sock);

    return 0;
}
