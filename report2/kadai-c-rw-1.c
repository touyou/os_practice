#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    int ffile, tfile;
    ssize_t rstat, wstat;
    size_t count = 1;
    char buf[count];
    
    if ((ffile = open(argv[1], O_RDONLY)) == -1) {
        perror("コピー元のファイルが開けません。");
        return -1;
    }
    if ((tfile = open(argv[2], O_RDWR|O_CREAT, S_IWRITE|S_IREAD)) == -1) {
        perror("コピー先のファイルを作成できません。");
        return -1;
    }
    
    printf("start copy from %s to %s\n", argv[1], argv[2]);
    
    struct timeval tv, aftv;
    gettimeofday(&tv, NULL);
    
    while ((rstat = read(ffile, buf, count)) != 0) {
        if (rstat == -1) {
            perror("ファイルの読み出し中にエラーが発生しました。");
            return -1;
        }
        
        wstat = write(tfile, buf, rstat);
        if (wstat == -1) {
            perror("ファイルの書き出し中にエラーが発生しました。");
            return -1;
        } else if (wstat < rstat) {
            perror("容量が足りません。");
            return -1;
        }
    }
    
    gettimeofday(&aftv, NULL);
    printf("time: %ld sec %06lu micro sec\n", aftv.tv_sec-tv.tv_sec, aftv.tv_usec-tv.tv_usec);
    
    if (close(tfile) == -1) {
        perror("コピー先のファイルが正常に閉じられませんでした。");
        return -1;
    }
    if (close(ffile) == -1) {
        perror("コピー元のファイルが正常に閉じられませんでした。");
        return -1;
    }
    
    return 0;
}
