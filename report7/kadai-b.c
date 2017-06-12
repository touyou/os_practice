#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_FLIP 300

int main() {
        char a[80] = "Hello, World";
        int ret;
        ret = syscall(SYS_FLIP, a, strlen(a));
        printf("%d: %s\n", ret, a);
        return 0;
}

