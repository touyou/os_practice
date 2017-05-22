#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile int count;

void handler(int signal) {
    count++;
}

int main() {
    int rc = 0;
    struct sigaction act;

    count = 0;

    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    act.sa_flags = 0;
    while (1) {
        if ((rc = sigaction(SIGINT, &act, NULL)) < 0) {
            perror("SIG_INT");
            return -1;
        }
        if (count >= 10) {
            printf("exit\n");
            break;
        }
    }
    return 0;
}
