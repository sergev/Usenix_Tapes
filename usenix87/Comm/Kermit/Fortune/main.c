#include "kermit.h"
/*
 *      K e r m i t
 *
 *      Main routine - parse command and option, set up the
 *      tty lines and dispatch to the appropriate routine.
 */

int ttyflg;

main(argc,argv)
int argc;
char **argv;
{
        char *cp;
        int cflg;
        int rflg;
        int sflg;
        int iflg;

        if (argc == 1) usage();

        cp = *++argv; 
        argv++; 
        argc -= 2;
        speed=cflg=sflg=rflg=iflg=ttyflg=0; /* turn off all flags */

        strcpy ( shell , "/bin/sh");
        strcpy ( remtty , "/dev/cul0" ); /* default device */
        escchr = BRKCHR;                /* default esc char */
        sleep_len = 5;                  /* default sleep on send */
        eol = MYEOL;
        quote = MYQUOTE;

        while ((*cp) != NULL)
                switch (*cp++)
                {
                case '-': 
                        break;
                case 'c': 
                        cflg++; 
                        break;                  /* connect command */
                case 's': 
                        sflg++; 
                        break;                  /* send command */
                case 'r': 
                        rflg++; 
                        break;                  /* receive command */
                case 'h':                       /* help detail */
                        khelp ();
                        exit ();
                        break;
                case 'i': 
                        iflg++; 
                        break;          /* interactive mode */
                case 'e': 
                        if (argc--)     /* specify escape char */
                                escchr = atoi(*argv++);
                        /*  as ascii decimal number */
                        else usage();
                        if (debug) 
                                fprintf(stderr,
                                "escape char is ascii %d\n",escchr);
                        break;
                case 'l': 
                        if (argc--) /* specify tty line to use */
                        {
                                strcpy ( remtty , *argv );
                                *argv++;
                        }
                        else usage ();
                        if (debug) fprintf(stderr,"line %s\n",remtty);
                        ttyflg++;
                        break;
                case 'b': 
                        if (argc--) /* specify baud rate */
                                speed = atoi(*argv++);
                        else usage();
                        if (debug) fprintf(stderr,"speed %d\n",speed);
                        break;
                case 'd': 
                        debug = TRUE; 
                        break;            /* debug mode */
                case 'v':
                        eol = '\15';
                        break;
                }
        if (debug)
        {
                if (cflg) fprintf(stderr,"cflg ");
                if (sflg) fprintf(stderr,"sflg ");
                if (rflg) fprintf(stderr,"rflg ");
                fprintf(stderr,"line %s, speed %d\n",remtty,speed);
        }
        /* only one command allowed, for now */
        if ( iflg ) sflg=rflg=0;
        if ((cflg+sflg+rflg+iflg) != 1) usage();


        /* save tty mode so we can restore it later */
        ioctl(0,TIOCGETP,&cookedmode);  
        ioctl(0,TIOCGETP,&rawmode);     

        open_port ();

        if (iflg) ttyflg++;
        if (cflg) connect();                  /* connect command */
        if (iflg) interact ();
        if (sflg)                             /* send command */
        {
                if (argc--) filnam = *argv;
                else usage();
                printf ( "Sleeping for %d seconds\n" , sleep_len );
                sleep ( sleep_len );
                filelist = argv;
                if (host) ioctl(0,TIOCSETP,&rawmode);
                if (sendsw() == FALSE)
                        printf("Send failed.\n");
                else
                    printf("OK\n");
                if (host) ioctl(0,TIOCSETP,&cookedmode);
        }
        if (rflg)                             /* receive command */
        {
                if (host) ioctl(0,TIOCSETP,&rawmode);
                if (recsw() == FALSE)
                        printf("Receive failed.\n");
                else
                    printf("OK\n");
                if (host) ioctl(0,TIOCSETP,&cookedmode);
        }
        quit_port ();
}
usage()
{
        fprintf(stderr,
        "usage:\tkermit [csri][dh][lbe] [line] [baud] [char] [files]\n");
        fprintf(stderr,
        "\tkermit h <for help detail>\n" );
        quit_port ();
        exit();
}
khelp ()
{
        printf ( "usage:\tkermit [csri][dhv][lbe] [line] [baud] [char] [files]\n\n" );

        printf ( "\tc\tconnect command to go to another machine\n" );
        printf ( "\ts\tsend command to act as server kermit\n" );
        printf ( "\tr\trecieve command to act as server kermit\n" );
        printf ( "\ti\tinteractive mode: defeats c,s,and r\n" );
        printf ( "\t\tKermit in a terminal emulator mode\n" );
        printf ( "\t\twith the ability to do many things\n" );
        printf ( "\t\tenter kermit in this mode and ask for help\n" );
        printf ( "\td\tturns on debugging output\n" );
        printf ( "\th\tprints this message <defeats everything else>\n" );
        printf ( "\tv\tsets end of line to <CR> for Vax\n" );
        printf ( "\tl\tuse with <line> for the connect device\n" );
        printf ( "\tb\tuse with <baud> for the baud rate\n" );
        printf ( "\te\tuse with <char> decimal value of the escape\n" );
        printf ( "\t\tcharacter now set at 33 for escape\n" );
        printf ( "\tfiles\ta list of files to act on\n\n" );
}
set_faults ()
{
        extern quit_kermit ();

        signal ( SIGHUP , quit_kermit );
        signal ( SIGINT , quit_kermit );
        signal ( SIGQUIT, quit_kermit );
        signal ( SIGFPE , SIG_IGN     );
        signal ( SIGALRM, quit_kermit );
        signal ( SIGTERM, quit_kermit );
}
static char lockname [48];
get_port ( remtty )

char *remtty;

{

        struct stat buff;

        char *cc;
        char *rindex ();

        FILE *temp;

        strcpy ( lockname , "/usr/spool/uucp/LCK.." );

        cc = rindex ( remtty , '/' );
        if ( cc )
        {
                cc++;
                strcat ( lockname , cc );
        }
        else
            {
                strcat ( lockname , remtty );
        }


        while ( 1 )
        {
                if ( stat ( lockname , buff ) )
                {
                        temp = fopen ( lockname , "w" );
                        fclose ( temp );
                        break;
                }
                printf ( "Port %s is busy... sleeping 5 seconds\n" , remtty );
                sleep ( 5 );
        }
}
quit_port ()
{
        if ( !remfd ) return;
        unlink ( lockname );
        unlink ( lockname );
        return;
}
quit_kermit ()
{
        quit_port ();
        exit (0);
}
open_port ()
{
if ( ttyflg )
        {
        get_port ( remtty );
        set_faults ();
        remfd = open(remtty,2); /* we're a remote kermit, so */
        if (remfd < 0)          /*  set up the tty line */
        {
                fprintf(stderr,
                "Kermit: cannot open %s\n",remtty);
                exit(-1);
        }
        host = FALSE;
}
        else
        {
                remfd = 0;     /* we're a host kermit */
                host = TRUE;
        }
        /* rawmode is used during connect, send, receive */
        rawmode.sg_flags |= (RAW|TANDEM);
        rawmode.sg_flags &= ~(ECHO|CRMOD);

        /* if remote kermit, get remote tty mode */
        ioctl(remfd,TIOCGETP,&oldmode);      
        ioctl(remfd,TIOCGETP,&remttymode);      
        remttymode.sg_flags |= (RAW|TANDEM);
        remttymode.sg_flags &= ~(ECHO|CRMOD);
        if (speed)                    /* user specified a speed? */
        {
                switch(speed)         /* get internal system code */
                {
                case 110: 
                        speed = B110; 
                        break;
                case 150: 
                        speed = B150; 
                        break;
                case 300: 
                        speed = B300; 
                        break;
                case 1200: 
                        speed = B1200; 
                        break;
                case 2400: 
                        speed = B2400; 
                        break;
                case 4800: 
                        speed = B4800; 
                        break;
                case 9600: 
                        speed = B9600; 
                        break;
                default: 
                        fprintf(stderr,"bad line speed\n");
                }
                remttymode.sg_ispeed = speed;
                remttymode.sg_ospeed = speed;
        }
        ioctl(remfd,TIOCSETP,&remttymode);
}
