#include <stdio.h>
char *strcat();
/*	con
 
 	concatenates argv[1] and argv[2] and prints the result
*/
main (argc,argv)
int argc;
char **argv;
{
  if (argc >= 4 || argc <= 2)
  {
    printf("Usage : %s string1 string2\n",argv[0]);
    exit(-2);
  }
  strcat(argv[1],argv[2]);
  printf("%s\n",argv[1]);
}
