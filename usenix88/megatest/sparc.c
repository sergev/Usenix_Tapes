#include <stdio.h>

main(argc,argv)
 char** argv;
{
  argc--; argv++;
  while(argc--)
    arch(*argv++);
}

arch(file)
  char* file;
{
  printf(".%s\n", file);
  { FILE* fp = fopen(file, "r");
    int ch;
    ch = fgetc(fp);
    while(ch != EOF)
      {
	putchar('>');
	while(ch != '\n')
	  { putchar(ch);
	    ch = fgetc(fp);
	  }
	putchar('\n');
	ch = fgetc(fp);
      }

    fclose(fp);
  }
}
