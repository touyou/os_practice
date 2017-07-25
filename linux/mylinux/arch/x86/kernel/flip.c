#include <linux/kernel.h>
#include <linux/syscalls.h>

#include <asm/syscalls.h>

int str_flip(char *p, int len) {
    // Flip given byte
    int i, ret = -1;
    char tmp[len], res[len];
    unsigned long x = copy_from_user(tmp, p, (unsigned long)len);
    if (ret == -1) ret = len;
    for (i=0; i<ret; i++) {
        res[i] = tmp[ret-i-1];
    }
    unsigned long y = copy_to_user(p, res, (unsigned long)len);
    return ret;
}

SYSCALL_DEFINE2(flip, char *, p, int, len) {
    return str_flip(p, len);
}


