/*
** TERMWIDTH - Sets the external variable `Termwidth' to the # of columns
** on the terminal.
*/
termwidth ()
{
	register char           *termtype ;
	register int            twidth ;
	char                    buf[ 1025 ] ;
	extern unsigned         Termwidth ;
	char                    *getenv() ;

	Termwidth = 80 ;
	if ( !(termtype = getenv( "TERM" )) )
		return ;
	if ( tgetent( buf, termtype ) != 1 )
		return ;
	twidth = tgetnum( "co" ) ;
	if ( twidth > 40 )
		Termwidth = twidth ;
}
