/*
      Copyright (C) Duncan Carr Agnew 1980
*/
#define PM 0644		/* r&w for owner, r for everyone else */
#define NAMLEN 83	/* length of filename on disk (including pathname) */
#define MAXOPN 16	/* max number of files allowed open */
#include <stdio.h>
static char jfile[MAXOPN][NAMLEN];
static int maxno = 0, nopen = 0;
static int fd[MAXOPN];
extern long lbt[];	/* last byte handled (zeroed when file returned) */

getfd(name,iact,iloc)
/* For a filename name, returns the fd attached to it. Names of files
   previously accessed are contained in the jfile table. If a file has
   not been previously accessed (name not in the jfiletable), it is
   opened (if iact<=0) or created (if iact>0) and placed in the table.
   The location of the file in the jfile table is returned by iloc,
   which is thus available for indexing purposes in the calling program.
   An error message is printed and the program aborted if:
        1. A file to be opened cannot be.
        2. A file to be created cannot be.
        3. More than MAXOPN files are to be accessed at once (this is
           the number of spaces in the jfile table; files may be removed
           from the table using subroutine return).

   Original version written at CIRES by D. Agnew March 1980
   Revised by D. Agnew November 1980 */
char name[];
int iact,*iloc;
{
int i,ity;
char filnam[NAMLEN];
for (i=0; i<maxno; i++){
   if(strcmp(name,&jfile[i][0])==0){
      *iloc = i;
      return(fd[i]);
   }
}
/* file is not in jfile table. Check if there is room for another */
/* (note that nopen is the number of files currently open, while */
/* maxno points just after the last filled place in the */
/* jfile table), and if so, open or create it, and put it in the first */
/* empty place */
if (nopen==MAXOPN){
   fprintf(stderr,"No more than %d files may be open at once.\n",MAXOPN);
   fprintf(stderr,"File %s is over this limit: program aborts.\n",name);
   exit(15);
}
strcpy(filnam,name);
strcat(filnam,".D");
if(iact<=0){
   ity=open(filnam,2);		/*open for read and write, since we may want both*/
   if(ity==-1){		/*error off on failed open*/
      fprintf(stderr,"File named %s could not be opened. It may not exist\n",name);
      fprintf(stderr,"or you may lack write permission for it. Program aborts\n");
      exit(15);
   }
   lseek(ity,0L,0);		/*rewind to start of file*/
}
else
   ity=creat(filnam,PM);
   if(ity==-1){
      fprintf(stderr,"Could not create file %s. Program aborts.\n",name);
      exit(15);
   }
   close(ity);			/*have to return and then open file to*/
   ity=open(filnam,2);		/*be able to read it in same program call*/
   lseek(ity,0L,0);
nopen++;
for(i=0; i<maxno; i++){
   if(strcmp(&jfile[i][0],"0empty0")==0)
      break;
}
/* if we fall off the end of this loop, put new filename at the end, */
/* otherwise it goes within the table, in a space made blank using  */
/* return_ */
strcpy(&jfile[i][0],name);
fd[i] = ity;
if(i==maxno)
   maxno++;
*iloc = i;
return(ity);
}

return_(name,dum)
/* Fortran-callable routine for closing the file name, and removing it */
/* from the jfile table */
char name[];
long dum;
{
int i;
i=namtyp(name);		/*call to put null after filename*/
for(i=0; i<maxno; i++){
   if(strcmp(name,&jfile[i][0])==0){
      strcpy(&jfile[i][0],"0empty0");
      close(fd[i]);
      lbt[i]=0L;
      fd[i]= -1;
      nopen--;
      if(i==maxno-1)
         maxno--;
   }
}
return;
}
