/*
**	hold - program to copy standard input into a file
**	and rename it to the first argument after end of file
**	useful for putting output of programs like tr back
**	into the original file
**			Ken Yap, Sydney University, March 1982.
*/
#include	<stdio.h>
#include	<signal.h>

char tmpname[64];
char buffer[BUFSIZ];

main(argc,argv)
	int argc;
	char **argv;{

	if(argc < 2){
		fprintf(stderr,"Usage: %s file\n",argv[0]);
		exit(1);
		}
	hold(argv[1]);
	exit(0);
}

hold(name)
	char *name;{
	register char *p;
	register int length,fd;
	int (*oldsig)();
	char *mktemp(),*rindex();
	int read(),write(),link();
	extern catch();
	(int (*)()) signal();

	if((p = rindex(name,'/')) == NULL)
		length = 0;
	else
		length = p - name + 1;
	strncpy(tmpname,name,length);
	strcpy(tmpname + length,mktemp("holdXXXXXX"));
#ifdef	DEBUG
	fprintf(stderr,"tmpname is %s\n",tmpname);
#endif
	if((oldsig = signal(SIGINT,SIG_IGN)) == SIG_DFL)
		signal(SIGINT,catch);
#ifdef	DEBUG
	fprintf(stderr,"oldsig is %d\n",oldsig);
#endif
	if((fd = creat(tmpname,0644)) < 0){
		perror(name);
		exit(2);
		}
	while((length = read(0,buffer,BUFSIZ)) > 0)
		write(fd,buffer,length);
	close(fd);
	signal(SIGINT,SIG_IGN);
	unlink(name);
	if(link(tmpname,name) < 0){
		perror(name);
		exit(3);
		}
	unlink(tmpname);
}

catch(){

	unlink(tmpname);
	exit(0);
}
