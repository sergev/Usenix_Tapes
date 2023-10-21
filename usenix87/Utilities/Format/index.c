#include <stdio.h>
/*	index

	Returns the character position of the first occurrence of argv[2]
	within argv[1]
*/
index(s, t) 		/* return index of t in s, -1 if none */
char s[], t[];
{
  int i,j,k;

  for (i=0;s[i] != '\0';i++)
  {
    for (j=i, k=0; t[k] != '\0' && s[j]==t[k]; j++,k++)
      ;
    if (t[k] == '\0') return (i);
  }
  return (-1);
}


main(argc,argv)
int argc;
char **argv;
{ int ind;
  if (argc>=4 || argc<=2)
  {
    printf("Usage : %s string pattern\n",argv[0]);
    exit(-2);
  }
  ind = index(argv[1],argv[2]);
  if (ind!=-1) printf("%d\n",ind);
  else 
  {
    printf("%s\n","");
    exit(-1);
  }
}
