#include <sgtty.h>
raw(lu)  /* Set logical unit lu to raw mode. */
int lu;
{
struct sgttyb tty;

ioctl(lu,TIOCGETP,&tty);
tty.sg_flags = tty.sg_flags | 040;
ioctl(lu,TIOCSETP,&tty);
return;
}
