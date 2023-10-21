#include <stdio.h>
/*	mid
*/
main (argc,argv)
int argc;
char **argv;
{
  char *format;
  int  count,position ;
  if (argc >= 5 || argc <= 3)
  {
    printf("Usage : %s string position width\n",argv[0]);
    exit(-2);
  }
  sscanf(argv[2],"%d",&position);
  sscanf(argv[3],"%d",&count);
  format =argv[1]+position;
  printf("%.*s\n",count,format);
}
