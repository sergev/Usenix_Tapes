#include <stdio.h>
/*	len

	Returns length of string in argv[1]
*/
main (argc,argv)
int argc;
char **argv;
{
  char res[];
  if (argc >= 3 || argc <= 1)
  {
    printf("Usage : %s string\n",argv[0]);
    exit(-2);
  }
  printf("%d\n",strlen(argv[1]));
}
