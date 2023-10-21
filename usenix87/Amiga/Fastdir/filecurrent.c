

/* filecurrent.c */

/* Author:  Rob Peck   5/13/86  */

/* part of "fastdir" */


#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "exec/memory.h"

#define ABS(x) (x > 0 ? x : -x)
#define SPRINTF 1
#define FPRINTF 0

extern struct FileLock *Lock(),*DupLock();
extern struct FileLock *CurrentDir(), *ParentDir();
extern struct FileHandle *stdout, *Open();

/* filecurrent -- See if there is a "current" copy of a specified file name
 *		  in the directory in which we now reside.  Return FALSE if
 *		  it is either nonexistent, or if it is non-current, based
 *		  on the minutes and seconds information passed to this
 *		  subroutine.  (When a file is opened into a directory
 *		  under AmigaDOS, both the file-written date and the 
 *		  directory-written date are modified.  The file date
 *		  itself gets written when it is closed.)  
 */

int 
filecurrent(name, error)

	char *name;	/* What is the name of the file to look for */
	int *error;	/* Where to put an error code, if and only if
			 * a value of FALSE is returned (TRUE is expected).
			 */
{
	LONG success, i; 
	LONG *parentdate;
	struct FileHandle *fh;
	char cbuffer[50];	    /* creation date buffer */
	char dbuffer[50];	    /* .dir file contents   */
	int actual, status;

	struct FileInfoBlock *fib;  /* for getting date information 	  */
	struct FileLock *filelock;  /* lock on the selected file          */
	struct FileLock *currentlock;  /* lock on the selected file       */
	struct FileHandle *dummyfh;

	dummyfh = NULL;		    /* not used anyhow */
	fib = NULL;
	currentlock = NULL;
	filelock = NULL;

	/* Try to lock this file, just to see if it exists */

	filelock = Lock(name, ACCESS_READ);

	if(!filelock)
	{
	    *error = ERROR_OBJECT_NOT_FOUND; 
	    return(FALSE);	/* If non-existent, it is not current! */
	}
	UnLock(filelock);
	filelock = NULL;

 	/* Once we get here, we know that the file exists, but we now
	 * have to determine whether it is current.  If it didn't exist,
	 * the above return(FALSE) would have happened.  Now read the 
	 * last modified date of the directory the file is in to get
	 * something against which to compare.  This particular date
	 * is stored as the first info within the file itself.
	 */

	/* FileInfoBlock MUST BE LONGWORD ALIGNED, so we have to 
	 * dynamically allocate it.
	 */

	fib = (struct FileInfoBlock *)
		AllocMem(sizeof(struct FileInfoBlock),MEMF_CLEAR);

	if(fib == 0)	/* no memory for file info block? */
	{
	    *error = ERROR_NO_FREE_STORE;
	    return(FALSE);	/* Cannot allocate memory */
	}
	
	/* initialize the buffers */

	for(i=0; i<50; i++)
	{
	    cbuffer[i] = 0x20;  /* ascii blank */
	    dbuffer[i] = 0x20;
	}
	cbuffer[49] = 0;  dbuffer[49] = 0;	/* end of string nulls */
	
	/* (Passing a null-string asks for a lock on the current directory) */

	/* Look at the directory we've moved into, and check its mod date   */
	/* Start by getting a lock on the current directory		    */

	currentlock = Lock("",ACCESS_READ);
	if(!currentlock)
	{
	    *error = ERROR_DIR_NOT_FOUND;  /* Can't lock current directory */
	    status = FALSE;
	    goto cleanfinish;
	}

	/* Now using that lock, Examine current directory to get the date */

	success = Examine(currentlock,fib);
	if(success)
	{
	    parentdate = (LONG *)&(fib->fib_Date);
	    DateToAscii(SPRINTF, cbuffer, parentdate, dummyfh);
	}
	status = FALSE;	   /* Start out assuming it is not the same */
	*error = 0;	   /* A false return with 0 errors is valid....
			    * it means file found, but not up to date   
			    */
	
	fh = Open(name,MODE_OLDFILE);
	if(!fh) goto cleanfinish;

	actual = Read(fh, dbuffer, 40);	/* get the date from the file */
	if(actual != 40)
	{
	    printf("Error while reading %ls\n",name);
	    goto cleanfinish;
	}
	status = comparedates(cbuffer, dbuffer);

    cleanfinish:

	if(fh) Close(fh);
	if(fib) FreeMem(fib, sizeof(struct FileInfoBlock));
	if(currentlock) UnLock(currentlock);
	return(status);
}

comparedates(s,t)
	char *s, *t;
{
	while( *s == *t )
	{
	    if(*s == '.') 	/* both equal and reach a period, ok! */
	    {
		return(TRUE);
	    }
	    if(*s == '\0') return(FALSE);	/* error, hit end of string
						 * without finding '.' 
						 */
	s++; t++;
	}
	return(FALSE);	/* found an unequal character prior to the period */
}


