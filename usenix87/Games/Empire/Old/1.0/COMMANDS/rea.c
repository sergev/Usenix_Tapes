#define D_TELSTR
#define D_FILES
#include	"empdef.h"

rea()
{
	register char	*cp, *mbp;
	char	buf[514], *cname(), *ctime();
	char	*getstri(), *mailbox();
	int	tgms, n, ntelf;

	mbp = mailbox(cnum);
	telf = open(mbp, O_RDWR);
	if( telf < 0 ) {
		printf("You have no telegraph office! (%s)\n", mbp);
		return(SYN_RETURN);
	}
	for( tgms=0; read(telf, &tgm, sizeof(tgm)) == sizeof(tgm); tgms++ ) {
		if( tgm.tel_length <  0 ||
		    tgm.tel_length > 512 ) {
			printf("Telegraph wires cut!  (size=%d)\n", tgm.tel_length);
			return(SYN_RETURN);
		}
		if( tgm.tel_from != 0 ) {
			printf("Telegram from %s, (#%d)", cname(tgm.tel_from), tgm.tel_from);
		} else {
			printf("BULLETIN!     ");
		}
		printf("  dated %s", ctime(&tgm.tel_date));
		read(telf, buf, tgm.tel_length);
		buf[tgm.tel_length] = '\0';
		printf("%s", buf);
		printf("\n");
	}
	switch( tgms ) {
	case 0:
		printf("No telegrams for you at the moment...\n");
		cp = "no";
		goto X430;
	case 1:
		if( rand() % 32768 > 060000 ) {
			cp = "Forget this one? ";
		} else {
			cp = "Shall I burn it? ";
		}
		break;
	default:
		if( rand() % 32768 > 060000 ) {
			cp = "Can I throw away these old love letters? ";
		} else {
			cp = "Into the shredder, boss? ";
		}
		break;
	}
	cp = getstri(cp);
X430:	
	if( *cp == 'y' ) {
		unlink(mbp);
		ntelf = creat(mbp, 0600);
		while( (n = read(telf, buf, sizeof(buf))) != 0 ) {
			write(ntelf, buf, n);
		}
		close(ntelf);
	}
	close(telf);
	return(NORM_RETURN);
}
