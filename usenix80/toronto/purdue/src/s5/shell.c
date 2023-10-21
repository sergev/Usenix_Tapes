/*	Subroutine "shell":
 *
 *
 *	This subroutine will perform a combination of a 
 *	"fork" and "execl" system call to execute system
 *	calls from C in an easy manner.
 *
 *	John Bruner	April 5, 1977
 *
 */
shell(cmd)
char	cmd[];
{
	register int pid;

	if ((pid=fork()) < 0){
		write(2,"Couldn't fork--try again\n",25);
		return(-1);
	}

	if (pid != 0) wait();
		else {
			execl("/bin/sh","sh","-c",cmd,0);
			write(2,"Can't execute \"sh\"\n",19);
			exit();
		}
	return(0);
}
