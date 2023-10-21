#include "kermit.h"
/*
 *      c o n n e c t
 *
 *      establish a virtual terminal connection
 *      with the remote host, over a tty line.
 *
 */
int done;
int divert;
int unit;
int crlf;
int screen;
int count;
int len;

char cc         [1024];
char file0      [128];
char file1      [128];

FILE *popen ();
FILE *cunit;

jmp_buf oops;

connect()
{

        extern int con_oops ();
        extern int quit_kermit ();

        char c;

        if (host)        /* if in host mode, nothing to connect to */
        {
                fprintf(stderr,
                        "Kermit: host mode nothing to connect to\n");
                ioctl(0,TIOCSETP,&cookedmode);
                return;
        }

        signal ( SIGINT , con_oops );

        printf("Remote.\r\n");

        setjmp (oops);

        ioctl (0,TIOCSETP,&rawmode);

        c = done = divert = 0;
        screen = 1;

        while (c != escchr)
        {
                ioctl ( 0     , FIONREAD , &count );
                if ( count )
                {
                read  ( 0     , &c       , 1      );
                if ( c == '\1' ) eval (c);
                else write ( remfd , &c       , 1      );
                if ( done ) break;
                }
                if ( done ) break;
                read_rem ();
        }

        ioctl(0,TIOCSETP,&cookedmode);

        printf("\nLocal.\n");
        fclose ( cunit );
        signal ( SIGINT , quit_kermit );
        return;
}

eval (c0)
char c0;
{

        int err;
        char c;

        read ( 0 , &c , 1 );
        ioctl(0    ,TIOCSETP,&cookedmode);
        switch ( c )
        {
                case '@':       help_con ();
                                break;

                case 'a':       done = 1;
                                break;

                case 'b':       ioctl ( 0 , TIOCSBRK , 0 );
                                break;

                case 'c':       
                                printf ( "Local shell\n" );
                                system ( "/bin/sh" );
                                printf ( "Remote.\n" );
                                break;

                case 'd':       printf ( "Local shell name ? " );
                                gets ( file0 );
                                if ( !strlen(file0) ) break;
                                cunit = popen (file0,"r");
                                if ( cunit == NULL ) break;
                                while (fgets(cc,sizeof(cc),cunit) 
                                        != NULL ) {
                                len = strlen ( cc );
                                write ( remfd , cc , len );
                                read_rem (); }
                                pclose ( cunit );
                                break;

                case 'e':       printf ( "Command  name ? " );
                                gets ( file0 );
                                if ( !strlen(file0) ) break;
                                remcomx ();
                                break;

                case 'f':       if ( divert ) {
                                close ( divert );
                                divert = 0;
                                printf ( "Divert closed\n" );
                                } else {
                                printf ( "Divert filename ? " );
                                gets ( file0 );
                                if ( !strlen(file0) ) break;
                                divert = creat ( file0 , 0644 );}
                                break;

                case 'g':       screen = !screen;
                                if ( screen ) printf ( "Screen on\n" );
                                else printf ( "Screen off\n" );
                                break;

                case 'h':       if ( divert ) {
                                close ( divert );
                                divert = 0;
                                printf ( "Divert closed\n" );
                                } else {
                                printf ( "Append divert filename ? " );
                                gets ( file0 );
                                if ( !strlen(file0) ) break;
                                divert = open  ( file0 , 1 );
                                if ( !divert )
                                  divert = creat ( file0 , 0644 );
                                lseek ( divert , 0 , 2 );
                                break; }

                case 'i':       printf ( "Take file [from] [to] :" );
                                gets ( cc );
                                if ( !strlen(cc) ) break;
                                err=sscanf ( cc,"%s %s",file0,file1 );
                                if ( err == 0 ) break;
                                if ( err == 1 ) strcpy ( file1,file0 );
                                take_file ();
                                break;

                case 'j':       printf ( "Send file [from] [to] :" );
                                gets ( cc );
                                if ( !strlen(cc) ) break;
                                err=sscanf ( cc,"%s %s",file0,file1 );
                                if ( err == 0 ) break;
                                if ( err == 1 ) strcpy ( file1,file0 );
                                send_file ();
                                break;

                case 'k':       printf ( "Transmit file :" );
                                gets ( cc );
                                if ( !strlen(cc) ) break;
                                err=sscanf ( cc,"%s %s",file0,file1 );
                                if ( err == 0 ) break;
                                strcpy ( file1,file0 );
                                tran_file ();
                                break;

                case 'l':       crlf++;
                                crlf = crlf % 5;
                                printf ( "Line termination set to " );
                                if ( crlf == 0 ) printf ( "CR\n" );
                                if ( crlf == 1 ) printf ( "NL\n" );
                                if ( crlf == 2 ) printf ( "RAW\n" );
                                if ( crlf == 3 ) printf ( "CRLF\n" );
                                if ( crlf == 4 ) printf ( "LFCR\n" );
                                break;

                default:        if ( screen ) {
                                write ( remfd , &c0 , 1 );
                                write ( remfd , &c  , 1 );
                                }
                                break;
        }
        ioctl (0    ,TIOCSETP,&rawmode);
}
remcomx ()
{

        strcpy ( file1 , file0 );
        cunit = fopen ( file0 , "r" );
        if ( cunit == NULL ) return;

        strcpy ( cc , "stty -echo;cat >" );
        strcat ( cc , file1 );
        strcat ( cc , ";stty echo;sh " );
        strcat ( cc , file1 );
        strcat ( cc , ";rm " );
        strcat ( cc , file1 );
        strcat ( cc , "\r" );
        len = strlen ( cc );
        write ( remfd , cc , len );
        transfer (1);
        fclose ( cunit );
}
send_file ()
{

        cunit = fopen ( file0 , "r" );
        if ( cunit == NULL ) return;

        strcpy ( cc , "stty -echo;cat > " );
        strcat ( cc , file1 );
        strcat ( cc , ";stty echo\r" );
        len = strlen ( cc );
        write ( remfd , cc , len );
        transfer (1);
        fclose ( cunit );
}
take_file ()
{
char c;
        c = 0;
        unit = creat ( file1 , 0644 );
        if ( unit == -1 ) return;

        strcpy ( cc , "echo -n \2;cat " );
        strcat ( cc , file0 );
        strcat ( cc , ";echo -n \3\3\3\3\3\r" );
        len = strlen ( cc );
        write ( remfd , cc , len );
        while ( c != 2 ) read ( remfd , &c , 1 );
        done = 0;
        while ( 1 )
        {
        ioctl ( remfd , FIONREAD , &count );
        if ( count > sizeof (cc) ) count = sizeof (cc);
        read  ( remfd , cc       , count  );
        for ( len = 0 ; len < count ; len ++ )
        {
                c = cc[len] & 0177;;
                if ( c == '\r' ) continue;
                if ( c == 3    ) {done++;break;}
                write ( unit , &c , 1 );    
        }
        if ( done ) break;
        }
        done = 0;
        close ( unit );
        read_rem ();
}
tran_file ()
{

        cunit = fopen ( file0 , "r" );
        if ( cunit == NULL ) return;
        transfer (0);
        fclose ( cunit );
}
transfer (logic)
int logic;
{

int num;
char c;

        num = 0;
        sleep ( 2 );
        read_rem ();
        while ( (c=fgetc(cunit)) != EOF )
        {
                if ( c=='\r' || c=='\n' ) switch ( crlf )
                        {
                        case 0: c = '\r';
                                break;
                        case 1: c = '\n';
                                break;
                        case 2: break;
                        case 3: c = '\r';
                                write ( remfd,&c,1 );
                                c = '\n';
                                break;
                        case 4: c = '\n';
                                write ( remfd,&c,1 );
                                c = '\r';
                                break;
                        }
                write ( remfd , &c , 1 );       
                num++;
                if ( num % 512 == 0 ) read_rem ();
        }
        c = '\04';
        if ( logic ) write ( remfd , &c , 1 );
        read_rem ();
        sleep ( 1 );
}
read_rem ()
{
char c;

        ioctl ( remfd , FIONREAD , &count );
        if ( count > sizeof (cc) ) count = sizeof (cc);
        read  ( remfd , cc       , count  );

        if ( screen )
        write ( 1     , cc       , count  );

        if ( divert )
        {
        poff ( cc , count );
        write ( divert, cc       , count  );
        }
}
poff ( text , len )

char *text;
int len;

{
while ( len-- )
        {
        *text &= 0177;
        text++;
        }
}
help_con ()
{
        printf ( "Function Keys:\n\n" );
        printf ( "\tHelp\t\tHelp\tDisplay toggle\tf7\n" );
        printf ( "\tQuit emulator\tf1\tAppend divert\tf8\n" );
        printf ( "\tBreak\t\tf2\tReceive file\tf9\n" );
        printf ( "\tLocal shell\tf3\tSend file\tf10\n" );
        printf ( "\tLcl shl rmt dsp\tf4\tTransmit file\tf11\n" );
        printf ( "\tRemote cmd xdt\tf5\tToggle crlf\tf12\n" );
        printf ( "\tDivert\t\tf6\n\n" );
}
con_oops ()
{
        close  ( divert );
        close  ( unit   );
        fclose ( cunit  );
        longjmp ( oops , 1 );
}
