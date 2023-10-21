/* inivartbl.c
*
*	This routine initializes a list which contains the commands that
*	can be used with the '%' command. This list will be searched to 
*	find which command is being used.
*
*			Programmer: G. Skillman
*/
inivartbl()
{
	extern char	*VARTABLE[];
	extern int	EOT;

	VARTABLE[0] = "MACRO";
	VARTABLE[1] = "TRACE";
	VARTABLE[2] = "NFLAG";
	EOT = 2;

}
