
/*
 * SUB.C
 *
 * (c)1986 Matthew Dillon     9 October 1986
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 */

#include "shell.h"

#define HM_STR 0	      /* various HISTORY retrieval modes */
#define HM_REL 1
#define HM_ABS 2

/* extern struct FileLock *Clock; */
seterr()
{
   char buf[32];
   int stat;

   sprintf(buf, "%d", Lastresult);
   set_var(LEVEL_SET, V_LASTERR, buf);
   stat = atoi(get_var(LEVEL_SET, V_STAT));
   if (stat < Lastresult)
      stat = Lastresult;
   sprintf(buf, "%d", stat);
   set_var(LEVEL_SET, V_STAT, buf);
}


char *
next_word(str)
register char *str;
{
   while (*str	&&  *str != ' '	 &&  *str != 9)
      ++str;
   while (*str	&& (*str == ' ' || *str == 9))
      ++str;
   return (str);
}


char *
compile_av(av, start, end)
char **av;
{
   char *cstr;
   int i, len;

   len = 0;
   for (i = start; i < end; ++i)
      len += strlen(av[i]) + 1;
   cstr = malloc(len + 1);
   *cstr = '\0';
   for (i = start; i < end; ++i) {
      strcat (cstr, av[i]);
      strcat (cstr, " ");
   }
   return (cstr);
}

/*
 * FREE(ptr)   --frees without actually freeing, so the data is still good
 *		 immediately after the free.
 */


Free(ptr)
char *ptr;
{
   static char *old_ptr;

   if (old_ptr)
      free (old_ptr);
   old_ptr = ptr;
}

/*
 * Add new string to history (H_head, H_tail, H_len,
 *  S_histlen
 */

add_history(str)
char *str;
{
   register struct HIST *hist;

   if (H_head != NULL && strcmp(H_head->line, str) == 0)
       return(0);
   while (H_len > S_histlen)
      del_history();
   hist = (struct HIST *)malloc (sizeof(struct HIST));
   if (H_head == NULL) {
      H_head = H_tail = hist;
      hist->next = NULL;
   } else {
      hist->next = H_head;
      H_head->prev = hist;
      H_head = hist;
   }
   hist->prev = NULL;
   hist->line = malloc (strlen(str) + 1);
   strcpy (hist->line, str);
   ++H_len;
}

del_history()
{
   if (H_tail) {
      --H_len;
      ++H_tail_base;
      free (H_tail->line);
      if (H_tail->prev) {
	 H_tail = H_tail->prev;
	 free (H_tail->next);
	 H_tail->next = NULL;
      } else {
	 free (H_tail);
	 H_tail = H_head = NULL;
      }
   }
}

char *
get_history(ptr)
char *ptr;
{
   register struct HIST *hist;
   register int len;
   int mode = HM_REL;
   int num  = 1;
   char *str;
   char *result = NULL;

   if (ptr[1] >= '0' && ptr[1] <= '9') {
      mode = HM_ABS;
      num  = atoi(&ptr[1]);
      goto skip;
   }
   switch (ptr[1]) {
   case '!':
      break;
   case '-':
      num += atoi(&ptr[2]);
      break;
   default:
      mode = HM_STR;
      str  = ptr + 1;
      break;
   }
skip:
   switch (mode) {
   case HM_STR:
      len = strlen(str);
      for (hist = H_head; hist; hist = hist->next) {
	 if (strncmp(hist->line, str, len) == 0 && *hist->line != '!') {
	    result = hist->line;
	    break;
	 }
      }
      break;
   case HM_REL:
      for (hist = H_head; hist && num--; hist = hist->next);
      if (hist)
	 result = hist->line;
      break;
   case HM_ABS:
      len = H_tail_base;
      for (hist = H_tail; hist && len != num; hist = hist->prev, ++len);
      if (hist)
	 result = hist->line;
      break;
   }
   if (result) {
      fprintf(stderr,"%s\n",result);
      return(result);
   }
   printf("History failed\n");
   return ("");
}

replace_head(str)
char *str;
{
   if (str == NULL)
      str = "";
   if (H_head) {
      free (H_head->line);
      H_head->line = malloc (strlen(str)+1);
      strcpy (H_head->line, str);
   }
}


pError(str)
char *str;
{
   int ierr = (long)IoErr();
   ierror(str, ierr);
}

ierror(str, err)
register char *str;
{
   register struct PERROR *per = Perror;

   if (err) {
      for (; per->errstr; ++per) {
	 if (per->errnum == err) {
	    fprintf (stderr, "%s%s%s\n",
		  per->errstr,
		  (str) ? ": " : "",
		  (str) ? str : "");
	    return ((short)err);
	 }
      }
      fprintf (stderr, "Unknown DOS error %ld %s\n", err, (str) ? str : "");
   }
   return ((short)err);
}

/*
 * Disk directory routines
 *
 * dptr = dopen(name, stat)
 *    struct DPTR *dptr;
 *    char *name;
 *    int *stat;
 *
 * dnext(dptr, name, stat)
 *    struct DPTR *dptr;
 *    char **name;
 *    int  *stat;
 *
 * dclose(dptr)			 -may be called with NULL without harm
 *
 * dopen() returns a struct DPTR, or NULL if the given file does not
 * exist.  stat will be set to 1 if the file is a directory.  If the
 * name is "", then the current directory is openned.
 *
 * dnext() returns 1 until there are no more entries.  The **name and
 * *stat are set.  *stat = 1 if the file is a directory.
 *
 * dclose() closes a directory channel.
 *
 */

struct DPTR *
dopen(name, stat)
char *name;
int *stat;
{
   struct DPTR *dp;

   *stat = 0;
   dp = (struct DPTR *)malloc(sizeof(struct DPTR));
   if (*name == '\0')
      dp->lock = (struct FileLock *)DupLock ((struct FileLock *)Myprocess->pr_CurrentDir);
   else
      dp->lock = (struct FileLock *)Lock (name, ACCESS_READ);
   if (dp->lock == NULL) {
      free (dp);
      return (NULL);
   }
   dp->fib = (struct FileInfoBlock *)
	 AllocMem((long)sizeof(struct FileInfoBlock), MEMF_PUBLIC);
   if (!Examine (dp->lock, dp->fib)) {
      pError (name);
      dclose (dp);
      return (NULL);
   }
   if (dp->fib->fib_DirEntryType >= 0)
      *stat = 1;
   return (dp);
}

dnext(dp, pname, stat)
struct DPTR *dp;
char **pname;
int *stat;
{
   if (dp == NULL)
      return (0);
   if (ExNext (dp->lock, dp->fib)) {
      *stat = (dp->fib->fib_DirEntryType < 0) ? 0 : 1;
      *pname = dp->fib->fib_FileName;
      return (1);
   }
   return (0);
}


dclose(dp)
struct DPTR *dp;
{
   if (dp == NULL)
      return (1);
   if (dp->fib)
      FreeMem (dp->fib,(long)sizeof(*dp->fib));
   if (dp->lock)
      UnLock (dp->lock);
   free (dp);
   return (1);
}


isdir(file)
char *file;
{
   register struct DPTR *dp;
   int stat;

   stat = 0;
   if (dp = dopen (file, &stat))
      dclose(dp);
   return (stat == 1);
}


free_expand(av)
register char **av;
{
   char **base = av;

   if (av) {
      while (*av) {
	 free (*av);
	 ++av;
      }
      free (base);
   }
}

/*
 * EXPAND(base,pac)
 *    base	     - char * (example: "df0:*.c")
 *    pac	     - int  *  will be set to # of arguments.
 *
 * 22-May-87 SJD.  Heavily modified to allow recursive wild carding and
 *		   simple directory/file lookups. Returns a pointer to
 *		   an array of pointers that contains the full file spec
 *		   eg. 'df0:c/sear*' would result in : 'df0:C/Search'
 *
 *		   Now no longer necessary to Examine the files a second time
 *		   in do_dir since expand will return the full file info
 *		   appended to the file name. Set by formatfile().
 *		   eg. fullfilename'\0'rwed  NNNNNN NNNN  DD-MMM-YY HH:MM:SS
 *
 *		   Caller must call free_expand when done with the array.
 *
 * base		    bname =	  ename =
 * ------	    -------	  -------
 *  "*"		      ""	    "*"
 *  "!*.info"	      ""	    "*.info" (wild_exclude set)
 *  "su*d/*"	      ""	    "*"	     (tail set)
 *  "file.*"	      ""	    "file.*"
 *  "df0:c/*"	      "df0:c"	    "*"
 *  ""		      ""	    "*"
 *  "df0:.../*"	      "df0:"	    "*"	     (recur set)
 *  "df0:sub/.../*"   "df0:sub"	    "*"	     (recur set)
 *
 * ---the above base would be provided by execom.c or do_dir().
 * ---the below base would only be called from do_dir().
 *
 *  "file.c"	      "file.c"	    ""	     if (dp == 0) fail else get file.c
 *  "df0:"	      "df0:"	    "*"
 *  "file/file"	      "file/file"   ""	     (dp == 0) so fail
 *  "df0:.../"	      "df0:"	    "*"	     (recur set)
 *
 */


char **
expand(base, pac)
char *base;
int *pac;
{
   register char *ptr;
   char **eav = (char **)malloc(sizeof(char *) * (2));
   short eleft, eac;
   char *name;
   char *svfile();
   char *bname, *ename, *tail;
   int stat, recur, scr,wild_exclude,bl;
   register struct DPTR *dp;

   *pac = recur = eleft = eac = wild_exclude = 0;

   base = strcpy(malloc(strlen(base)+1), base);
   for (ptr = base; *ptr && *ptr != '?' && *ptr != '*'; ++ptr);

   if (!*ptr)	/* no wild cards */
      --ptr;
   else
      for (; ptr >= base && !(*ptr == '/' || *ptr == ':'); --ptr);

   if (ptr < base) {
      bname = strcpy (malloc(1), "");
   } else {
      scr = ptr[1];
      ptr[1] = '\0';
      if (!strcmp(ptr-3,".../")) {
	 recur = 1;
	 *(ptr-3) = '\0';
      }
      bname = strcpy (malloc(strlen(base)+2), base);
      ptr[1] = scr;
   }
   bl = strlen(bname);
   ename = ++ptr;
   for (; *ptr && *ptr != '/'; ++ptr);
   scr = *ptr;
   *ptr = '\0';
   if (scr) ++ptr;
   tail = ptr;
   if (*ename == '!') wild_exclude = 1;

   if ((dp = dopen (bname, &stat)) == NULL || (stat == 0 && *ename)) {
      free (bname);
      free (base);
      free (eav);
      return (NULL);
   }

   if (!stat) {		       /* eg. 'dir file' */
      char *p,*s;
      for(s = p = bname; *p; ++p) if (*p == '/' || *p == ':') s = p;
      if (s != bname) ++s;
      *s ='\0';
      eav[eac++] = svfile(bname,dp->fib->fib_FileName,dp->fib);
      goto done;
   }
   if (!*ename) ename = "*";	/* eg. dir df0: */
   if (*bname && bname[bl-1] != ':' && bname[bl-1] != '/') { /* dir df0:c */
      bname[bl] = '/';
      bname[++bl] = '\0';
   }
   while ((dnext (dp, &name, &stat)) && !breakcheck()) {
	int match = compare_ok(ename+wild_exclude,name);

      if (wild_exclude) match ^= 1;
      if (match && !(!recur && *tail)) {
	 if (eleft < 2) {
	       char **scrav = (char **)malloc(sizeof(char *) * (eac + 10));
	       movmem (eav, scrav, (eac + 1) << 2);
	       free (eav);
	       eav = scrav;
	       eleft = 10;
	 }
	 eav[eac++] = svfile(bname,name,dp->fib);
	 --eleft;
      }
      if ((*tail && match) || recur) {
	 int alt_ac;
	 char *search, **alt_av, **scrav;
	 struct FileLock *lock;

	 if (!stat)	      /* expect more dirs, but this not a dir */
	    continue;
	 lock = (struct FileLock *)CurrentDir (dp->lock);
	 search = malloc(strlen(ename)+strlen(name)+strlen(tail)+5);
	 strcpy (search, name);
	 strcat (search, "/");
	 if (recur) {
	    strcat(search, ".../");
	    strcat(search, ename);
	 }
	 strcat (search, tail);
	 scrav = alt_av = expand (search, &alt_ac);
	 /* free(search); */
	 CurrentDir (lock);
	 if (scrav) {
	    while (*scrav) {
	       int l;
	       if (eleft < 2) {
		  char **scrav = (char **)malloc(sizeof(char *) * (eac + 10));
		  movmem (eav, scrav, (eac + 1) << 2);
		  free (eav);
		  eav = scrav;
		  eleft = 10;
	       }

	       l = strlen(*scrav);
	       scrav[0][l] = ' ';
	       eav[eac] = malloc(bl+l+40);
	       strcpy(eav[eac], bname);
	       strcat(eav[eac], *scrav);
	       eav[eac][l+bl] = '\0';

	       free (*scrav);
	       ++scrav;
	       --eleft, ++eac;
	    }
	    free (alt_av);
	 }
      }
   }
done:
   dclose (dp);
   *pac = eac;
   eav[eac] = NULL;
   free (bname);
   free (base);
   if (eac) {
      return (eav);
   }
   free (eav);
   return (NULL);
}

/*
 * Compare a wild card name with a normal name
 */

#define MAXB   8

compare_ok(wild, name)
char *wild, *name;
{
   register char *w = wild;
   register char *n = name;
   char *back[MAXB][2];
   register char s1, s2;
   int	bi = 0;

   while (*n || *w) {
      switch (*w) {
      case '*':
	 if (bi == MAXB) {
	    printf(stderr,"Too many levels of '*'\n");
	    return (0);
	 }
	 back[bi][0] = w;
	 back[bi][1] = n;
	 ++bi;
	 ++w;
	 continue;
goback:
	 --bi;
	 while (bi >= 0 && *back[bi][1] == '\0')
	    --bi;
	 if (bi < 0)
	    return (0);
	 w = back[bi][0] + 1;
	 n = ++back[bi][1];
	 ++bi;
	 continue;
      case '?':
	 if (!*n) {
	    if (bi)
	       goto goback;
	    return (0);
	 }
	 break;
      default:
	 s1 = (*n >= 'A' && *n <= 'Z') ? *n - 'A' + 'a' : *n;
	 s2 = (*w >= 'A' && *w <= 'Z') ? *w - 'A' + 'a' : *w;
	 if (s1 != s2) {
	    if (bi)
	       goto goback;
	    return (0);
	 }
	 break;
      }
      if (*n)  ++n;
      if (*w)  ++w;
   }
   return (1);
}


char *
svfile(s1,s2,fib)
char *s1,*s2;
struct FileInfoBlock *fib;
{
   char *p;
   p = malloc (strlen(s1)+strlen(s2)+40);
   strcpy(p, s1);
   strcat(p, s2);
   formatfile(p,fib);
   return(p);
}

/* will have either of these formats:
 *
 *    fullfilename'\0'rwed    <Dir>	 DD-MMM-YY HH:MM:SS\n'\0'
 *    fullfilename'\0'rwed  NNNNNN NNNN	 DD-MMM-YY HH:MM:SS\n'\0'
 *		      0123456789012345678901234567890123456 7  8
 *
 */
formatfile(str,fib)
char *str;
register struct FileInfoBlock *fib;
{
   char *dates();

   while(*str++);
   *str++ = (fib->fib_Protection & FIBF_READ)	 ? '-' : 'r';
   *str++ = (fib->fib_Protection & FIBF_WRITE)	 ? '-' : 'w';
   *str++ = (fib->fib_Protection & FIBF_EXECUTE) ? '-' : 'e';
   *str++ = (fib->fib_Protection & FIBF_DELETE)	 ? '-' : 'd';

   if (fib->fib_DirEntryType < 0)
      sprintf(str,"  %6ld %4ld  ", (long)fib->fib_Size, (long)fib->fib_NumBlocks);
   else
      strcpy(str,"    <Dir>      ");
   strcat(str,dates(&fib->fib_Date));
}
