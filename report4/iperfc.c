#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 10000000000
char sendbuf[BUFSIZE];


int main(int argc, char **argv) {
  int sock;
  struct sockaddr_in addr;
  struct timeval tv, gtv;
  socklen_t addrlen;
  char buf[10];

  if (argc != 3) {
    perror("number of argument");
    return 1;
  }

  // socket prepare
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(argv[2]));
  addr.sin_addr.s_addr = inet_addr(argv[1]);
  addrlen = sizeof(addr);

  memset(sendbuf, 0, sizeof(sendbuf));
  sendbuf[BUFSIZE-1] = EOF;

  // connect
  if (connect(sock, (struct sockaddr *)&addr, addrlen) == -1) {
    perror("connection");
    return 1;
  }

  // send
  gettimeofday(&tv, NULL);
  size_t cn = write(sock, sendbuf, sizeof(sendbuf));
  if (cn == -1) {
    perror("sending");
    return 1;
  }
  // return EOF
  read(sock, buf, 10);
  gettimeofday(&gtv, NULL);

  double tim = gtv.tv_sec - tv.tv_sec + (gtv.tv_usec - tv.tv_usec) / 1000000.0;
  printf("%ld byte %f sec %f Mbps\n", BUFSIZE, tim, BUFSIZE/1000000.0*8.0/tim);

  close(sock);
  return 0;
}
