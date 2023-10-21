/*----------------------------------------------------------------------*/
/*                           scandir()					*/
/*    returns a pointer to an array of pointers to structures of type   */
/*    "direct", based on a user supplied selection routine.		*/
/*    The structures are sorted according to the user supplied compare()*/
/*    routine.								*/
/*----------------------------------------------------------------------*/
/*  	uses calls to malloc() to allocate space for the structures     */
/*	History:							*/
/*	March 25, 1985	- written and tested ( out of desperation )     */
/*	Author:	J.L. Portman III 					*/
/*              of ... ihnp4!uw-beaver!tikal!teldata!transys!root	*/
/*	Environment: IBM-AT(tm) under XENIX(tm) system V from SCO	*/
/*									*/
/*		Copyright: Author claims exclusive ownership of the	*/
/* 		following. You are free to use the material in any 	*/
/*		form as long as this notice remains in place.		*/
/*		Hint: It may or may not have bugs!!			*/
/*----------------------------------------------------------------------*/
/* usage:								*/
/* scandir(dir,area,select,compare) where				*/
/* char *dir;		  directory to scan				*/
/* struct direct ***area; ptr to ptr to array of ptr's to struct direct */
/* int (select*)();	  ptr to function returning an int		*/
/* int (compare*)();	  ptr to function returning an int		*/
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ndir.h>			/* might be dir.h on Usg	  */
#include <malloc.h>
#ifndef FAIL
#define FAIL (-1)
#endif
#define DNULL ((DIR *)(0))
#define DPNULL ((struct direct *)(0))
#define DPPNULL ((struct direct **)(0))

int scandir(dir,filst,select,compare)
char *dir;				/* directory to scan 		   */
struct direct **filst[];  		/* where to put the sorted answers */
int (*select)();			/* pointer to selection routine    */
int (*compare)();			/* pointer to comparison for sort  */
{
	register DIR  *fp;
	register struct direct *dirp;
	register struct direct  **temp;		/* temporary storage ptr   */
	register unsigned int count = 0;	/* uunsigned math is faster*/
	void qsort();			/* sorts the directories pointers  */
	char *calloc(),*malloc();	/* works just fine here		   */
	char *strcpy();

	if (((fp = opendir(dir))==DNULL))
		return(FAIL);		/* could not open the dir	   */

	/* count the number of entries in the directory */
	
	for(count=0,dirp=readdir(fp);dirp != DPNULL;dirp=readdir(fp)) {
		if ((*select)(dirp))
			count++;
	}
	if (count==0)			/* skip this if no match */
		return(0);

	/* allocate a contiguous space of memory for pointers to the   */
	/* entries , allow for new files that might be created         */

        temp=((struct direct**) malloc((count+10) * sizeof(struct direct *))); 
	if (temp==DPPNULL)
		return(FAIL);
	
	/* rewind the directory */
	rewinddir(fp);

	/* put the pointers to all active elements in the newly allocated */
 	/* array */

	for(count=0,dirp=readdir(fp);dirp != DPNULL;dirp=readdir(fp))
		if ((*select)(dirp)){
		temp[count]=((struct direct *)malloc(sizeof(struct direct)));
			if (temp[count] == DPNULL) {
				return(FAIL);
  			}
		 temp[count]->d_ino=dirp->d_ino;		/* put it in */
		 temp[count]->d_reclen=dirp->d_reclen;	        /* put it in */
		 temp[count]->d_namlen=dirp->d_namlen;	        /* put it in */
	   (void)strcpy(temp[count]->d_name,dirp->d_name);      /* put it in */
		 count++;				        /* tell me   */
		}

	/* now sort the entries */

  	(void)qsort((char*)temp,count,sizeof(struct direct*),compare);

	*filst=temp;		/* don't ask me, it works!! */
	return((int)count);
}
