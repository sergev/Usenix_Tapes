/* mtime.c to print last modification time of file -*-update-version-*-
 * HFVR VERSION=Thu Mar 21 13:29:02 1985
 */

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

char FORMAT[512] = "+%a %h %d %H:%M:%S 19%y";

/* prformat: to print string in FORMAT */
prformat(time)
struct tm *time;
{
 int i;
 char format[512];

 strcpy(format,FORMAT);
 for ( i=1 ; format[i] != '\0' ; i++) {
  if ( format[i] != '%') {
   printf("%c",format[i]);
  } else {
   switch (format[i+1]) {
    case '\0':
         break;
    case 'n' :
         printf("\n");
	 i++;
	 break;
    case 't' :
         printf("\t");
	 i++;
	 break;
    case 'm' :
         printf("%.2d",time->tm_mon+1);
	 i++;
	 break;
    case 'd' :
         printf("%.2d",time->tm_mday);
	 i++;
	 break;
    case 'y' :
         printf("%.2d",time->tm_year);
	 i++;
	 break;
    case 'D' :
         printf("%.2d/%.2d/%.2d",time->tm_mon+1,time->tm_mday,time->tm_year);
	 i++;
	 break;
    case 'H' :
         printf("%.2d",time->tm_hour);
	 i++;
	 break;
    case 'M' :
         printf("%.2d",time->tm_min);
	 i++;
	 break;
    case 'S' :
         printf("%.2d",time->tm_sec);
	 i++;
	 break;
    case 'T' :
         printf("%.2d:%.2d:%.2d",time->tm_hour,time->tm_min,time->tm_sec);
	 i++;
	 break;
    case 'j' :
         printf("%.3d",time->tm_yday+1);
         i++;
	 break;
    case 'w' :
         printf("%.2d",time->tm_wday);
	 i++;
	 break;
    case 'a' :
         switch (time->tm_wday) {
          case 0 : printf("Sun"); break;
	  case 1 : printf("Mon"); break;
	  case 2 : printf("Tue"); break;
	  case 3 : printf("Wed"); break;
	  case 4 : printf("Thu"); break;
	  case 5 : printf("Fri"); break;
	  case 6 : printf("Sat"); break;
	  default: printf("???"); break;
	 }/*switch*/
         i++;
	 break;
    case 'h' :
         switch (time->tm_mon) {
          case 0: printf("Jan"); break;
	  case 1: printf("Feb"); break;
	  case 2: printf("Mar"); break;
	  case 3: printf("Apr"); break;
	  case 4: printf("May"); break;
	  case 5: printf("Jun"); break;
	  case 6: printf("Jul"); break;
	  case 7: printf("Aug"); break;
	  case 8: printf("Sep"); break;
	  case 9: printf("Oct"); break;
	  case 10:printf("Nov"); break;
	  case 11:printf("Dec"); break;
	  default:printf("???"); break;
	 }/*switch*/
         i++;
	 break;
    default:
         printf("%c",format[i+1]);
         i++;
	 break;
   }/*switch*/
  }/*fi*/
 }/*for*/
 printf("\n");
} /*prformat*/

/* workon: work on a file mentioned in path */
workon(argv,path)
char *argv[];
char *path;
{
 struct stat filbuf; /* file descriptor */
 time_t mtime;
 extern struct tm *localtime();
 extern char *asctime();
 struct tm *local;
 
/* get info on file */
 if ( stat(path,&filbuf) == -1 ) {
 fprintf(stderr,"\007%s: error %d. Cannot work on: %s,\n",argv[0],errno,path);
 perror("because");
  return(errno);
 }

/* print m_time */
 mtime = filbuf.st_mtime;
 local = localtime(&mtime);
 prformat(local);
} /*workon*/


main(argc,argv)
int argc;
char *argv[];
{
int i;

/* work on all file arguments */
 for (i = 1 ; i < argc ; i++) {
  if ( argv[i][0] == '-' ) {
   fprintf(stderr,"Usage: %s { [+output-format] filename }*\n",argv[0]);
   fprintf(stderr,"To display last modification date of file(s)\n");
   exit(1);
  }
  if ( argv[i][0] == '+') {
   strcpy(FORMAT,argv[i]);
  } else {
   workon(argv,argv[i]); 
  }
 }
} /*main*/
