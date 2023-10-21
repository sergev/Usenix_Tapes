
/*
 * SUB.C
 *
 * Matthew Dillon, 24 Feb 1986
 *
 */


#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include "shell.h"

#define HM_STR 0
#define HM_REL 1
#define HM_ABS 2

extern struct FileLock *Lock(), *DupLock(), *CurrentDir();
extern struct FileLock *Clock;

char *
next_word(str)
register char *str;
{
   while (*str  &&  *str != ' '  &&  *str != 9)
      ++str;
   while (*str  && (*str == ' ' || *str == 9))
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
         if (strncmp(hist->line, str, len) == 0)
            return (hist->line);
      }
      return (NULL);
   case HM_REL:
      for (hist = H_head; hist && num--; hist = hist->next);
      return ((hist) ? hist->line : NULL);
   case HM_ABS:
      len = H_tail_base;
      for (hist = H_tail; hist && len != num; hist = hist->prev, ++len);
      return ((hist) ? hist->line : NULL);
   }
   return (NULL);
}

replace_head(str1, str2, flag)
char *str1, *str2;
{
   char *str3;

   if (str1 == NULL)
      str1 = "";
   if (str2 == NULL) {
      str2 = "";
      flag = 0;
   }
   str3 = ((flag & (FL_EOC|FL_EOL)) == FL_EOC) ? ";" : " ";
   if (H_head) {
      free (H_head->line);
      H_head->line = malloc (strlen(str1)+strlen(str2)+2);
      strcpy (H_head->line, str1);
      strcat (H_head->line, str3);
      strcat (H_head->line, str2);
   }
}

perror(str)
char *str;
{
   struct PERROR *per = Perror;
   int err = IoErr();

   if (err) {
      for (; per->errstr; ++per) {
         if (per->errnum == err) {
            printf ("%s%s%s\n",
                  per->errstr,
                  (str) ? ": " : "",
                  (str) ? str : "");
            return (err);
         }
      }
      printf ("Unknown DOS error %d %s\n", err, (str) ? str : "");
   }
   return (err);
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
 * dclose(dptr)                  -may be called with NULL without harm
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
   int namelen, endslash = 0;

   namelen = strlen(name);
   if (namelen && name[namelen - 1] == '/') {
      name[namelen - 1] = '\0';
      endslash = 1;
   }
   *stat = 0;
   dp = (struct DPTR *)malloc(sizeof(struct DPTR));
   if (*name == '\0')
      dp->lock = DupLock (Clock);
   else
      dp->lock = Lock (name, ACCESS_READ);
   if (endslash)
      name[namelen - 1] = '/';
   if (dp->lock == NULL) {
      free (dp);
      return (NULL);
   }
   dp->fib = (struct FileInfoBlock *)
         AllocMem(sizeof(struct FileInfoBlock), MEMF_PUBLIC);
   if (!Examine (dp->lock, dp->fib)) {
      perror (name);
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
      FreeMem (dp->fib, sizeof(*dp->fib));
   if (dp->lock)
      UnLock (dp->lock);
   free (dp);
   return (1);
}

free_expand(av)
char **av;
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
 * EXPAND(wild_name, pac)
 *    wild_name      - char * (example: "df0:*.c")
 *    pac            - int  *  will be set to # of arguments.
 *
 * Standalone, except in requires Clock to point to the Current-Directory
 * lock.
 */


char **
expand(base, pac)
char *base;
int *pac;
{
   char **eav = (char **)malloc (sizeof(char *));
   int  eleft, eac;

   char *ptr, *name;
   char *bname, *ename, *tail;
   int stat, scr;
   struct DPTR *dp;

   *pac = eleft = eac = 0;

   base = strcpy(malloc(strlen(base)+1), base);
   for (ptr = base; *ptr && *ptr != '?' && *ptr != '*'; ++ptr);
   for (; ptr >= base && !(*ptr == '/' || *ptr == ':'); --ptr);
   if (ptr < base) {
      bname = strcpy (malloc(1), "");
   } else {
      scr = ptr[1];
      ptr[1] = '\0';
      bname = strcpy (malloc(strlen(base)+1), base);
      ptr[1] = scr;
   }
   ename = ptr + 1;
   for (ptr = ename; *ptr && *ptr != '/'; ++ptr);
   scr = *ptr;
   *ptr = '\0';
   tail = (scr) ? ptr + 1 : NULL;

   if ((dp = dopen (bname, NULL, &stat)) == NULL  ||  stat == 0) {
      free (bname);
      free (base);
      free (eav);
      puts ("Could not open directory");
      return (NULL);
   }
   while (dnext (dp, &name, &stat)) {
      if (compare_ok(ename, name)) {
         if (tail) {
            int alt_ac;
            char *search, **alt_av, **scrav;
            struct FileLock *lock;

            if (!stat)           /* expect more dirs, but this not a dir */
               continue;
            lock = CurrentDir (Clock = dp->lock);
            search = malloc(strlen(name)+strlen(tail)+2);
            strcpy (search, name);
            strcat (search, "/");
            strcat (search, tail);
            scrav = alt_av = expand (search, &alt_ac);
            CurrentDir (Clock = lock);
            if (scrav) {
               while (*scrav) {
                  if (eleft < 2) {
                     char **scrav = (char **)malloc(sizeof(char *) * (eac + 10));
                     movmem (eav, scrav, sizeof(char *) * (eac + 1));
                     free (eav);
                     eav = scrav;
                     eleft = 10;
                  }
                  eav[eac] = malloc(strlen(bname)+strlen(*scrav)+1);
                  strcpy(eav[eac], bname);
                  strcat(eav[eac], *scrav);
                  free (*scrav);
                  ++scrav;
                  --eleft, ++eac;
               }
               free (alt_av);
            }
         } else {
            if (eleft < 2) {
               char **scrav = (char **)malloc(sizeof(char *) * (eac + 10));
               movmem (eav, scrav, sizeof(char *) * (eac + 1));
               free (eav);
               eav = scrav;
               eleft = 10;
            }
            eav[eac] = malloc (strlen(bname)+strlen(name)+1);
            eav[eac] = strcpy(eav[eac], bname);
            strcat(eav[eac], name);
            --eleft, ++eac;
         }
      }
   }
   dclose (dp);
   *pac = eac;
   eav[eac] = NULL;
   free (bname);
   free (base);
   if (eac)
      return (eav);
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
   char *w = wild;
   char *n = name;
   char *back[MAXB][2];
   int  bi = 0;

   while (*n || *w) {
      switch (*w) {
      case '*':
         if (bi == MAXB) {
            puts ("Too many levels of '*'");
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
         if (*n != *w) {
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




