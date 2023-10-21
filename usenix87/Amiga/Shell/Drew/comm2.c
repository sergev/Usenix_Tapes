
/*
 * COMM2.C
 *
 * (c)1986 Matthew Dillon     9 October 1986
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 */

#include "shell.h"
typedef struct FileInfoBlock FIB;

/* Casting conveniences */
#define BPTR_TO_C(strtag, var)	((struct strtag *)(BADDR( (ULONG) var)))
#define PROC(task)		((struct Process *)task)
#define ROOTNODE		((struct RootNode *)DOSBase->dl_Root)
#define CLI(proc)		(BPTR_TO_C(CommandLineInterface, proc->pr_CLI))

/* Externs */
extern int has_wild;			/* flag set if any arg has a ? or * */
extern struct DosLibrary *DOSBase;	/* dos library base pointer         */

/* globals */
int cp_update;
int cp_date;

do_abortline()
{
   Exec_abortline = 1;
   return (0);
}

do_return()
{
   Exec_abortline = 1;
   if (Src_stack) {
      fseek (Src_base[Src_stack - 1], 0, 2);
      return ((ac < 2) ? 0 : atoi(av[1]));
   } else {
      main_exit ((ac < 2) ? 0 : atoi(av[1]));
   }
}

/*
 * STRHEAD
 *
 * place a string into a variable removing everything after and including
 * the 'break' character or until a space is found in the string.
 *
 * strhead varname breakchar string
 *
 */

do_strhead()
{
   register char *str = av[3];
   char bc = *av[2];

   while (*str && *str != bc)
      ++str;
   *str = '\0';
   set_var (LEVEL_SET, av[1], av[3]);
   return (0);
}

do_strtail()
{
   register char *str = av[3];
   char bc = *av[2];

   while (*str && *str != bc)
      ++str;
   if (*str)
      ++str;
   set_var (LEVEL_SET, av[1], str);
   return (0);
}



/*
 * if A < B   <, >, =, <=, >=, !=, where A and B are either:
 * nothing
 * a string
 * a value (begins w/ number)
 */

do_if(garbage, com)
char *garbage;
{
   char *v1, *v2, *v3, result, num;
   int n1, n2;

   switch (com) {
   case 0:
      if (If_stack && If_base[If_stack - 1]) {
	 If_base[If_stack++] = 1;
	 break;
      }
      result = num = 0;
      if (ac <= 2) {	   /* if $var; */
	 if (ac == 1 || strlen(av[1]) == 0 || (strlen(av[1]) == 1 && *av[1] == ' '))
	    goto do_result;
	 result = 1;
	 goto do_result;
      }
      if (ac != 4) {
	 ierror(NULL, 500);
	 break;
      }
      v1 = av[1]; v2 = av[2]; v3 = av[3];
      while (*v1 == ' ')
	 ++v1;
      while (*v2 == ' ')
	 ++v2;
      while (*v3 == ' ')
	 ++v3;
      if (*v1 >= '0' && *v1 <= '9') {
	 num = 1;
	 n1 = atoi(v1);
	 n2 = atoi(v3);
      }
      while (*v2) {
	 switch (*v2++) {
	 case '>':
	    result |= (num) ? (n1 >  n2) : (strcmp(v1, v3) > 0);
	    break;
	 case '<':
	    result |= (num) ? (n1 <  n2) : (strcmp(v1, v3) < 0);
	    break;
	 case '=':
	    result |= (num) ? (n1 == n2) : (strcmp(v1, v3) ==0);
	    break;
	 default:
	    ierror (NULL, 503);
	    break;
	 }
      }
do_result:
      If_base[If_stack++] = !result;
      break;
   case 1:
      if (If_stack > 1 && If_base[If_stack - 2])
	 break;
      if (If_stack)
	 If_base[If_stack - 1] ^= 1;
      break;
   case 2:
      if (If_stack)
	 --If_stack;
      break;
   }
   disable = (If_stack) ? If_base[If_stack - 1] : 0;
   if (If_stack >= MAXIF) {
      fprintf(stderr,"If's too deep\n");
      disable = If_stack = 0;
      return(-1);
      }
   return (0);
}

do_label()
{
   char aseek[32];

   if (Src_stack == 0) {
      ierror (NULL, 502);
      return (-1);
   }
   sprintf (aseek, "%ld %d", Src_pos[Src_stack-1], If_stack);
   set_var (LEVEL_LABEL + Src_stack - 1, av[1], aseek);
   return (0);
}

do_goto()
{
   int new;
   long pos;
   char *lab;

   if (Src_stack == 0) {
      ierror (NULL, 502);
   } else {
      lab = get_var (LEVEL_LABEL + Src_stack - 1, av[1]);
      if (lab == NULL) {
	 ierror (NULL, 501);
      } else {
	 pos = atoi(lab);
	 fseek (Src_base[Src_stack - 1], pos, 0);
	 Src_pos[Src_stack - 1] = pos;
	 new = atoi(next_word(lab));
	 for (; If_stack < new; ++If_stack)
	    If_base[If_stack] = 0;
	 If_stack = new;
      }
   }
   Exec_abortline = 1;
   return (0);	    /* Don't execute rest of this line */
}


do_inc(garbage, com)
char *garbage;
{
   char *var;
   char num[32];

   if (ac == 3)
      com = atoi(av[2]);
   var = get_var (LEVEL_SET, av[1]);
   if (var) {
      sprintf (num, "%d", atoi(var)+com);
      set_var (LEVEL_SET, av[1], num);
   }
   return (0);
}

do_input()
{
   char in[256];

   if ((gets(in)) != 0)
      set_var (LEVEL_SET, av[1], in);
   return (0);
}

do_ver()
{
   puts (VERSION);
   return (0);
}


do_ps()
{
	/* this code fragment based on ps.c command by Dewi Williams */

	register ULONG	 *tt;		/* References TaskArray		*/
	register int	 count;		/* loop variable		*/
	register UBYTE	 *port;		/* msgport & ptr arith		*/
	register struct Task *task;	/* EXEC descriptor		*/
	char		 strbuf[64];   /* scratch for btocstr()	       */
	char		 *btocstr();	/* BCPL BSTR to ASCIIZ		*/

	tt = (unsigned long *)(BADDR(ROOTNODE->rn_TaskArray));

	printf("Proc Command Name	  CLI Type    Pri.  Address  Directory\n");
	Forbid();		/* need linked list consistency */

	for (count = 1; count <= (int)tt[0] ; count++) {/* or just assume 20?*/
		if (tt[count] == 0) continue;		/* nobody home */

		/* Start by pulling out MsgPort addresses from the TaskArray
		 * area. By making unwarranted assumptions about the layout
		 * of Process and Task structures, we can derive these
		 * descriptors. Every task has an associated process, since
		 * this loop drives off a CLI data area.
		 */

		port = (UBYTE *)tt[count];
		task = (struct Task *)(port - sizeof(struct Task));

		/* Sanity check just in case */
		if (PROC(task)->pr_TaskNum == 0 || PROC(task)->pr_CLI == 0)
			continue;		/* or complain? */

			btocstr(CLI(PROC(task))->cli_CommandName, strbuf);
			printf("%2d   %-21s",count,strbuf);
			strcpy(strbuf,task->tc_Node.ln_Name);
			strbuf[11] = '\0';
			printf("%-11s",strbuf);
			printf(" %3d  %8lx  %s\n",
			   task->tc_Node.ln_Pri,task,
			   btocstr(CLI(PROC(task))->cli_SetName, strbuf));
	}
	Permit();		/* outside critical region */
	return(0);
}


char *
btocstr(b, buf)
ULONG	b;
char	*buf;
{
	register char	*s;

	s = (char *)BADDR(b);	/* Shift & get length-prefixed str */
	movmem(s +1, buf, s[0]);
	buf[s[0]] = '\0';
	return buf;
}


/*
 * CP [-d] [-u] file file
 * CP [-d] [-u] file file file... destdir
 * CP [-r][-u][-d] dir dir dir... destdir
 */

char *errstr;	       /* let's be alittle more informative */
do_copy()
{
   register int recur, ierr;
   register char *destname;
   register char destisdir;
   register FIB *fib;
   int i,opt;

   errstr = "";
   ierr = 0;

   fib = (FIB *)AllocMem((long)sizeof(FIB), MEMF_PUBLIC);

   opt = get_opt("rud",&i);
   recur     = (opt & 0x01);
   cp_update = (opt & 0x02);
   cp_date   = (!(opt & 0x04)); /* the default is keep orignal file date */

   destname = av[ac - 1];

   if (ac < i + 2) {
      ierr = 500;
      goto done;
   }

   destisdir = isdir(destname);
   if (ac > i + 2 && !destisdir) {
      ierr = 507;
      goto done;
   }

   /*
    * copy set:			       reduce to:
    *	 file to file			  file to file
    *	 dir  to file (NOT ALLOWED)
    *	 file to dir			  dir to dir
    *	 dir  to dir			  dir to dir
    *
    */

   for (; i < ac - 1; ++i) {
      short srcisdir = isdir(av[i]);
      if (srcisdir && has_wild && (ac >2)) /* hack to stop dir's from */
	  continue;			   /* getting copied if specified */
					   /* from wild expansion */
      if (CHECKBREAK())
	 break;
      if (srcisdir) {
	 struct FileLock *srcdir, *destdir;
	 if (!destisdir) {
	    if (destdir = (struct FileLock *)Lock(destname, ACCESS_READ)) {
	       UnLock(destdir);
	       ierr = 507;		  /* disallow dir to file */
	       goto done;
	    }
	    if (destdir = (struct FileLock *)CreateDir(destname))
	       UnLock(destdir);
	    destisdir = 1;
	 }
	 if (!(destdir = (struct FileLock *)Lock(destname, ACCESS_READ))) {
	    ierr = 205;
	    errstr = destname;
	    goto done;
	 }
	 if (!(srcdir = (struct FileLock *)Lock(av[i], ACCESS_READ))) {
	    ierr = 205;
	    errstr = av[i];
	    UnLock(destdir);
	    goto done;
	 }
	 ierr = copydir(srcdir, destdir, recur);
	 UnLock(srcdir);
	 UnLock(destdir);
	 if (ierr)
	    break;
      } else {			    /* FILE to DIR,   FILE to FILE   */
	 struct FileLock *destdir, *srcdir, *tmp;
	 char *destfilename;

	 srcdir = (struct FileLock *)(Myprocess->pr_CurrentDir);

	 if ((tmp = (struct FileLock *)Lock(av[i], ACCESS_READ)) == NULL || !Examine(tmp,fib)){
	    if (tmp) UnLock(tmp);
	    ierr = 205;
	    errstr = av[i];
	    goto done;
	 }
	 UnLock(tmp);
	 if (destisdir) {
	    destdir = (struct FileLock *)Lock(destname, ACCESS_READ);
	    destfilename = fib->fib_FileName;
	 } else {
	    destdir = srcdir;
	    destfilename = destname;
	 }
	 printf(" %s..",av[i]);
	 fflush(stdout);
	 ierr = copyfile(av[i], srcdir, destfilename, destdir);
	 if (destisdir)
	    UnLock(destdir);
	 if (ierr)
	    break;
      }
   }
done:
   FreeMem(fib, (long)sizeof(*fib));
   if (ierr) {
      ierror(errstr, ierr);
      return(20);
   }
   return(0);
}


copydir(srcdir, destdir, recur)
register struct FileLock *srcdir, *destdir;
{
   struct FileLock *cwd;
   register FIB *srcfib;
   register struct FileLock *destlock, *srclock;
   int ierr;
   static int level;

   level++;
   ierr = 0;
   srcfib = (FIB *)AllocMem((long)sizeof(FIB), MEMF_PUBLIC);
   if (Examine(srcdir, srcfib)) {
      while (ExNext(srcdir, srcfib)) {
	 if (CHECKBREAK())
	    break;
	 if (srcfib->fib_DirEntryType < 0) {
	    printf("%*s%s..",(level-1) * 6," ",srcfib->fib_FileName);
	    fflush(stdout);
	    ierr = copyfile(srcfib->fib_FileName,srcdir,srcfib->fib_FileName,destdir);
	    if (ierr)
	       break;
	 } else {
	    if (recur) {
	       cwd = (struct FileLock *)CurrentDir(srcdir);
	       if (srclock = (struct FileLock *)Lock(srcfib->fib_FileName, ACCESS_READ)) {
		  CurrentDir(destdir);
		  if (!(destlock = (struct FileLock *)
				Lock(srcfib->fib_FileName))) {
		     destlock = (struct FileLock *)CreateDir(srcfib->fib_FileName);
		     printf("%*s%s (Dir)....[Created]\n",(level-1) * 6,
				" ",srcfib->fib_FileName);

 			/* UnLock and re Lock if newly created dir
 			   for file_date() to work properly.
 			*/   
 		     if (destlock) UnLock(destlock);
 		     destlock = (struct FileLock *)Lock(srcfib->fib_FileName);		     
		  }
		  else
		     printf("%*s%s (Dir)\n",(level-1) * 6," ",srcfib->fib_FileName);
		  if (destlock) {
		     ierr = copydir(srclock, destlock, recur);
		     UnLock(destlock);
		  } else {
		     ierr = (int)((long)IoErr());
		  }
		  UnLock(srclock);
	       } else {
		  ierr = (int)((long)IoErr());
	       }
	       CurrentDir(cwd);
	       if (ierr)
		  break;
	    }
	 }
      }
   } else {
      ierr = (int)((long)IoErr());
   }
   --level;
   FreeMem(srcfib, (long)sizeof(FIB));
   return(ierr);
}


copyfile(srcname, srcdir, destname, destdir)
char *srcname, *destname;
struct FileLock *srcdir, *destdir;
{
   struct FileLock *cwd;
   struct FileHandle *f1, *f2;
   struct DateStamp *ds;
   long i;
   int stat,ierr;
   char *buf;
   struct DPTR		*dp, *dps = NULL;

   buf = (char *)AllocMem(8192L, MEMF_PUBLIC|MEMF_CLEAR);
   if (buf == NULL) {
      ierr = 103;
      goto fail;
   }

   ierr = 0;
   cwd = (struct FileLock *)CurrentDir(srcdir);
   f1 = Open(srcname, MODE_OLDFILE);
   if (f1 == NULL) {
      errstr = srcname;
      ierr = 205;
      goto fail;
   }
   dps = dopen(srcname,&stat);
   ds = &dps->fib->fib_Date;
   CurrentDir(destdir);
   if (cp_update  && (dp = dopen (destname, &stat))) {
	long desttime,srctime;
	struct DateStamp *dd;

	dd = &dp->fib->fib_Date;
	desttime = dd->ds_Days * 24 * 60 * 60 + dd->ds_Minute * 60 +
				       dd->ds_Tick/TICKS_PER_SECOND;
	srctime	 = ds->ds_Days * 24 * 60 * 60 + ds->ds_Minute * 60 +
				       ds->ds_Tick/TICKS_PER_SECOND;

	if (srctime <= desttime &&
	  !strcmp(dps->fib->fib_FileName,dp->fib->fib_FileName)) {
	    dclose(dp);
	    Close(f1);
	    printf("..not newer\n");
	    goto fail;
	}
	dclose(dp);
   }
   f2 = Open(destname, MODE_NEWFILE);
   if (f2 == NULL) {
      Close(f1);
      ierr = (int)((long)IoErr());
      errstr = destname;
      goto fail;
   }
   while (i = Read(f1, buf, 8192L))
      if (Write(f2, buf, i) != i) {
	 ierr = (int)((long)IoErr());
	 break;
      }
   Close(f2);
   Close(f1);
   if (!ierr)  {
#ifdef AZTEC_C
      if (cp_date) file_date(ds,destname);
#endif
      printf("..copied\n");
   }
   else {
      DeleteFile(destname);
      printf("..Not copied..");
   }
fail:
   dclose(dps);
   if (buf)
      FreeMem(buf, 8192L);
   CurrentDir(cwd);
   return(ierr);
}

#ifdef AZTEC_C		 /* since 3.20a did'nt have dos_packet() */

file_date(date,name)
struct DateStamp *date;
char *name;
{
    UBYTE *ptr;
    struct MsgPort *task;
    struct FileLock *dirlock;
    struct DPTR *tmp; 
    int stat;
    long ret, dos_packet();

    if (!(task = (struct MsgPort *)DeviceProc(name)))
	return(1);
    if (tmp = dopen(name, &stat)) {
	dirlock = (struct FileLock *)ParentDir(tmp->lock);
	ptr = (UBYTE *)AllocMem(64L,MEMF_PUBLIC);
	strcpy((ptr + 1),tmp->fib->fib_FileName);
	*ptr = strlen(tmp->fib->fib_FileName);
	dclose(tmp);
	ret = dos_packet(task,34L,NULL,dirlock,
		         (ULONG)&ptr[0] >> 2L,date);
	FreeMem(ptr,64L);
	UnLock(dirlock);
    }
}

#endif

