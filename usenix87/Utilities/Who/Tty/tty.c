/* tty improved version will take login name as argument -*-update-version-*-
** HFVR VERSION=Thu Dec 12 14:34:46 1985
*/

#include <sys/types.h>
#include <utmp.h>
#include <stdio.h>
#include <string.h>

main(argc,argv)
int	argc;
char	*argv[];
{ struct utmp *record;
  extern struct utmp *getutent();
  extern char *getlogin();
  int user;
  char *name;

/* checkoptions */
  if ( argc >= 2 ) {
   if ( argv[1][0] == '-' ) {
    switch (argv[1][1]) {
     case 'V': fprintf(stderr,"%s: version 0.99\n",argv[0]);
               exit(1);
	       break;
     default : fprintf(stderr,"Usage: %s [login]\n",argv[0]);
               exit(1);
               break;
    }/*switch*/
   }/*fi*/
  }/*fi*/

  if ( argc == 2 ) {
   name = argv[1];
  } else {
   name = getlogin();
  }
  while ( (record = getutent()) != NULL ) {
    if ( strncmp(record->ut_user,name,8) == 0 ) {
     printf("/dev/%s\n",record->ut_line);
     exit(0);
    }/*fi*/
  }/*while*/
  exit(1);
}/*main*/
