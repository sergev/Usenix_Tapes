/*
 *	halt - halt the system
 *
 *		The system can be halted in two ways:
 *		  1.  anywhere, anytime by the super-user
 *		  2.  from the console by user Nobody if noone is logged on.
 *
 *		Bill Shannon - CWRU - 21 Feb 79
 */

main (argc, argv)
int     argc;
char  **argv;
{
	if ((getuid () & 0377) != 0 && (ttyn (0) != '8' || users () > 0))
		printf ("No permission.\n");
	else
		halt ();
}
