#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    FILE *ffile, *tfile;
    ssize_t rstat, wstat;
    size_t count = 1;
    char buf[count];
    
    if ((ffile = fopen(argv[1], "r")) == NULL) {
        perror("コピー元のファイルが開けません。");
        return -1;
    }
    if ((tfile = fopen(argv[2], "w")) == NULL) {
        perror("コピー先のファイルを作成できません。");
        return -1;
    }
    
    printf("start copy from %s to %s\n", argv[1], argv[2]);
    
    struct timeval tv, aftv;
    gettimeofday(&tv, NULL);
    
    while (1) {
        fread(buf, count, 1, ffile);
        if (feof(ffile) != 0) break;
        if (ferror(ffile) != 0) {
            perror("ファイルの読み出し中にエラーが発生しました。");
            return -1;
        }
        
        fwrite(buf, count, 1, tfile);
        
        if (ferror(tfile) != 0) {
            perror("ファイルの書き出し中にエラーが発生しました。");
            return -1;
        }
    }
    
    gettimeofday(&aftv, NULL);
    printf("time: %ld sec %06lu micro sec\n", aftv.tv_sec-tv.tv_sec, aftv.tv_usec-tv.tv_usec);
    
    fclose(ffile);
    fclose(tfile);
    
    return 0;
}
