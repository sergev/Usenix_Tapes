/* del command: deletes file arguments after waiting for newline
 * J. Gillogly 27 Nov 75
 */
main(narg,args)
int narg;
char **args;
{       register char **aptr;
	for (aptr = args+1; *aptr != -1; aptr++)/* until end of arg list */
		printf("%s\n",*aptr);           /* print each filename */
	if (getchar()=='n') exit(0);    /* almost any character from user */
	args[narg]=0;                           /* was -1 */
	execv("/bin/rm",args);                  /* send the list to rm */
	printf("can't find /bin/rm\n");
}
