#include <stdio.h>
/*	left

	Returns leftmost substring of argv[1], length of substring in argv[2]
*/
char *strcpy();
char *strncpy();

main (argc,argv)
int argc;
char **argv;
{
  char res[];
  int  count , length;
  if (argc >= 4 || argc <= 2)
  {
    printf("Usage : %s string length\n",argv[0]);
    exit(-2);
  }
  sscanf(argv[2],"%d",&length);
  printf("%.*s\n",length,argv[1]);
}
