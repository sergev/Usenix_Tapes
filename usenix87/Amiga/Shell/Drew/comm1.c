/*
 * COMM1.C
 *
 * Matthew Dillon, August 1986
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 */

#include "shell.h"
typedef struct FileInfoBlock FIB;

#define DIR_SHORT 0x01
#define DIR_FILES 0x02
#define DIR_DIRS  0x04

extern int has_wild;
char cwd[256];

/*
    Parse the options specified in sw[]
    Setting a bit for each one found
*/
get_opt(sw,count)
char *sw;
int *count;
{
   int l,i = 0, opt = 0;
   char *c,*s;

   while((++i < ac) && (av[i][0] == '-')) {
	for (c = av[i]+1; *c ; c++) {
	    for(l = 0,s = sw;*s && *s != *c; ++s) ++l;
	    if (*s) opt |= (1 << l);
	}
   }
   *count = i;
   return(opt);
}

do_sleep()
{
   register int i;

   if (ac == 2) {
      i = atoi(av[1]);
      while (i > 0) {
	 Delay ((long)100);
	 i -= 2;
	 if (CHECKBREAK())
	    break;
      }
   }
   return (0);
}


do_number()
{
   return (0);
}

do_cat()
{
   FILE *fopen(), *fi;
   int i;
   char buf[256];

   if (ac == 1) {
      while (gets(buf)) {
	 if (CHECKBREAK()) break;
	 puts(buf);
	 }
      clearerr(stdin);
      return (0);
   }

   for (i = 1; i < ac; ++i) {
      if ((fi = fopen (av[i], "r")) != 0) {
	   while (fgets(buf,256,fi)) {
	    fputs(buf,stdout);
	    fflush(stdout);
	    if (CHECKBREAK()) {
	       breakreset();
	       break;
	    }
	 }
	 fclose (fi);
      } else {
	 ierror(av[i], 205);
      }
   }
   return (0);
}

do_devinfo()
{
   struct DPTR		*dp;
   struct InfoData	*info;
   int stat,i;
   char *p,*s,*get_pwd(),*index();

   if (ac == 1) {
      ++ac;
      av[1] = "";
   }
   for (i=1; i < ac; ++i) {
      if (!(dp = dopen (av[i], &stat)))
	 continue;
      info = (struct InfoData *)AllocMem((long)sizeof(struct InfoData), MEMF_PUBLIC);
      if (Info (dp->lock, info)) {

	 s = get_pwd(dp->lock);
	 p = index(s,':');
	 *p = '\0';

	 printf ("Unit:%2ld  Errs:%3ld	Used: %-4ld %3ld%% Free: %-4ld	Volume: %s\n",
	     info->id_UnitNumber,
	     info->id_NumSoftErrors,
	     info->id_NumBlocksUsed,
	     (info->id_NumBlocksUsed * 100)/ info->id_NumBlocks,
	     (info->id_NumBlocks - info->id_NumBlocksUsed),
	     s);

      } else {
	     pError (av[i]);
      }
      FreeMem (info,(long) sizeof(*info));
      dclose(dp);
   }
   return(0);
}



/* things shared with display_file */

char  lspec[128];
int   filecount, col;
long  bytes, blocks;

/*
 * the args passed to do_dir will never be expanded
 */
do_dir()
{
   void display_file();
   int i, options;

   col = filecount = 0;
   bytes = blocks = 0L;
   *lspec = '\0';

   options  = get_opt("sfd",&i);

   if (ac == i) {
      ++ac;
      av[i] = "";
   }
   if (!(options & (DIR_FILES | DIR_DIRS)))  options |= (DIR_FILES | DIR_DIRS);

   for (; i < ac; ++i) {
      char **eav;
      int c,eac;
      if (!(eav = expand(av[i], &eac)))
	 continue;
      QuickSort(eav, eac);
      for(c=0;c < eac && !breakcheck();++c) display_file(options,eav[c]);
      free_expand (eav);
      if (CHECKBREAK()) break;
   }
   if (col)  printf("\n");
   if (filecount > 1) {
      blocks += filecount;     /* account for dir blocks */
      printf (" %ld Blocks, %ld Bytes used in %d files\n", blocks, bytes, filecount);
   }
   return (0);
}

void
display_file(options,filestr)
int options;
char *filestr;
{
   long atol();
   int isadir,slen;
   char sc;
   char *c,*s,*fi;
   struct FileLock *lock;
   char *get_pwd();
   char *strcpy();
   
/*     if current dir different from lspec then
       look for ':' or '/' if found lock it and get_pwd.
       else then use cwd.
*/
   for(s = c = filestr; *c; ++c) if (*c == ':' || *c == '/') s = c;
   if (*s == ':') ++s;
   sc = *s;
   *s = '\0';
   c = filestr;
   if (!*c) c = cwd;
   if (strcmp (c, &lspec))  {
      strcpy(lspec, c);
      if (col)	  printf("\n");
      if (lock = (struct FileLock *)Lock(c,SHARED_LOCK)) {
	 printf ("Directory of %s\n", get_pwd(lock));
	 UnLock(lock);
      }
      col = 0;
   }
   *s = sc;
   if (sc == '/') s++;
   slen = strlen(s);
   fi = s + slen + 1;
   isadir = (fi[9] =='D');

   if (!(((options & DIR_FILES) && isadir) ||
	 ((options & DIR_DIRS)	&& !isadir)))
      return;

   if (options & DIR_SHORT) {
      if ((col == 3) && slen >18) {
	 printf("\n");
	 col = 0;
      }
      if (isadir)  {
	 printf ("\033[3m");
      }
      if (slen >18) {
	 printf(" %-37s",s);
	 col += 2;
      }
      else {
	 printf(" %-18s",s);
	 col++;
      }
      if (col > 3) {
	 printf("\n");
	 col = 0;
      }
      if (isadir) printf("\033[0m");
   }
   else		/* print full info */
      printf("   %-24s %s",s ,fi);
   fflush(stdout);
   fi[12] = fi[17] = '\0';
   bytes  += atol(fi+6);
   blocks += atol(fi+13);
   filecount++;
   return;
}

/*
   converts dos date stamp to a time string of form dd-mmm-yy
*/
char *
dates(dss)
struct DateStamp *dss;
{
   register struct tm tm;
   register long time, t;
   register int i;
   static char timestr[20];
   static char months[12][4] = {
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec"
   };
   static char days[12] = {
	31,28,31,30,31,30,31,31,30,31,30,31
   };
   time = dss->ds_Days * 24 * 60 * 60 + dss->ds_Minute * 60 +
				       dss->ds_Tick/TICKS_PER_SECOND;
   tm.tm_sec = time % 60; time /= 60;
   tm.tm_min = time % 60; time /= 60;
   tm.tm_hour= time % 24; time /= 24;
   tm.tm_wday= time %  7;
   tm.tm_year= 78 + (time/(4*365+1)) * 4; time %= 4 * 365 + 1;
   while (time) {
	t = 365;
	if ((tm.tm_year&3) == 0) t++;
	if (time < t) break;
	time -= t;
	tm.tm_year++;
   }
   tm.tm_yday = ++time;
   for (i=0;i<12;i++) {
	t = days[i];
	if (i == 1 && (tm.tm_year&3) == 0) t++;
	if (time <= t) break;
	time -= t;
   }
   tm.tm_mon = i;
   tm.tm_mday = time;

   sprintf(timestr,"%02d-%s-%2d %02d:%02d:%02d\n",tm.tm_mday,
		months[tm.tm_mon],tm.tm_year,
		tm.tm_hour,tm.tm_min,tm.tm_sec);
   return(timestr);

}

date()
{
   struct   DateStamp	dss;
   char *dates();

   DateStamp(&dss);
   printf("%s",dates(&dss));
   return(0);
}

do_quit()
{
   if (Src_stack) {
      Quit = 1;
      return(do_return());
   }
   main_exit (0);
}


do_echo(str)
char *str;
{
   register char *ptr;
   char nl = 1;

   for (ptr = str; *ptr && *ptr != ' '; ++ptr);
   if (*ptr == ' ')
      ++ptr;
   if (av[1] && strcmp (av[1], "-n") == 0) {
      nl = 0;
      ptr += 2;
      if (*ptr == ' ')
	 ++ptr;
   }
   printf("%s",ptr);
   fflush(stdout);
   if (nl)
      printf("\n");
   return (0);
}

do_source(str)
char *str;
{
   register FILE *fi;
   char buf[256];

   if (Src_stack == MAXSRC) {
      fprintf (stderr,"Too many source levels\n");
      return(-1);
   }
   if ((fi = fopen (av[1], "r")) == 0) {
      ierror(av[1], 205);
      return(-1);
   }
   set_var(LEVEL_SET, V_PASSED, next_word(next_word(str)));
   ++H_stack;
   Src_pos[Src_stack] = 0;
   Src_base[Src_stack] = (long)fi;
   ++Src_stack;
   while (fgets (buf, 256, fi)) {
      buf[strlen(buf)-1] = '\0';
      Src_pos[Src_stack - 1] += 1+strlen(buf);
      if (Verbose)
	 fprintf(stderr,"%s\n",buf);
      exec_command (buf);
      if (CHECKBREAK())
	 break;
   }
   --H_stack;
   --Src_stack;
   unset_level(LEVEL_LABEL + Src_stack);
   unset_var(LEVEL_SET, V_PASSED);
   fclose (fi);
   return (0);
}


/*
 * return ptr to string that contains full cwd spec.
 */
char *
get_pwd(flock)
struct FileLock *flock;
{
   char *ptr;
   char *name;
   int err=0;
   static char pwdstr[256];

   struct FileLock *lock, *newlock;
   FIB *fib;
   int i, len;

   lock = (struct FileLock *)DupLock(flock);
         
   fib = (FIB *)AllocMem((long)sizeof(FIB), MEMF_PUBLIC);
   pwdstr[i = 255] = '\0';

   while (lock) {
      newlock = (struct FileLock *)ParentDir(lock);
      if (!Examine(lock, fib)) ++err;
      name = fib->fib_FileName;
      if (*name == '\0')	    /* HACK TO FIX RAM: DISK BUG */
	 name = "RAM";
      len = strlen(name);
      if (newlock) {
	 if (i == 255) {
	    i -= len;
	    bmov(name, pwdstr + i, len);
	 } else {
	    i -= len + 1;
	    bmov(name, pwdstr + i, len);
	    pwdstr[i+len] = '/';
	 }
      } else {
	 i -= len + 1;
	 bmov(name, pwdstr + i, len);
	 pwdstr[i+len] = ':';
      }
      UnLock(lock);
      lock = newlock;
   }
   FreeMem(fib, (long)sizeof(FIB));
   movmem(pwdstr + i, pwdstr, 256 - i);
   if (err) return(cwd);
   return(pwdstr);
}

/*
 * set process cwd name and $_cwd, if str != NULL also print it.
 */
do_pwd(str)
char *str;
{
   char *ptr;

   if ((struct FileLock *)Myprocess->pr_CurrentDir == 0)
       attempt_cd(":"); /* if we just booted 0 = root lock */
   strcpy(cwd,get_pwd(Myprocess->pr_CurrentDir,1));
   if (str)
      puts(cwd);
   set_var(LEVEL_SET, V_CWD, cwd);
   /* put the current dir name in our CLI task structure */
   ptr = (char *)((ULONG)((struct CommandLineInterface *)
      BADDR(Myprocess->pr_CLI))->cli_SetName << 2);
   ptr[0] = strlen(cwd);
   movmem(cwd,ptr+1,(int)ptr[0]);
   return(0);
}


/*
 * CD
 *
 * CD(str, 0)	    -do CD operation.
 *
 *    standard operation: breakup path by '/'s and process independantly
 *    x:    -reset cwd base
 *    ..    -remove last cwd element
 *    N	    -add N or /N to cwd
 */

do_cd(str)
char *str;
{
   char sc, *ptr;
   int err=0;

   str = next_word(str);
   if (*str == '\0') {
      puts(cwd);
      return(0);
   }
   str[strlen(str)+1] = '\0';	       /* add second \0 on end */
   while (*str) {
      for (ptr = str; *ptr && *ptr != '/' && *ptr != ':'; ++ptr);
      switch (*ptr) {
      case ':':
	 sc = ptr[1];
	 ptr[1] = '\0';
	 err = attempt_cd(str);
	 ptr[1] = sc;
	 break;
      case '\0':
      case '/':
	 *ptr = '\0';
	 if (strcmp(str, "..") == 0 || str == ptr)
	    str = "/";
	 if (*str) err = attempt_cd(str);
	 break;
      }
      if (err) break;
      str = ptr + 1;
   }
   do_pwd(NULL);	 /* set $_cwd */
   return(err);
}

attempt_cd(str)
char *str;
{
   struct FileLock *oldlock, *filelock;

   if (filelock = (struct FileLock *)Lock(str, ACCESS_READ)) {
      if (isdir(str)) {
	 if (oldlock = (struct FileLock *)CurrentDir(filelock))
	    UnLock(oldlock);
	 return (0);
      }
      UnLock(filelock);
      ierror(str, 212);
   } else {
      ierror(str, 205);
   }
   return (-1);
}

do_mkdir()
{
   register int i;
   register struct FileLock *lock;

   for (i = 1; i < ac; ++i) {
      if (lock = (struct FileLock *)Lock(av[i],ACCESS_READ)) {
	 ierror(av[i],203);
	 UnLock (lock);
	 continue;
      }
      if (lock = (struct FileLock *)CreateDir (av[i])) {
	 UnLock (lock);
	 continue;
      }
      pError (av[i]);
   }
   return (0);
}


do_mv()
{
   char dest[256];
   register int i;
   char *str;

   --ac;
   if (isdir(av[ac])) {
      for (i = 1; i < ac; ++i) {
	 str = av[i] + strlen(av[i]) - 1;
	 while (str != av[i] && *str != '/' && *str != ':')
	    --str;
	 if (str != av[i])
	    ++str;
	 if (*str == 0) {
	    ierror(av[i], 508);
	    return (-1);
	 }
	 strcpy(dest, av[ac]);
	 if (dest[strlen(dest)-1] != ':')
	    strcat(dest, "/");
	 strcat(dest, str);
	 if (Rename(av[i], dest) == 0)
	    break;
      }
      if (i == ac)
	 return (1);
   } else {
      i = 1;
      if (ac != 2) {
	 ierror("", 507);
	 return (-1);
      }
      if (Rename (av[1], av[2]))
	 return (0);
   }
   pError (av[i]);
   return (-1);
}

rm_file(file)
char *file;
{
      if (has_wild) printf("  %s...",file);
      fflush(stdout);
      if (!DeleteFile(file))
	 pError (file);
      else
	 if (has_wild) printf("Deleted\n");
}

do_rm()
{
   int i, recur;

   recur = get_opt("r",&i);

   for (; i < ac; ++i) {
      if (CHECKBREAK()) break;
      if (isdir(av[i]) && recur)
	 rmdir(av[i]);
      if (!(recur && av[i][strlen(av[i])-1] == ':'))
	 rm_file(av[i]);
   }
   return (0);
}

rmdir(name)
char *name;
{
   register struct FileLock *lock, *cwd;
   register FIB *fib;
   register char *buf;

   buf = (char *)AllocMem(256L, MEMF_PUBLIC);
   fib = (FIB *)AllocMem((long)sizeof(FIB), MEMF_PUBLIC);

   if (lock = (struct FileLock *)Lock(name, ACCESS_READ)) {
      cwd = (struct FileLock *) CurrentDir(lock);
      if (Examine(lock, fib)) {
	 buf[0] = 0;
	 while (ExNext(lock, fib)) {
	    if (CHECKBREAK()) break;
	    if (isdir(fib->fib_FileName))
	       rmdir(fib->fib_FileName);
	    if (buf[0]) {
	       rm_file(buf);
	    }
	    strcpy(buf, fib->fib_FileName);
	 }
	 if (buf[0] && !CHECKBREAK()) {
	    rm_file(buf);
	 }
      }
      UnLock(CurrentDir(cwd));
   } else {
      pError(name);
   }
   FreeMem(fib, (long)sizeof(FIB));
   FreeMem(buf, 256L);
}



do_history()
{
   register struct HIST *hist;
   register int i = H_tail_base;
   register int len = (av[1]) ? strlen(av[1]) : 0;

   for (hist = H_tail; hist; hist = hist->prev) {
      if (len == 0 || strncmp(av[1], hist->line, len) == 0) {
	 printf ("%3d ", i);
	 puts (hist->line);
      }
      ++i;
      if (CHECKBREAK())
	 break;
   }
   return (0);
}

do_mem()
{
   long cfree, ffree;
   extern long AvailMem();

   Forbid();
   cfree = AvailMem (MEMF_CHIP);
   ffree = AvailMem (MEMF_FAST);
   Permit();

   if (ffree)	    {
   printf ("FAST memory: %ld\n", ffree);
   printf ("CHIP memory: %ld\n", cfree);
   }
   printf ("Total  Free: %ld\n", cfree + ffree);
   return(0);
}

/*
 * foreach var_name  ( str str str str... str ) commands
 * spacing is important (unfortunetly)
 *
 * ac=0	   1 2 3 4 5 6 7
 * foreach i ( a b c ) echo $i
 * foreach i ( *.c )   "echo -n "file ->";echo $i"
 */

do_foreach()
{
   register int i, cstart, cend, old;
   register char *cstr, *vname, *ptr, *scr, *args;

   cstart = i = (*av[2] == '(') ? 3 : 2;
   while (i < ac) {
      if (*av[i] == ')')
	 break;
      ++i;
   }
   if (i == ac) {
      fprintf (stderr,"')' expected\n");
      return (-1);
   }
   ++H_stack;
   cend = i;
   vname = strcpy(malloc(strlen(av[1])+1), av[1]);
   cstr = compile_av (av, cend + 1, ac);
   ptr = args = compile_av (av, cstart, cend);
   while (*ptr) {
      while (*ptr == ' ' || *ptr == 9)
	 ++ptr;
      scr = ptr;
      if (*scr == '\0')
	 break;
      while (*ptr && *ptr != ' ' && *ptr != 9)
	 ++ptr;
      old = *ptr;
      *ptr = '\0';
      set_var (LEVEL_SET, vname, scr);
      if (CHECKBREAK())
	 break;
      exec_command (cstr);
      *ptr = old;
   }
   --H_stack;
   free (args);
   free (cstr);
   unset_var (LEVEL_SET, vname);
   free (vname);
   return (0);
}


do_forever(str)
char *str;
{
   int rcode = 0;
   char *ptr = next_word(str);

   ++H_stack;
   for (;;) {
      if (CHECKBREAK()) {
	 rcode = 20;
	 break;
      }
      if (exec_command (ptr) < 0) {
	 str = get_var(LEVEL_SET, V_LASTERR);
	 rcode = (str) ? atoi(str) : 20;
	 break;
      }
   }
   --H_stack;
   return (rcode);
}

