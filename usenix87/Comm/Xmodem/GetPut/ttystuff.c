#include <stdio.h>
#include <termio.h>

static struct termio _mtotty, _mtntty;
static int _mtflag = 0;

ttybinary()
{
	if(_mtflag==0)
		_mtgetmode();
	ioctl(fileno(stdin), TCSETA, &_mtntty);
	_mtflag = 2;
}

ttynormal()
{
	if(_mtflag==0)
		_mtgetmode();
	ioctl(fileno(stdin), TCSETA, &_mtotty);
}

_mtgetmode()
{
	_mtflag = 1;
	ioctl(fileno(stdin), TCGETA, &_mtotty);

	_mtntty = _mtotty;
	_mtntty.c_iflag = IGNBRK;
	_mtntty.c_oflag = _mtntty.c_lflag = 0;
	_mtntty.c_cflag &= (CBAUD);
	_mtntty.c_cflag |= (CS8|CREAD);
	_mtntty.c_cc[VMIN] = 1;
	_mtntty.c_cc[VTIME] = 1;
}

ttyr()
{
	int j;
	char ccc;
	j = read(0,&ccc,1);
	if(j==0)
		j = -1;
	else
		j = ccc & 255;
	return j;
}

ttyw(c)
char c;
{
	write(0,&c,1);
}
