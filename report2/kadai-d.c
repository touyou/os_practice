#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    int fp, i;
    ssize_t bufsz;
    size_t count = 512;
    char buf[count];
    
    if ((fp = open(argv[1], O_RDONLY)) == -1) {
        perror("ファイルが開けません。");
        return -1;
    }
    
    int lc = 0, wc = 0, bn = 0;
    int tmp = 0;
    
    while ((bufsz = read(fp, buf, count)) != 0) {
        if (bufsz == -1) {
            perror("読み出し中のエラー");
            return -1;
        }
        
        bn += bufsz;
        
        int flag = 0;
        for (i=0; i<bufsz; i++) {
            if (buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\0') {
                flag = 1;
            } else if (flag == 1 && buf[i] == ' ') {
                flag = 0;
                tmp++;
            } else if (buf[i] == '\n' || buf[i] == '\0') {
                wc += (flag == 0 ? tmp : tmp + 1);
                flag = 0;
                tmp = 0;
                lc++;
            }
        }
    }
    
    printf("行数: %d 単語数: %d バイト数: %d\n", lc, wc, bn);
    if (close(fp) == -1) {
        perror("close failed");
        return -1;
    }
    
    return 0;
}
