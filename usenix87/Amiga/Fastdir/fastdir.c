
/* fastdir.c */

/*
 * Author:  Rob Peck 5/13/86
 */

#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "exec/memory.h"

#define ABS(x) (x > 0 ? x : -x)
#define MAX(x,y) (x > y ? x : y)

extern struct FileHandle *stdout;

extern struct FileLock *Lock(),*DupLock();
extern struct FileLock *CurrentDir(), *ParentDir();
extern struct FileHandle *stdout, *Open();
char *blanks = "                                   ";	/* 35 blank spaces */
char linebuffer[80];			/* workspace for output formatting */
int leftright=0;			/* start at left column  */

struct tnode	/* for quick alphabetizing */
{
	struct tnode *left;
	struct tnode *right;
	char name[36];	/* space to store the name (30 + ' (dir)') max */
	char dirflag;	/* nonzero if it is a directory */
};

extern struct tnode *tree();
struct tnode *root;	   /* Pointer to the first one so we can deallocate */
struct FileInfoBlock *m;   /* for getting write-protect information */
struct FileLock *newlock;  /* Lock on current (or requested) dir    */
struct FileLock *dirlock;  /* Lock on the .dir file 	 	     */
int    maxlen=0;	   /* Maximum length of any string, provided 
			    * for later change in formatting (more
			    * entries per line
		            */

#define PROTECTED 1
#define WRITEABLE 0    

main(argc, argv)
	int argc;
	char *argv[];
{
	LONG myerror;		    /* place for error return filecurrent */
	LONG success;	    	    /* work variables			  */
	char *whichdir;		    /* which directory to get a lock on?  */
	struct FileLock *startlock; /* lock on the current directory      */
	struct FileLock *ignorlock; /* going back to original dir.        */
	struct InfoData *id;	    /* see if disk is write-protected     */
	struct FileLock *originallock; /* for "going home" again */

	/* Save a lock to original directory so that before we exit, we
	 * can move the CLI back to where it started
	 */
	originallock = Lock("",ACCESS_READ);	

	id = NULL;

	if(argc == 1) 
	{
	    /* No directory specified, use current one! */
	    /* Null string means use current dir */
	    whichdir = "";	
	}
	else
	{
	    whichdir = argv[1];
	} 
	/* Get a lock on the user-specified directory */

   	newlock = Lock(whichdir,ACCESS_READ);

	if(newlock == 0)
	{
	  if(argc == 1)
	    printf("Can't get a lock on current dir!\n");
	  else
	    printf("Can't find %ls\n",argv[1]);
	  exit(100);
	}
	else
	{
	    /* Move into this user selected directory */
	
	    startlock = CurrentDir(newlock);
	}
	/* Now see if there is a current copy of the ".dir" file here */

	if(filecurrent(".dir",&myerror))
	{
	    /* If its there and current, then output it */

	    output_dirfile(newlock);
	}
	else
	{ 
	    /* If its not current, attempt to create or overwrite it */

	    /* InfoData MUST BE LONGWORD ALIGNED, so allocate it     */

	    id = (struct InfoData *)
		AllocMem(sizeof(struct InfoData),MEMF_CLEAR);

	    if(id)
	    {
		/* Get info to see if disk is write protected */

	        success = Info(newlock, id);  
	        if(success)
	        {
		    if(id->id_DiskState == ID_WRITE_PROTECTED)
		    {
		        DirList(newlock, PROTECTED, whichdir);
			goto finish;
		    }
	        } 
		else
		{
		    printf("Can't get Info about current directory\n");
		    goto finish;	/* Info command failed */
		}
	        if(DirList(newlock, WRITEABLE, whichdir))
	        {
		    /* If DirList returns nonzero, means the file
		     * got created (there was at least one entry
		     * in the directory and the disk wasn't write
		     * protected.)
		     */ 
	             output_dirfile(newlock);  /* copy ".dir" to stdout */
	        }
	        goto finish;
	    }
	    else
	    {
	        printf("Not enough memory for InfoData\n");
		/* If this happens, there certainly isn't enough
		 * to generate a DirList either! 
		 */
	    }
    finish:		
	}
	if(id) FreeMem(id, sizeof(struct InfoData));
    cleanup:
	if(newlock) UnLock(newlock);
	/* move into the original directory from whence you came */
	ignorlock = CurrentDir(originallock);
/* 	if(originallock) UnLock(originallock);   */
}


/* output_dirfile -- copy ".dir" to stdout  	        	*/

output_dirfile(lock)
struct FileLock *lock;	/* shows the current directory to use */
{
    struct FileHandle *fh;
    struct FileLock *ignoredlock;
    UBYTE buffer[300];
    int actualcount, outcount;

    ignoredlock = CurrentDir(lock);	/* make sure are in correct place */
    fh = Open(".dir",MODE_OLDFILE);   
    if(fh)
    {
      while(1)				     /* forever, till EOF */
      {

        actualcount = Read(fh, buffer, 255); /* decent block size */
	if(actualcount == 255)
	{
	    /* ======================++++++++======================= */
	    /* NOTE: This assumes an AmigaDOS file handle for stdout */
	    /* ======================++++++++======================= */
	    outcount = Write(stdout, buffer, actualcount); 
	}
	if(actualcount < 255)
	{
	    outcount = Write(stdout, buffer, actualcount); 
	    break;
	}
	if(actualcount == -1)
	{
	    printf("Error ( %ld ) copying .dir to output\n",IoErr());
	    break;
	}
      }
    Close(fh);	/* close the file */
    }
    else
    {
	printf("'.dir' won't open!  IoErr() = %ld\n", IoErr());
    }
} 

/* DirList -- Look through the binary tree listing that tree() produced
 * 	      and output the names in that tree in alphabetical order.
 *	      If selected directory is writeable, create a file named
 * 	      .dir there, containing the formatted output.  Current
 * 	      implementation puts two names on each line, just like DIR.
 */


DirList(lock, protectstatus, pathname)   
struct FileLock *lock;
int protectstatus;
char *pathname;			/* Pathname that user gave us. */
{
    struct FileHandle *tfh;
    LONG success;
    struct tnode *allocated;
    struct tnode *t;
    LONG *ds;

    /* if at the end of the road, don't print anything */
    if(!lock) return(0);   

    root = (struct tnode *)
		AllocMem(sizeof(struct tnode),MEMF_CLEAR);

    if(!root) return(0);

    /* allocate space for a FileInfoBlock */

    m = (struct FileInfoBlock *)
    	AllocMem(sizeof(struct FileInfoBlock),MEMF_CLEAR);

    if(!m) goto noentries;

    success = Examine(lock,m);

    if(!success) 
    {
	printf("Can't examine directory for '%ls'\n",pathname);
	goto noentries;
    }	
  	 
   /* The first call to examine fills the FileInfoBlock with 
    * INFORMATION ABOUT THE DIRECTORY.  If it is called at the 
    * root level, it contains the volume name of the disk.  If 
    * we're building a directory listing, we won't want this 
    * entry to be in it, so we call ExNext before starting the
    * listing itself.
    */

    /* Three conditions to consider here... 
     * a.  is NOT a directory.
     * b.  is an empty directory.
     * c.  has at least one entry.
     */

    /* NOT A DIRECTORY */

    if(m->fib_DirEntryType <= 0)
    {
	/* If this is true, we're dealing with a plain file.
	 * It should exit saying this is NOT a directory 
	 */
	printf("'%ls' is not a directory.\n",pathname);

  noentries:

	if(root) FreeMem(root, sizeof(struct tnode));
	/* treeprint normally frees all entries, including root, but
	 * we never get there so we have to free the memory here instead
	 */
	if(m) FreeMem(m, sizeof(struct FileInfoBlock));
	return(0);	/* didn't create a file, so don't print it. */
    }	
    success = ExNext(lock,m);

    /* EMPTY DIRECTORY */

    if(!(success)) 
    {
	printf("Directory '%ls' is empty.\n", pathname);
	goto noentries;
    }

    /* AT LEAST ONE ENTRY */
   
    /* Initialize the very first node */
	
    t = root;		
    strcpy(  &(t->name[0]), &(m->fib_FileName[0]) );
    if (m->fib_DirEntryType > 0)
    {
	strcat( &(t->name[0]), " (dir)" );
    }
    maxlen = MAX(maxlen, strlen( &(t->name[0])));

    /* t->left and t->right pointers are already set to zero by AllocMem */

    success = ExNext(lock,m);
    while (success)		/* was (success != 0)  */
    {
	t = root;	/* begin the search at the root */

 	/* Install it into the chain, recursively.  Allocated is an
	 * indicator that there was still enough memory to build a
	 * new chain for this node.  If allocated == 0, have to
	 * terminate prematurely.
	 */
	allocated = tree(t, &(m->fib_FileName[0]));
	if(allocated == 0) break;	
      	success = ExNext(lock,m);
    }
    if(allocated == 0)	/* header for the premature termination */
    {
	printf("\nPARTIAL DIR LISTING ONLY....NOT ENOUGH MEMORY\n");
	goto showlist;
    }
     
    /* "KLUGE".... we are trying to create a file named '.dir', but we
     * don't have any (FileInfoBlock) info about it yet since the builder 
     * of the file hasn't yet even written it.  So we'll take the very 
     * last entry in the file info block and modify its name field to 
     * be '.dir' so that this file name will actually appear in the listing.
     * (this is ok since tree() is not using anything but the name field).
     */

    if(protectstatus == WRITEABLE)
    {
        strcpy( &(m->fib_FileName[0]), ".dir" );
        allocated = tree(root, ".dir");		/* install .dir */
    }
showlist:

    /* Now begin to build the output listing, two to a line */

    sprintf(&linebuffer[0], "%ls", blanks);	/* blank first 35 locations */
    sprintf(&linebuffer[35], "%ls", blanks);	/* blank next  35 locations */
    linebuffer[67] = '\0';	/* null terminator */

    tfh = 0;				/* start out with no file open    */
    if(protectstatus == WRITEABLE)	/* If seems to be unprotected ... */
    {
	tfh = Open(".dir",MODE_NEWFILE);  /* if fails, tfh = 0 */
    }
    /* When this file is FIRST opened, the time of its parent directory
     * modification is established.  If we want to know if the file we
     * have created is current, we have to store the change time of the
     * parent directory in this file itself.  Then no matter, with
     * multi-tasking, how long it takes to create this .dir file
     * (whose date is established when it CLOSES), we'll have the
     * correct numbers to compare, and be zero time-ticks off.
     * The .dir file will contain its parent's creation date and time as the
     * first line, so you'll be able to see whether DIR gets generated
     * fresh or if it came from the .dir file.
     */
    success = Examine(newlock,m);	/* Succeeded once already so it
					 * should also succeed here.
					 * Now the date has been changed
					 * to reflect the .dir file opened.
					 */
    ds = (LONG *)&(m->fib_Date); 	/* Pass along the address of the 
					 * datestamp of the parent directory
					 */
    dotreeprint(root, tfh, ds);

    if(tfh) Close(tfh);
    if(m) FreeMem(m,sizeof(struct FileInfoBlock));
    return(TRUE);	/* Created a file, so its ok to print it later */
}

strcat( to, from )
register char *to, *from;
{
    while( *to ) to++;	/* find the end of the current string */

    strcpy( to, from ); /* then overlay trailing null, with first char.
			 * of the string to be concatenated. */
}

strlen( s )
register char *s;
{
    register i = 0;

    while( *s++ ) i++;

    return( i );
}

