/*****************************************************************************
*	FLST.C							   Version 1.0
*
*	Print out the directory tree.
*
*	flst			print out all files
*	flst -d			print out only directories
******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/file.h>

extern int alphasort();			/* Routine to sort dir. names */
extern char *malloc();			/* Allocate memory */

char VERSION[] = "Version 1.0 by Bruce Holloway (holloway@drivax.UUCP)";

int level = 0;
int dironly = 0;

main(acnt,avar)
int acnt;
char **avar;
{
    char *s;

    if(acnt == 1) lets_do(".");
    else while(--acnt){
	++avar;
	if(**avar == '-'){
	    s = *avar + 1;
	    while(*s) switch(*s++){
		case 'd': ++dironly; break;
		}
	    }
	else lets_do(*avar);
	}
    return(0);
}

/* Ignore any files that begin with a dot. */

int selfile(d)
struct direct *d;
{
    return(d->d_name[0] != '.');
}

/* Each subdirectory contains some number of other subdirectories, and
   some number of regular files, whose names are numbers. Any files that
   are directories append their names to fname and get called recursively.
*/

lets_do(fname)
char *fname;
{
    char xname[128];
    char qname[64];
    struct direct **namelist, *d;
    int nument, i, j;
    int flen, isdir;
    struct stat stb;

    flen = strlen(fname);
    (void) strcpy(xname,fname);

    if((nument = scandir(xname, &namelist, selfile, alphasort)) == -1){
	fprintf(stderr,"%s: Not a directory, or not enough memory.\n",xname);
	return;
	}

    for(i = 0; i < nument; ++i){
	d = namelist[i];
	(void) strcpy(qname,d->d_name);

	(void) strcat(xname,"/");
	(void) strcat(xname,qname);

	if(stat(xname,&stb) == -1){
	    fprintf(stderr,"Error finding stats for \"%s\".\n",xname);
	    continue;
	    }

	isdir = ((stb.st_mode & S_IFMT) == S_IFDIR);

	if(!dironly || (dironly && isdir)){
	    for(j=0; j++ < level;) printf("    ");
	    printf("%s%s\n",qname,(isdir)?"/":"");
	    }

	if(isdir){
	    ++level;
	    lets_do(xname);
	    --level;
	    }

	xname[flen] = 0;
	}

    for(i = 0; i < nument; ++i) free((char *)(namelist[i]));
    free((char *)namelist);
}
