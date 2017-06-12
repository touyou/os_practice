#include <linux/kernel.h>
#include <linux/syscalls.h>

#include <asm/syscalls.h>

int str_flip(char *p, int len) {
    // Flip given byte
    int i, ret = -1;
    char tmp[len];
    for (i=0; i<len; i++) {
        if (p[i] == '\0') {
            ret = i;
            break;
        }
        tmp[i] = p[i];
    }
    if (ret == -1) ret = len;
    for (i=0; i<ret; i++) {
        p[i] = tmp[ret-i-1];
    }
    return ret;
}

SYSCALL_DEFINE2(flip, char *, p, int, len) {
    return str_flip(p, len);
}


