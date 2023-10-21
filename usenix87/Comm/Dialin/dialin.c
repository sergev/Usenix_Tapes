/*----------------------------------------------------------------
**
** dialin.c	-- create uucp LCK file for a dialin line.
**
** The 7300 has an annoying habit of blasting a dialed-in user
** when it wants the UUCP phone line.  Running this program out
** of the profile prevents that from happening by creating a
** /usr/spool/uucp/LCK..ph? file for the line.  A child process
** is left sleeping, and when the phone is hung up by the caller,
** the lock file is removed.
**
** Compile:
**		cc -O -c dialin.c
**		ld /lib/crt0s.o /lib/shlib.ifile dialin.o -o dialin
**		strip dialin
**		cp dialin /usr/bin		
**
** Edit /etc/profile and add the line:
**
**		/usr/bin/dialin
**
** to the block that asks what kind of terminal is being used, BEFORE
** the 'while true' loop is started.
**
** Author:
**	David Brower,  {ucbvax!mtxinu,cbosgd,amdahl}!rtech!gonzo!daveb
**
** Rights:
**	No rights reserved.  This program is placed in the public domain.
*/


# include <stdio.h>
# include <signal.h>

bye()
{
	/* a null signal catcher */
}

main()
{
	char cmd[80];
	char ttydev[ 20 ];
	char * strrchr();
	char * ttyname();
	char * tty;
	
	strcpy( ttydev, ttyname());
	tty = strrchr( ttydev, '/' ) + 1;
	printf("locking %s until HUP\n", ttydev );
	sprintf( cmd, "echo $LOGNAME > /usr/spool/uucp/LCK..%s\n", tty );
	system( cmd );
	sprintf( cmd, "rm /usr/spool/uucp/LCK..%s\n", tty );

	if( !fork() ) 
	{
		/* wait for HUP and remove lock file */
		(void) signal( SIGHUP, bye );		
		pause();
		system( cmd );
	}
	exit( 0 );
}

