/*
 * ct.c - sets tty nohang, noecho and ignore carrier,
 *  then does call back and restores tty settings
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define NORMAL 0
#define IGNORE (!NORMAL)

main (argc,argv)
int argc;
char *argv[];
{
        int nohup = LNOHANG;
        struct sgttyb ttystuff;

        if( argc != 2 ) {
            fprintf(stderr,"Usage: exec %s phone-number\n",argv[0]);
            exit(1);
        }

        if( setcarr(IGNORE)) {  /* ignore carrier */
            fprintf(stderr,"%s: error setting hardwired carrier\n",argv[0]);
            execl( "/bin/login", "login", (char *) 0 );
        }

        ioctl( 0, TIOCGETP, &ttystuff);
        ttystuff.x_flags = ttystuff.sg_flags &= (~ECHO); /* stty noecho */
        ioctl( 0, TIOCSETP, &ttystuff);

        ioctl( 0, TIOCLBIS, &nohup); /* stty nohang */

        setbuf(stdout,NULL); sleep(2);  /* kill buffering */
        printf("+++"); sleep(2);
        printf("ATH\r"); sleep(2);
        printf("ATDT %s\r",argv[1]);
        sleep(15);

        ttystuff.x_flags = ttystuff.sg_flags |= ECHO; /* stty echo */
        ioctl( 0, TIOCSETP, &ttystuff);

        ioctl( 0, TIOCLBIC, &nohup); /* stty hang */

        setcarr(NORMAL); /* normal carrier detect */

        execl( "/bin/login", "login", (char *) 0 );
}

setcarr(enable)
int enable;
{
        int     ifd, iunit, action, carrbit;
        char    buf[32];
        struct  stat sbuf;

        if (fstat(0, &sbuf) == -1)
                return (-1);

        action = (enable == NORMAL) ? ITPCLEARCARR : ITPSETCARR;

        iunit = minor(sbuf.st_rdev) >> 4;
        sprintf(buf, "/dev/itp%d", iunit);

        carrbit = 1 << (minor(sbuf.st_rdev) & 0xf);

        if ((ifd = open(buf, 2)) == -1)
                return(-1);

        if (ioctl(ifd, action, &carrbit) == -1)
                return(-1);

        close(ifd);
        return(0);
}
