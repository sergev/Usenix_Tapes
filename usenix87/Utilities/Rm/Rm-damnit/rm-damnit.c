/*********************************************************
*							 *
* rm-damnit - remove stubborn files.			 *
*							 *
*    Usage: cd directory				 *
*           rm-damnit					 *
*							 *
* Permits the user to remove files with improper names.  *
* Interactively prompts for each file in a directory.    *
*							 *
* ORIGINAL April, 1985					 *
*   Bruce Karsh, Univ. Wisconsin, Madison, Dept. of      *
*   Geology and Geophysics.                              *
*   UUCP: uwvax\!geowhiz\!karsh                          *
*********************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/dir.h>
main()
{
int fd,status,i,lastchr;
struct direct direct;
char string[255],nm[DIRSIZ];
fd=open(".",O_RDONLY);
while(read(fd,&direct,sizeof(direct))==sizeof(direct))
  {
  if(direct.d_ino != 0)
    {
/*
** Search for the last non-zero char.
*/
    for(lastchr=DIRSIZ-1;lastchr>=0;lastchr--)
      if(direct.d_name[lastchr] != 0)break;
/*
** If all chars are zero, all chars should be changed to
** question marks.
*/
    if(lastchr<0)lastchr=DIRSIZ-1;
/*
** Copy name into temp buffer to be made ready to print.
*/
    for(i=0;i<DIRSIZ;i++)
      nm[i]=direct.d_name[i];
/*
** Convert bad chars to question marks.
*/
    for(i=0;i<lastchr+1;i++)
      {
      if(nm[i]<=040 || nm[i]>=0177)nm[i]='?';
      }
/*
** Ask if it's ok to delete.
*/
    printf("%.14s ?",nm);
    fgets(string,255,stdin);
/*
** Process user's response.
*/
    if(strcmp(string,"y\n") == 0)
      {
      printf("Removing %.14s\n",nm);
      status=unlink(direct.d_name);
      if(status != 0)printf("Remove failed\n");
      }
    if(strcmp(string,"q\n") == 0)exit(0);
    }
  }
printf("All done\n");
exit(0);
}
