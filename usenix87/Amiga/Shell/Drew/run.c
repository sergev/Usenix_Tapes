
/*
 * RUN.C
 *
 * (c)1986 Matthew Dillon     9 October 1986
 *
 *    RUN   handles running of external commands.
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 */

#include "shell.h"

char *FindIt();

do_run(str)
char *str;
{
   int i, try = -1;
   int run = 0;
   char buf[128];
   char runcmd[128];
   char *save, *path, *index();

   char *p = av[0];
   char **args = av+1;
   
   while(*p++) *p &= 0x7F;	/* allow "com mand" */

   while(*args) {                  /* if any arg contains a space then */
      if (index(*args,' ')) {      /* surround with quotes, since must */
         i = strlen(*args);        /* of specified via "arg u ment" on */
	 movmem(*args,(*args)+1,i);/* original command line.           */
         args[0][0] = args[0][i+1] = '\"';  /* mpush in execom.c has   */
         args[0][i+2] = '\0';      /* allowed for these 2 extra bytes. */
      }
      ++args;
   }
   
   if (path = FindIt(av[0],"",buf)) {
      if (!strcmp(av[0],"run") || !strcmp(av[0],"ru")) {      /* was a run */
	 if (FindIt(av[1],"",runcmd)) {
	    run = 1;
	    save = av[1];
	    av[1] = runcmd;
	 }
      }
      if ((try = fexecv(path, av)) == 0)
	 i = wait();
      if (run) av[1] = save;
   }
   else {
      APTR original;
      original = Myprocess->pr_WindowPtr;
      Myprocess->pr_WindowPtr = (APTR)(-1);
      if ((try = fexecv(av[0], av)) == 0)
	 i = wait();
      Myprocess->pr_WindowPtr = original;
   }
   if (try) {
      long lock;
      char *copy;

      if ((path = FindIt(av[0],".sh",buf)) == NULL) {
	 fprintf(stderr,"Command Not Found %s\n",av[0]);
	 return (-1);
      }
      av[1] = buf;		 /* particular to do_source() */
      copy = malloc(strlen(str)+3);
      strcpy(copy+2,str);
      copy[0] = 'x';
      copy[1] = ' ';
      i = do_source(copy);
      free(copy);
   }
   return (i);
}


char *
FindIt(cmd, ext, buf)
char *cmd;
char *ext;
char *buf;
{
   long lock = 0;
   char hasprefix = 0;
   APTR original;
   char *ptr, *s = NULL;

   original = Myprocess->pr_WindowPtr;

   for (ptr = cmd; *ptr; ++ptr) {
      if (*ptr == '/' || *ptr == ':')
	 hasprefix = 1;
   }

   if (!hasprefix) {
	Myprocess->pr_WindowPtr = (APTR)(-1);
	s = get_var(LEVEL_SET, V_PATH);
   }

   strcpy(buf, cmd);
   strcat(buf, ext);
   while ((lock = (long)Lock(buf, ACCESS_READ)) == 0) {
      if (*s == NULL || hasprefix) break;
      for(ptr = s; *s && *s != ','; s++) ;
      strcpy(buf, ptr);
      buf[s-ptr] = '\0';
      strcat(buf, cmd);
      strcat(buf, ext);
      if (*s) s++;
   }
   Myprocess->pr_WindowPtr = original;
   if (lock) {
      UnLock(lock);
      return(buf);
   }
   return(NULL);
}
