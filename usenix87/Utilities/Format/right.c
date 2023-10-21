#include <stdio.h>
char *strcpy();
/*	right

	Returns the right substring of argv[1], length in argv[2]
*/
main (argc,argv)
int argc;
char **argv;
{
  char *format;
  int  count , length , index;
  if (argc >= 4 || argc <= 2)
  {
    printf("Usage : %s string length\n",argv[0]);
    exit(-2);
  }
  sscanf(argv[2],"%d",&length);
  format = argv[1];
  format += strlen(argv[1])-length;
  printf("%s\n",format);
}
