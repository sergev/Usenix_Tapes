/*
 * su.c - become super-user
 *
 *	modified 01-Nov-1980 by D A Gwyn:
 *	1) fixed bug on bad password file;
 *	2) create entry in "/etc/xtmp" for bad password;
 *	3) fixed buffer overflow bug;
 *	4) V7 `crypt' now used;
 *	5) "-sh" passed to shell instead of "-".
 */

char	pwbuf[100];
int	ttybuf[3];
struct	{
	char		name[8];
	char		tty;
	char		tfill;
	int		time[2];
	int		fill;
	char		password[100];
	}	xtmp;

main()
{
	register int	f;
	register char *p, *q;
	extern fin;

	if(getpw(0, pwbuf))
		goto badpw;
	(&fin)[1] = 0;
	p = pwbuf;
	while(*p != ':')
		if(*p++ == '\0')
			goto badpw;
	if(*++p == ':')
		goto ok;
	gtty(0, ttybuf);
	ttybuf[2] =& ~010;
	stty(0, ttybuf);
	printf("password: ");
	q = xtmp.password;
	f = 98;
	while((*q = getchar()) != '\n')
		{
		if ( *q == '\0' )
			exit( 1 );
		if ( f )
			{
			f--;
			q++;
			}
		}
	*q = '\0';
	ttybuf[2] =| 010;
	stty(0, ttybuf);
	printf("\n");
	q = crypt( xtmp.password , p );	/* p -> salt */
	while(*q++ == *p++);
	if(*--q == '\0' && *--p == ':')
		goto ok;
	/* bad password - make an entry in /etc/xtmp */
	if ( (f = open( "/etc/xtmp" , 1 )) >= 0 )
		{
		seek( f , 0 , 2 );	/* seek EOF */
		for ( q = xtmp.name , p = "---su---" ; *p ; )
			*q++ = *p++;
		xtmp.tty = ttyn( 0 );
		time( xtmp.time );
		write( f , &xtmp , 24 );	/* make a note */
		close( f );
		}
	goto error;

badpw:
	printf("bad password file\n");
	goto error;
ok:
	setuid(0);
	execl("/bin/sh", "-sh", 0);
	printf("cannot execute shell\n");
error:
	printf("sorry\n");
	exit( 1 );
}
