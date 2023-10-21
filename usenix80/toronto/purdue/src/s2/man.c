/* man - shuffle args and call "help" */
/*	to provide compatability with UNIX documentation */

main(argc, argv)
int argc;
char *argv[];
{
	if(argc == 2) {
		execl("/usr/bin/help","help",argv[1],0);
		goto out;
	}
	if(argc == 3) {
		execl("/usr/bin/help","help",argv[2],argv[1],0);
		goto out;
	}
	printf("syntax is:\nman <chapter> <name>\n");
	exit();

out:
	printf("cannot execute /usr/bin/help\n");
}
