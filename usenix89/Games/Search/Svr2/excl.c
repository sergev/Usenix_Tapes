#include <fcntl.h>
#include <stdio.h>

excl(str)
char *str;
{
    int fd, i;
    struct flock flock;

    flock.l_type = F_WRLCK;
    flock.l_start = 0;
    flock.l_len = 0;

    if (!access(str,0)) return -1;
    fd = open(str, O_CREAT|O_WRONLY, 0666);
    if (fd < 0) return fd;
    i = fcntl(fd, F_SETLK, &flock);
    return i;
}
