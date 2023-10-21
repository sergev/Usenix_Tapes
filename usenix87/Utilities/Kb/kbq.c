/* kbq.c to query db-keys for keys -*-update-version-*-
** HFVR VERSION=Tue Jan  6 14:17:54 1987
*/

#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include "kb.h"

char	*blk = (char *)NULL;	/* ptr to block allocated */
int	blksize = 0;		/* size of blk block */
int	filsize;		/* size of file */

static int  count = 0;		/* number of files found matching */
char KNOWHOW[512];	/* holds copy of $KNOWHOW */
char *NAME;		/* pointer to program name */
char *CMD;		/* pointer to <command> */
BOOL dflag = FALSE;	/* TRUE if must print code */


/* findkey return TRUE if found else FALSE */
BOOL findkey(ptr)
CODE	*ptr;
{ 
	extern char *fsindex();

	if ( fsindex(blk,ptr->key,ptr->table) != NULL ) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}/*findkey*/

/* chk file for expression match */
SYM chkfile()
{ 
	extern BOOL interpret();

	if ( TRUE == interpret() ) {
		return(FOUNDsym);
	} else {
		return(NOTFOUNDsym);
	}
}/*readkey*/

/* get dir to work in via $KNOWHOW or $HOME/.knowhow */
void getdir()
{
	extern char *getenv();
	char *ptr;

	ptr = getenv("KNOWHOW");
	if ( ptr != NULL ) {
		strcpy(KNOWHOW,ptr);
	} else {
		ptr = getenv("HOME");
		strcpy(KNOWHOW,ptr);
		strcat(KNOWHOW,"/.knowhow");
	}
}/*getdir*/

/* command */
void command(name)
char	*name;
{
	char cmd[512];
	
	strcpy(cmd,CMD);
	strcat(cmd," ");
	strcat(cmd,name);
	system(cmd);
}/*command*/

/* do update on result of query */
void update(name)
char	*name;
{ 
	char *ptr;
	char EDITOR[512];
	char cmd[512];

	/* : ${EDITOR=/bin/ed} */
	ptr = getenv("EDITOR");
	if ( ptr != NULL ) {
		strcpy(EDITOR,ptr);
	} else {
		strcpy(EDITOR,"/bin/ed");
	}

	/* $EDITOR $KNOWHOW/name */
	printf("%s: Wait while I start %s ...",NAME,EDITOR); 
	fflush(stdout);
	strcpy(cmd,EDITOR);
	strcat(cmd," ");
	strcat(cmd,name);
	system(cmd);

	printf("%s: item %s updated\n",NAME,name);
}/*update*/

/* set global NAME to start of program name */
void getname(name)
char *name;
{ 
	extern char *strrchr();

	NAME = strrchr(name,'/');
	if ( NULL == NAME ) {
		NAME = name;
	} else {
		NAME++;
	}
}/*getname*/

void printkey(name)
char	*name;
{
	printf("%s\n",name);
}/*printkey*/

void printitem(name)
{
	char cmd[512];

	strcpy(cmd,"cat ");
	strcat(cmd,name);
	strcat(cmd," ; echo");
	system(cmd);
}/*printitem*/

/* strtolower convert string to uppercase */
void strtolower(s)
register char	*s;
{
	while ( *s ) {
		*s = tolower(*s);
		s++;
	}
}/*strtolower*/

/* determine whether we should print or update */
int workonid(name)
{
	struct stat stbuf;
	int f1;

	/* see if file exists, if so read it in buffer */
	if (stat(name,&stbuf) == -1) {
		return(10);	/* no such file */
	}

	/* see if we need more space */
	filsize = stbuf.st_size;
	if ( filsize + 1 > blksize ) {
		blksize = filsize + 1;
		if (blk != NULL) free(blk);
		blk = (char *)malloc(blksize);
		if (blk == NULL) {
			fprintf(stderr,"%s : ERROR : file %s too big\n",NAME,name);
			return(11);
		}
	}

	/* read in file */
	f1 = open(name,0);
	if (f1 == -1) {
		fprintf(stderr,"%s : ERROR : cannot read %s\n",NAME,name);
		return(12);
	}
	read(f1, blk, blksize);
	close(f1);
	blk[filsize] = '\0';
	strtolower(blk);

	if ( NOTFOUNDsym == chkfile() ) return(0);

	count++;
	switch(NAME[2]) {
	case 'u'     : 
		update(name);
		break;	/* kbu */
	case 'x'     :
		command(name);
		break;	/* kbx */
	case 'q'     : 
		printitem(name);
		break;	/* kbq */
	case 'c'     :
		break;   /* kbc */
	case 'k'     : 
		printkey(name);
		break;	/* kbk */
	default	     : 
		printitem(name);
		break;
	}
	return(0);
}/*workonid*/

main(argc,argv)
int	argc;
char	*argv[];
{ 
	register SYM result;
	extern void parse();
	int code;

	if ( argc < 2) {
		fprintf(stderr,"\007Usage: %s [-d] <query>\n",argv[0]);
		exit(1);
	}
	getname(argv[0]);
	if ( strcmp(argv[1],"-d") == 0 ) {
		dflag = TRUE;
		CMD = argv[3];
		strtolower(argv[2]);
		parse(argv[2]);
	} else {
		CMD = argv[2];
		strtolower(argv[1]);
		parse(argv[1]);
	}
	getdir();
	code = dir(KNOWHOW,workonid,1,1);
	switch(code) {
	case 0 : 
		break;
	case 1 : 
		fprintf(stderr,"%s : ERROR : %s directory does not exist\n",NAME,KNOWHOW);
		break;
	case 2 : 
		fprintf(stderr,"%s : ERROR : %s is not a directory\n",NAME,KNOWHOW);
		break;
	case 3 : 
		fprintf(stderr,"%s : ERROR : string '%s' too long to handle\n",NAME,KNOWHOW);
		break;
	case 4 : 
		fprintf(stderr,"%s : ERROR : cannot read %s\n",NAME,KNOWHOW);
		break;
	case 10: 
		break;
	case 11: 
		break;
	case 12: 
		break;
	default: 
		fprintf(stderr,"%s : ERROR : unknown cause\n");
		break;
	}
	fprintf(stderr,"%s: %d items found\n",NAME,count);
	return(code);
}/*main*/

