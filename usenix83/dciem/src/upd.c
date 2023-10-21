#
/*% cc -O upd.c
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * upd		update a set of files which are incremental versions
 *		of each other:
 * file00 file01 file02 ... file
 *
 * upd file		will copy file to filenn, where nn is the smallest
 *			unused version number.
 *
 * Author: Martin Tuori, DCIEM, Toronto, Ontario. Oct 1980.
 * based on an earlier version by Tom Duff.
 */
/* MIT01, Martin Tuori, Dept 1981. Modified to move existing file to filenn,
 * then copy filenn to file. Thus the date of the current version is preserved as filenn,
 * and file has just been 'touched'.
 */

struct stat inode;

#define MAXNAMELEN 14
#define MAXLEV 15

main(argc,argv)
	int argc;
	char **argv;
{
	register int version;
	char suffix[2];
	char workname[MAXLEV*MAXNAMELEN+1];
	char basename[MAXLEV*MAXNAMELEN+1];
	char command[5+(2*MAXLEV*MAXNAMELEN)];
	char *cp;

	argc--; argv++;		/* skip over program name */

	if(argc != 1){
		printf("usage: upd filename\n");
		exit(1);
	}
	/* check that the overall name will fit in the name buffers */
	if(strlen(*argv) >(MAXLEV*MAXNAMELEN-2)){
		printf("upd: filename %s is too long for internal buffers\n",*argv);
		exit(1);
	}
	/* check that the final node in the name is small enough */
	cp= &argv[0][strlen(*argv)];
	while(*cp!='/' && cp!=*argv) cp--;
	if(*cp=='/') cp++;	/* final node does not include the slash */
	if(strlen(cp) > MAXNAMELEN-2){
		printf("upd: filename %s is too long to append version numbers\n",*argv);
		exit(1);
	}
	strcpy(basename,*argv);
	if(stat(basename,&inode) <0){
		printf("upd: can't update %s, it doesn't exist.\n",basename);
		exit(1);
	}

	version= 0;
	do{
		strcpy(workname,basename);
		if(version<10) sprintf(suffix,"0%d",version);
		else sprintf(suffix,"%2d",version);
		strcat(workname,suffix);
		if(stat(workname,&inode) <0)
			break;
	}while(++version <= 99);
	if(version>99){
		printf("upd: there are already 99 versions of %s, fails\n",
			basename);
		exit(1);
	}
	/* MIT01 */
	strcat(command,"mv ");
	strcat(command,basename);
	strcat(command," ");
	strcat(command,workname);
	system(command);
	/* now check that the target was created */
	if(stat(workname,&inode) <0){
		printf("upd: failed to move file to: %s\n",workname);
		exit(1);
	}
	command[0]= '\0';	/* clear string */
	strcat(command,"cp ");
	strcat(command,workname);
	strcat(command," ");
	strcat(command,basename);
	system(command);
	/* now check that the target was created */
	if(stat(basename,&inode) <0){
		printf("upd: failed to make copy: %s\n",basename);
		exit(1);
	}
	/* end MIT01 */
	printf("upd: succeeds. There are now %d versions.\n",version+2);
	exit(0);
}
