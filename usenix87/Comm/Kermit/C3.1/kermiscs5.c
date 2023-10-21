#include <stdio.h>
#include <termio.h>

#define TANDEM 0

extern int turn,speed;

ttopen(tn)
char *tn;
{
	int fd;

	if (tn == 0) return(0);
	if ((fd = open(tn,2)) < 0)
	{
		printmsg("Cannot open %s",tn);
		exit(1);
	}
	return(fd);
}


ttbin(fd,old)
int fd;
struct termio *old;
{
	struct termio new;

	ioctl(fd,TCGETA,old);
	ioctl(fd,TCGETA,&new);
	old->c_cflag &= ~HUPCL;	/* hold line */
	new.c_cflag &= ~HUPCL;	/* hold line */
	new.c_iflag = 0;
	new.c_oflag = 0;
	new.c_cflag = CS8|(new.c_cflag & CBAUD);
	new.c_lflag = 0;
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 1;
	if (!turn)
		new.c_iflag |= IXON|IXOFF;
	if (fd != 0) setspd(&new);		/* Speed if requested and supported */
	ioctl(fd,TCSETA,&new);
}

ttres(fd,old)
int fd;
struct termio *old;
{
	sleep(1);		/* wait before releasing the line */
	ioctl(fd,TCSETA,old);
}

setspd(tty)
struct termio *tty;
{
	int spbits;
	if (speed)				/* User specified a speed? */
	{
		switch(speed)			/* Get internal system code */
		{
		case 110:  
			spbits = B110; 
			break;
		case 150:  
			spbits = B150; 
			break;
		case 300:  
			spbits = B300; 
			break;
		case 1200: 
			spbits = B1200; 
			break;
		case 1800: 
			spbits = B1800; 
			break;
		case 2400: 
			spbits = B2400; 
			break;
		case 4800: 
			spbits = B4800; 
			break;
		case 9600: 
			spbits = B9600; 
			break;

		default:   
			printmsg("Bad line spbits.");
			exit(1);
		}
		tty->c_cflag &= ~CBAUD;
		tty->c_cflag |= spbits;
	}
}

