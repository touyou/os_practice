#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

pid_t p_pid;

int main() {
    struct timeval tv, aftv;
    gettimeofday(&tv, NULL);
    p_pid = getpid();
    gettimeofday(&aftv, NULL);
    printf("1 times getpid() --- \nstart: %ld %06lu\nend: %ld %06lu\n",
        tv.tv_sec, tv.tv_usec, aftv.tv_sec, aftv.tv_usec);
    
    gettimeofday(&tv, NULL);
    for (int i=0; i<1000; i++) {
        p_pid = getpid();
    }
    gettimeofday(&aftv, NULL);
    printf("1000 times getpid() --- \nstart: %ld %06lu\nend: %ld %06lu\n",
        tv.tv_sec, tv.tv_usec, aftv.tv_sec, aftv.tv_usec);
    return 0;
}
