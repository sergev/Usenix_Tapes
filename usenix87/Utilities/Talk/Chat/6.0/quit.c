/* QUIT - Written By Sanford L. Barr for the Chat System 1985 */
/* quit.c :=: 9/25/85 */

#include "chat.h"

quit()		/* Exit	chat */
{

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	send(2);			/* Say goodbye */

	if (unlink(myfile) == ERR)	/* Clear out msg file */
		perror(myfile);

	trestore();			/* Back	to normal */

	puts("\n<*End of Chat Session*>");

	if (ridname() == ERR)		/* Take	my name	off the	list */
		exit(1);


	if (close(lfile) == ERR) {	/* Close up the	LOGFILE	*/
		perror(LOGFILE);
		exit(1);
	}
	if (stat(LOGFILE, &sbuf) == ERR) {
		perror(LOGFILE);
		exit(1);
	}
	if (sbuf.st_size == 0) {
		if (unlink(LOGFILE) == ERR) {
			perror(LOGFILE);
			exit(1);
		}
	}
	exit(OK);			/* Goodbye! */
}

