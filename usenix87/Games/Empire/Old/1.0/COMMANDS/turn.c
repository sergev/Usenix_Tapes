#define	D_TELSTR
#define	D_FILES
#include	"empdef.h"

turn()
{
	register char	*cp;
	char	buf[512], *getstar();

	cp = getstar(argp[1], "on, off or message? ");
	switch( *(cp+1) ) {
	case 'n':
		unlink(downfil);
		telf = creat(upfil, 0600);
		printf("on ");
		break;
	case 'f':
		unlink(upfil);
		telf = creat(downfil, 0600);
		printf("off ");
		break;
	default:
		telf = creat(upfil, 0600);
		break;
	}
	time(&tgm.tel_date);
	if( (tgm.tel_length = getele("The World", buf, argp[2]) + 1) <=  0 ) {
		printf("Ignored");
		return(SYN_RETURN);
	}
	write(telf, &tgm, sizeof(tgm));
	write(telf, buf, tgm.tel_length);
	close(telf);
	return(NORM_RETURN);
}
