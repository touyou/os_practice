#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[], char *envp[]) {
    int pid;
    int status;
    if ((pid = fork()) == 0) {
        // child
        char *opt[argc];
        for (int i=0; i<argc; i++) {
            opt[i] = argv[i+1];
        }
        execve(opt[0], opt, envp);
        perror("runnning failed");
        return -1;
    } else {
        // parent
        waitpid(pid, &status, WUNTRACED);
    }
    return 0;
}
