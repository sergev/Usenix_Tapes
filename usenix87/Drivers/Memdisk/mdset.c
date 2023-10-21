#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>

char **argvec;

/* usage: mdset <unit> <size> */
/* size can have suffix k or m */
/* assumes /dev/[r]mdisk%d naming scheme */

int mdfd;
char mdname[64];

int geti(s)
register char *s;
{
 register int i;

 i = 0;
 for (;*s;s++)
  { switch (*s)
     { case '0': case '1': case '2': case '3': case '4':
       case '5': case '6': case '7': case '8': case '9':
	  i = (i * 10) + (*s - '0');
	  break;
       case 'm': case 'M':
	  i *= 1024;
	  /* fall through */
       case 'k': case 'K':
	  i *= 1024;
	  break;
       default:
	  return(-1);
     }
  }
 return(i);
}

main(ac,av)
int ac;
char **av;
{
 int unit;
 int size;

 argvec = av;
 if (ac != 3)
  { fprintf(stderr,"Usage: %s unit size\n",argvec[0]);
    exit(1);
  }
 unit = geti(av[1]);
 size = geti(av[2]);
 if (unit < 0)
  { fprintf(stderr,"%s: bad number `%s'\n",argvec[0],av[1]);
    exit(1);
  }
 if (size < 0)
  { fprintf(stderr,"%s: bad number `%s'\n",argvec[0],av[2]);
    exit(1);
  }
 sprintf(mdname,"/dev/rmdisk%d",unit);
 mdfd = open(mdname,O_RDWR,0);
 if (mdfd < 0)
  { fprintf(stderr,"%s: cannot open %s: ",argvec[0],mdname);
    perror((char *)0);
    exit(1);
  }
 if (ioctl(mdfd,LIOC_MEM_SET,&size) < 0)
  { fprintf(stderr,"%s: cannot set size for %s: ",argvec[0],mdname);
    perror((char *)0);
    exit(1);
  }
 exit(0);
}
