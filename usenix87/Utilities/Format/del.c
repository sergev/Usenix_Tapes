#include <stdio.h>
/*	del

	Deletes the substring from argv[1] that starts at argv[2] and is
	argv[3] characters long.
*/
main (argc,argv)
int argc;
char **argv;
{
  char *format;
  int  count,position ;
  if (argc >= 5 || argc <= 3)
  {
    printf("Usage : %s string position number_to_delete\n",argv[0]);
    exit(-2);
  }
  sscanf(argv[2],"%d",&position);
  sscanf(argv[3],"%d",&count);
  if(position>strlen(argv[1]))position=strlen(argv[1]);
  if((position+count)>strlen(argv[1]))count=strlen(argv[1])-position;
  format =argv[1]+count+position;
  printf("%.*s%s\n",position,argv[1],format);
}
