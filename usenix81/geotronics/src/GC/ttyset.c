/*
	ttyset -- set terminal characteristics according to line number

	last edit: 03-Mar-1981  D A Gwyn

	line 1  VT100   (ADM3A plus hardware tabs)
	line h  Braille (VT100 but upper-case only)
	others  ADM3A   (basic dumb upper/lower case video terminal)
*/

#include        <sgtty.h>

extern char     *ttyname();
extern int      gtty(), strcmp(), stty();

static struct sgttyb    buf ;


main()
	{
	char    *dev = ttyname( 2 );

	if ( gtty( 2 , &buf ) < 0 )
		{
		perror( "/dev/tty" );
		exit( 1 );
		}
	buf.sg_erase = `H' - 0100 ; buf.sg_kill = `U' - 0100 ;
	if ( strcmp( dev , "/dev/tty1" ) == 0 )
		{
		write( 2 , "% vt100\r\n" , 9 );
		buf.sg_flags = ECHO|CRMOD|ANYP ;
		}
	else if ( strcmp( dev , "/dev/ttyh" ) == 0 )
		{
		write( 2 , "% braille\r\n" , 11 );
		buf.sg_flags = LCASE|ECHO|CRMOD|ANYP ;
		}
	else    {
		write( 2 , "% adm3a\r\n" , 9 );
		buf.sg_flags = XTABS|ECHO|CRMOD|ANYP ;
		}
	if ( stty( 2 , &buf ) < 0 )
		{
		perror( "/dev/tty" );
		exit( 1 );
		}
	exit( 0 );
	}
