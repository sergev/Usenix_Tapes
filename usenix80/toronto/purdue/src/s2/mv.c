/*
mv [-f] file1 file2

where -f suppresses the question if the file is write-protected.
--kt 02/28/78


if filename is '-f', can still be moved.

unlink file2
link file1 file2
unlink file1
*/
int stbuf[42];
struct sbuf {
int dev;
int inum;
int imode;
char nlink;
char uid1;
char uid0;
char siz0;
char siz1;
int addr[8];
int adate[2];
int mdate[2];
};
char strbuf[70];
int	temp;

main(argc,argv)
char **argv;
{
char *argv1, *argv2, *argv3, *argv4;
char *p, *p1, *p2;
char place[100];
int i, fflag;
int status;
int b;
/*
	check for correct number
	of arguments
*/

fflag = 0;

if(argv[1][0] == '-' && argv[1][1] == 'f' && argc == 4){
	fflag++;
	argv++;
}else{
if(argc != 3){
	printf("Usage: mv [-f] name1 name2\n");
	exit();
}
}
/*
	is there anything to do?
*/
argv3 = argv[1];
argv4 = argv[2];
if(stat(argv[1], stbuf) < 0){
	write(1,"Source file non-existent\n",25);
	exit();
}
/*
	yes, there is a source.
	check whether file or directory
*/
if((stbuf[0].imode & 060000) == 040000){
/*
	The source is a directory, so
	we do lots of checking and
	messing around so as not to get
	into trouble.  This patch of
	code contains administrative
	policies rather than system
	restrictions.
*/

/* restrict dir renames to su, so one cant mv /etc around -ghg 5/22/77 */
	if(getuid()) {
		printf("Directory rename requires Super-user permission.\n");
		exit();
	}
/* ......................................................................*/
	if(stat(argv[2], stbuf) >= 0){
		write(1,"Directory target exists.\n",25);
		exit();
	}
	argv1 = argv[1];
	argv2 = argv[2];
	while(*argv1 == *argv2){
		argv1++;
		if(*argv2++ == 0){
			write(1,"???\n",4);
			exit();
		}
	}
	while(*argv1)if(*argv1++ == '/'){
		write(1,"Directory rename only\n",22);
		exit();
	}
	while(*argv2)if(*argv2++ == '/'){
		write(1,"Directory rename only\n",22);
		exit();
	}
	if(*--argv1 == '.'){
		printf("can't change . \n");
		exit();
	}
}else{
/*
	the source is a file.
*/
	setuid(getuid());
	if(stat(argv4, &stbuf[2]) >= 0){
		if((stbuf[2].imode & 060000) == 040000){
			argv2 = strbuf;
			while(*argv2++ = *argv4++);
			argv2[-1] = '/';
			argv4 = argv[1];
			argv1 = argv[1];
			while(*argv4)
				if(*argv4++ == '/')
					argv1 = argv4;
			while(*argv2++ = *argv1++);
			argv4 = strbuf;
		}
		if(stat(argv4, &stbuf[2]) >= 0){
			if((stbuf[0]==stbuf[2]) && (stbuf[1]==stbuf[3])){
				write(1,"Files are identical.\n",21);
				exit();
			}
			temp = stbuf[2].uid0&0377 | stbuf[2].uid1<<8;
			if(getuid()  == temp)
				b = 0200; else
				b = 02;
			if((stbuf[2].imode & b) == 0) {
				if(!fflag){
				printf("%s is write-protected, are you sure? ",
					argv4);
				i = b = getchar();
				while(b != '\n' && b != '\0')
					b = getchar();
				if(i != 'y')
					exit();
				}
			}
			if(unlink(argv4) < 0){
				write(1,"Cannot remove target file.\n",27);
				exit();
			}
		}
	}
}
if(link(argv3, argv4) < 0){
	i = fork();
	if(i == -1){
		write(1,"Try again.\n",11);
		exit();
	}
	if(i){
		while(wait(&status) != i);
	}else{
		p = place;
		p1 = p;
		while(*p++ = *argv3++);
		p2 = p;
		while(*p++ = *argv4++);
		execl("/bin/cp","cp", p1, p2, 0);
		write(1, "no cp\n", 6);
		exit(1);
	}
	if((status & 0377) != 0){
		write(1,"?\n", 2);
		exit();
	}
	if(status != 0) exit();
}
if(unlink(argv3) < 0){
	write(1,"Cannot unlink source file.\n",27);
	exit();
}
}

putchar(c)
{
	write(1, &c, 1);
}

getchar()
{
	char c;

	if(read(0, &c, 1) != 1) return(0);
	return(c);
}
