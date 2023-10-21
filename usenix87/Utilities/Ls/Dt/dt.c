#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

#define INDENT_STR   3
#define MAXFILES     20
extern int errno;

char *pn;      /* program name */

int vflag = 0;       /* verbose flag */
int sflag = 0;       /* include symbolic links? */
int tflag = 1;       /* use tabs instead of spaces???? */
int in_size = 0;     /* indent size */

main(argc,argv)
int argc;
char **argv;
{
   char *dir;
   struct stat sb;
   register char c;
   int errflag = 0;
   extern char *optarg;
   extern int optind;

   pn = argv[0];


   while((c = getopt(argc,argv,"vi:s")) != EOF)
      switch(c) {
      case 'v':
         vflag = 1;
         break;
      case 's':
         sflag = 1;
         break;
      case 'i':
         if(!(in_size = atoi(optarg))) {
            fprintf(stderr,"%s: invalid indent size \"%s\"\n",pn,optarg);
            ++errflag;
         }
         tflag = 0;
         break;
      case '?':
      default:
         ++errflag;
      }

   if(errflag || (tflag && in_size)) {
      fprintf(stderr,"Usage:  %s [-vs] [-i n] <dir> [<dir>...]\n",pn);
      exit(99);
   }

   if(optind == argc) {
      dir = ".";
      descend(dir,0);
      exit(0);
   }

   
   for(;optind<argc;++optind)
      descend(argv[optind],0);
   
   exit(0);
}


descend(path,level)
char *path;
int level;
{
   register DIR *dirp;
   register struct direct *dent;
   long offset = 0L;
   struct stat sb;
   char ts[80];

   if(chdir(path) < 0) {
      if(vflag) {
         sprintf(ts,"<%s>!cd\n",path);
         pout(ts,level);
      }
      return;
   }
   pout(path,level);

   if((dirp = opendir(".")) == NULL) {
      fprintf(stderr,"%s: can't open \".\"\n",pn);
      exit(2);
   }

   errno = 0;
   while(dent=readdir(dirp)) {
      if(!strcmp(dent->d_name,".") || !strcmp(dent->d_name,".."))
         continue;
      if(stat(dent->d_name,&sb) < 0) {
         if(vflag) {
            sprintf(ts,"<%s>!stat",dent->d_name);
            pout(ts,level);
         }
         continue;
      }
      if((sb.st_mode&S_IFMT) != S_IFDIR)
         continue;

      /* check for symbolic link */
      if(lstat(dent->d_name,&sb) < 0) {
         if(vflag) {
            sprintf(ts,"<%s>!lstat",dent->d_name);
            pout(ts,level);
         }
         continue;
      }
      if(((sb.st_mode&S_IFMT) == S_IFLNK) && (!sflag)) {
         sprintf(ts,"%s(s)",dent->d_name);
         pout(ts,level+1);
         continue;
      }
      if(dirp->dd_fd >= MAXFILES-1) {
         offset = telldir(dirp);
         closedir(dirp);
         dirp = NULL;
      }
      descend(dent->d_name,level+1);
      if(!dirp) {
         if((dirp = opendir(".")) == NULL) {
            fprintf(stderr,"%s: can't reopen \".\"\n",pn);
            exit(9);
         }
         seekdir(offset);
      }
   }

   closedir(dirp);
   if(chdir("..") < 0) {
      fprintf(stderr,"%s: can't change to \"..\"\n",pn);
      exit(99);
   }
   return;
}



pout(name,indent)
char *name;
int indent;
{
   int i;

   if(tflag) {
      for(i=0;i<indent;++i)
         putchar('\t');
      puts(name);
   } 
   else {
      if(indent*in_size)
         printf("%*s%s\n",in_size*indent," ",name);
      else
         puts(name);
   }
   return;
}
