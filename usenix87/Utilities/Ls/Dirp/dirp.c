
/*
alias	ncd	'set d=`echo \!^* | sed "s/./\/&*/g"`; set d = `dirp $d`; cd $d || echo $d'
*/

/* dirp -- echo arguments that are directories/* 
include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

struct stat stats;

main(argc,argv)
     int argc;
     char **argv;
{
  char *file;
  argc--;
  argv++;
  while (argc > 0)
    {
      file = *argv;
      stat(file, &stats);
      if (((stats.st_mode) & S_IFMT) == S_IFDIR) printf ("%s\n", file);
      argc--;
      argv++;
    }
  exit(0);
}
