#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <string.h>
#define BUF_SIZE 8192

struct linux_dirent {
    unsigned long  d_ino;
    off_t          d_off;
    unsigned short d_len;
    char           d_name[];
};

int main() {
    int filedes, nread,pos,aflag;
    char buf[BUF_SIZE];
    struct linux_dirent *d;

 

    filedes = syscall(SYS_openat, AT_FDCWD, ".", O_RDONLY | O_DIRECTORY);
    if (filedes == -1) {
        exit(0x55);
    }

    printf("File List:\n");

    nread = syscall(SYS_getdents, filedes, buf, BUF_SIZE);
    if (nread == -1) {
        exit(0x55);
    }

    for (pos = 0; pos < nread;) {
        d = (struct linux_dirent *)((char *)buf + pos);

        if (d->d_name[0] != '.') {
            printf("%s\n", d->d_name);
        }
       
        pos =pos+ (d->d_len);
    }
    close(filedes);
    return 0;
}
