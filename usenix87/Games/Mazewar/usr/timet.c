#include <termio.h>
#include <fcntl.h>

int f;
struct termio t;

main()
{

	f = open("/dev/tty11",O_RDONLY | O_NDELEY);

	ioctl(f,TCGETA,&t);
	t.c_cc[VMIN] = 255;

