#include <stdio.h>
/*	ins

	Inserts argv[2] into argv[1] at position argv[3] and returns result
*/
main (argc,argv)
int argc;
char **argv;
{
  char *format;
  int  count ;
  if (argc >= 5 || argc <= 3)
  {
    printf("Usage : %s main_string insert_string position\n",argv[0]);
    exit(-2);
  }
  sscanf(argv[3],"%d",&count);
  if(count>strlen(argv[1]))count=strlen(argv[1]);
  format = argv[1] + count;
  strcat(argv[2],format);
  printf("%.*s%s\n",count,argv[1],argv[2]);
}
