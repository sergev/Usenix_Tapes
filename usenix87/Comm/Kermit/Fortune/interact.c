/*      interact.c      modules added to kermit for
                        interactive use.
*/

#include <ctype.h>
#include "kermit.h"

char ans        [128];
char text       [128];
char names      [128];
char *files     [128];

interact ()

{

        while ( 1 )
        {

                ioctl(0,TIOCSETP,&cookedmode);  
                str0 ( ans , 128 );

                printf (  "Kermit 32:16 ->" );
                fgets ( ans , 128 , stdin );
                switch ( ans[0] )
                {

                case 'B' :
                case 'b' :
                        debug = !debug;
                        if ( debug )
                        {
                                printf ( "Debug mode on\n" );
                        }
                        else
                            {
                                printf ( "Debug mode off\n" );
                        }
                        break;

                case 'C' :
                case 'c' :      /* connect command */
                        switch ( ans [1] )
                        {

                        case 'D' :
                        case 'd' :
                                move_dir ();
                                break;

                        default :
                                if ( !remfd ) open_port ();
                                connect ();
                                break;
                        }
                        break;

                case 'D':
                case 'd':
                        quit_port ();
                        close ( remfd );
                        remfd = 0;
                        break;


                case 'H':
                case 'h' :
                case '?' :

                        help ();
                        break;


                case 'Q':
                case 'q' :      /* quit and get out */

                        return;
                        break;

                case 'S':
                case 's' :      /* send command */
                        switch ( ans [1] )
                        {

                        case 'H':
                        case 'h':
                                showme ();
                                break;
                        case 'E':
                        case 'e':
                                switch ( ans [2] )
                                {
                                case 'T':
                                case 't' :
                                        set_up ();
                                        break;
                                case 'N':
                                case 'n' :
                                        send_files ();
                                        break;
                                }
                                break;
                        default:
                                send_files ();
                                break;
                        }
                        break;
                case 'R':
                case 'r' :      /* receive command */

                        if (recsw() == FALSE)
                                printf("Receive failed.\n");
                        else
                            printf("OK\n");
                        break;
                case 'V':
                case 'v' :
                        if ( (eol=='\n') || (eol==0) )
                        {
                                eol = '\r';
                                printf ( "Vax mode on\n" );
                        }
                        else
                            {
                                eol = '\n';
                                printf ( "Vax mode off\n" );
                        }
                        break;

                case '!':

                        system ( shell );
                        break;

                default:
                        system ( ans );
                        break;
                }
        }
}
help ()
{
        printf ( "\tb\ttoggle Debug mode\n" );
        printf ( "\tc\tconnect to remote system\n" );
        printf ( "\tcd\tunix level change directory\n" );
        printf ( "\td\tdisconnect current port\n" );
        printf ( "\th\thelp\n" );
        printf ( "\tq\tquit or exit from Kermit\n" );
        printf ( "\tr\trecieve files\n" );
        printf ( "\tsend\tsend files\n" );
        printf ( "\tset [h]\tset Kermit parameters\n" );
        printf ( "\tshow\tshow Kermit parameters\n" );
        printf ( "\tv\ttoggle Vax mode\n" );
        printf ( "\t?\thelp\n" );
        printf ( "\t!\tenter unix shell\n" );
}
set_help ()
{
        printf ( "Set modes available\n\n" );
        printf ( "port\t\tunix device to use\n" );
        printf ( "shell\t\tunix shell to use\n" );
        printf ( "eol\t\tend of line character\n" );
        printf ( "baud\t\tbaud rate to use\n" );
        printf ( "debug\t\tturn debug mode on or off\n" );
        printf ( "esc\t\tset escape character [octal]\n" );
        printf ( "padchar\t\tset padding character [octal]\n" );
        printf ( "padcount\tset number of pads to use [decimal]\n" );
        printf ( "quote\t\tset quote character [octal]\n\n" );
}
send_files ()
{
        FILE *point;
        FILE *popen ();
        char *malloc ();

        int count;

        count = 0;

        str0 ( names , 128 );
        sscanf ( ans , "%s %128c" , text , names );

        if ( strlen ( names ) == 0 )
        {
                printf ( "Which files please ? " );
                fgets ( names , 128 , stdin );
                if ( strlen ( names ) == 0 ) return;
        }

        strcpy ( text , "/bin/ls -1 " );
        strcat ( text , names );

        point = popen ( text , "r" );
        while ( (fgets (names,128,point)) != NULL )
        {
                names[strlen(names)-1]=0;
                files [count] = malloc ( strlen (names) );
                strcpy ( files[count] , names);
                count ++;
                files[count] == 0;
                if ( count > 127 ) break;
        }
        pclose ( point );
        count --;

        filelist = files;
        if (sendsw() == FALSE)
                printf("Send failed.\n");
        else
            printf("OK\n");
        for ( ;count>=0;count--) 
        {
                free ( files[count] );
                files [count] = 0;
        }

}
showme ()
{
        printf ( "Kermit current settings\n" );
        printf ( "port\t= %s\n" , remtty );
        printf ( "shell\t= %s\n" , shell );
        printf ( "eol\t= %o" , eol );
        if ( eol == '\n' ) printf ( " Normal mode is on\n" );
        if ( eol == '\r' ) printf ( " Vax mode is on\n" );
        if ( (eol!='\n') && (eol!='\r') ) printf ( " Unknown eol mode\n" );
        printf ( "%d baud\n" , speed );
        if ( debug ) printf ( "Debugging is on\n" );
        printf ( "%o is the escape char\n" , escchr );
        printf ( "%o is the padchar\n" , padchar );
        printf ( "%d is the padcount\n" , pad );
        printf ( "%o is the quote char\n" , quote );

}
set_up ()
{

        int len;

        str0 ( names , 128 );
        sscanf ( ans , "%s %128c" , text , names );
        str0 ( ans   , 128 );
        str0 ( text  , 128 );
        sscanf ( names , "%s %128c" , ans , text );

        len = strlen ( text );
        len --;
        if ( text[len] == '\n' ) text [len] = 0;

        switch ( ans [0] )
        {
        case '?' :
        case 'h' :
                set_help ();
                return;


        case 's' :
                strcpy ( shell , text );
                break;

        case 'e' :
                switch ( ans [1] )
                {
                case 'o' :
                        sscanf ( text , "%o" , &len );
                        eol = len;
                        break;
                case 's' :
                        sscanf ( text , "%o" , &len );
                        escchr = len;
                        break;
                }
                break;

        case 'd' :
                switch ( text[1] )
                {
                case 'n':
                        debug = 1;
                        break;
                case 'f':
                        debug = 0;
                        break;
                default :
                        printf ( "Unknown debug mode\n" );
                        break;
                }
                break;

        case 'b' :
                speed = atol ( text );
                break;

        case 'q' :
                sscanf ( text , "%o" , &len );
                quote = len;
                break;

        case 'p' :
                switch ( ans [4] )
                {
                case 'h' :
                        sscanf ( text , "%o" , &len );
                        padchar = len;
                        break;
                case 'o' :
                        pad = atol ( text );
                        break;
                }
                switch ( ans [1] )
                {
                case 'o' :
                        strcpy ( remtty , text );
                        break;
                }
                break;

        default :
                printf ( "Unknown set command\n" );
                break;
        }
        showme ();
}
lpack ( itext )
char itext[];
{
        int iam;
        int jam;
        int kam;
        int len;

        len = strlen ( text );

        for ( iam = 0 ; iam < len ; iam++ )
        {
                if ( isspace ( itext[iam] ) == 0 )
                {
                        jam = iam;
                        break;
                }
        }
        for ( iam = jam, kam = 0; iam < len; iam++ , kam++ )
                itext [kam] = itext[iam];
        kam = strlen ( itext );
        kam = kam - jam;
        for ( iam = kam ; iam < len ; iam++ )
                itext[iam] = 0;
}
move_dir ()
{

        sscanf ( ans , "%s %s" , text , names );
        chdir ( names );
}
str0 ( string , len )

char string [];

int len;

{

        len --;
        for ( ; len >= 0 ; len-- ) string [len]  = 0;

}
