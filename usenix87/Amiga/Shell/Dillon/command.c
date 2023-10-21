
/*
 * COMMAND.C
 *
 * Matthew Dillon, 24 Feb 1986
 *
 * Most of the commands are in here (the variable commands are in set.c)
 *
 */

#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <lattice/fcntl.h>
#include "shell.h"

extern struct FileLock  *CreateDir(), *CurrentDir(), *ParentDir();
extern struct FileLock  *Lock(), *DupLock();
extern struct FileLock  *cdgo();
extern char *AllocMem();

extern long disp_entry();

struct FileLock *Clock;

do_run(str)
char *str;
{
   register int i, len;
   register char *build, *ptr;
   char from[128];
   char to[128];

   from[127] = to[127] = '\0';
   strcpy (from, "<*");
   strcpy (to, ">*");
   for (i = 1, len = 0; i < ac; ++i) {
      ptr = (*av[i] == '>') ? to : ((*av[i] == '<') ? from : NULL);
      if (ptr) {
         strncpy (ptr, av[i], 126);
         if (!ptr[1]) {
            av[i] = "";
            if (av[++i] == NULL)
               goto rerr;
            strncat (ptr, av[i], 125);
         }
         av[i] = "";
         break;
      }
      len += strlen(av[i]) + 1;
   }
   len += strlen(av[0]) + strlen(from) + strlen(to) + 4;
   build = malloc(len);
   strcpy (build, av[0]);
   strcat (build, " ");
   strcat (build, from);
   strcat (build, " ");
   strcat (build, to);
   for (i = 1; i < ac; ++i) {
      if (*av[i]) {
         strcat (build, " ");
         strcat (build, av[i]);
      }
   }
   i = Execute (build, 0, 0);
   free (build);
   return ((i) ? 1 : -1);
rerr:
   puts ("redirection syntax error");
   return (-1);
}

do_number()
{
   return (1);
}

do_cat()
{
   FILE *fi;
   int i;
   char buf[256];

   for (i = 1; i < ac; ++i) {
      if (fi = fopen (av[i], "r")) {
         while (fgets (buf, 256, fi)) {
            fputs (buf, stdout);
            fflush (stdout);
         }
         fclose (fi);
      } else {
         perror (av[i]);
      }
   }
}

do_dir(garbage, com)
char *garbage;
{
   struct DPTR          *dp;
   struct InfoData      *info;
   char *name;
   int i, stat;
   long total = 0;

   if (ac == 1) {
      ++ac;
      av[1] = "";
   }
   for (i = 1; i < ac; ++i) {
      if ((dp = dopen (av[i], &stat)) == NULL)
         continue;
      if (com < 0) {
         info = (struct InfoData *)AllocMem(sizeof(struct InfoData), MEMF_PUBLIC);
         if (Info (dp->lock, info)) {
            int bpb = info->id_BytesPerBlock;
            printf ("Unit:%2d  Errs:%3d  Bytes: %-7d Free: %-7d\n",
                  info->id_UnitNumber,
                  info->id_NumSoftErrors,
                  bpb * info->id_NumBlocks,
                  bpb * (info->id_NumBlocks - info->id_NumBlocksUsed));
         } else {
            perror (av[i]);
         }
         FreeMem (info, sizeof(*info));
      } else {
         if (stat) {
            while (dnext (dp, &name, &stat))
               total += disp_entry (dp->fib);
         } else {
            total += disp_entry(dp->fib);
         }

      }
      dclose (dp);
   }
   printf ("TOTAL: %ld\n", total);
   return (1);
}


long
disp_entry(fib)
register struct FileInfoBlock *fib;
{
   char str[5];
   register char *dirstr;

   str[4] = '\0';
   str[0] = (fib->fib_Protection & FIBF_READ) ? '-' : 'r';
   str[1] = (fib->fib_Protection & FIBF_WRITE) ? '-' : 'w';
   str[2] = (fib->fib_Protection & FIBF_EXECUTE) ? '-' : 'x';
   str[3] = (fib->fib_Protection & FIBF_DELETE) ? '-' : 'd';
   dirstr = (fib->fib_DirEntryType < 0) ? "   " : "DIR";

   printf ("%s %6ld %s  %s\n", str, (long)fib->fib_Size, dirstr, fib->fib_FileName);
   return ((long)fib->fib_Size);
}


do_quit()
{
   exit (1);
}

do_echo(str)
char *str;
{
   if (strcmp (av[1], "-n") == 0) {
      fputs (next_word(next_word(str)), stdout);
   } else {
      puts (next_word(str));
   }
   fflush (stdout);
   return (1);
}


do_source(str)
char *str;
{
   register FILE *fi;
   char buf[256];

   fi = fopen (next_word(str), "r");
   if (fi == NULL) {
      printf ("Cannot open %s\n", str);
      return(-1);
   }
   ++H_stack;
   while (fgets (buf, 256, fi) != NULL) {
      buf[strlen(buf) - 1] = '\0';
      exec_command (buf);
   }
   --H_stack;
   fclose (fi);
   return (1);
}

/*
 * cd.  Additionally, allow '..' to get you back one directory in the path.
 */

do_cd()
{
   static char root[32];
   register struct FileLock *lock;
   register char *str, *ptr;
   register int notdone;

   if (ac != 2) {
      puts (root);
      Clock = cdgo (root, NULL);
      return (1);
   }
   str = av[1];
   while (*str == '/') {
      ++str;
      if (Clock)
         Clock = cdgo (NULL, Clock);
   }
   notdone = 1;
   while (notdone) {
      ptr = str;
      while (*str && *str != '/' && *str != ':')
         ++str;
      notdone = *str;
      *str++ = '\0';
      if (ptr == str - 1)
         continue;
      if (notdone == ':') {
         *(str-1) = notdone;
         notdone = *str;
         *str = '\0';
         strcpy (root, ptr);
         Clock = cdgo (root, NULL);
         *str = notdone;
      } else
      if (strcmp (ptr, "..") == 0) {
         if (Clock)
            Clock = cdgo (NULL, Clock);
      } else {
         if ((lock = cdgo (ptr, NULL)) == NULL)
            puts ("not found");
         else
            Clock = lock;
      }
   }
   return (1);
}


struct FileLock *
cdgo(ptr, lock)
char *ptr;
struct FileLock *lock;
{
   struct FileLock *newlock, *oldlock;

   if (lock)
      newlock = ParentDir (lock);
   else
      newlock = Lock (ptr, ACCESS_READ);
   if (newlock) {
      if (oldlock = CurrentDir (newlock))
         UnLock (oldlock);
   }
   return (newlock);
}


do_mkdir()
{
   register int i;
   register struct FileLock *lock;

   for (i = 1; i < ac; ++i) {
      if (lock = CreateDir (av[i])) {
         UnLock (lock);
         continue;
      }
      perror (av[i]);
   }
   return (1);
}

do_mv()
{
   if (ac != 3) {
      puts ("Rename required 2 arguments");
      return (-1);
   }
   if (Rename (av[1], av[2]))
      return (1);
   perror (NULL);
   return (-1);
}

do_rm()
{
   register int i;

   for (i = 1; i < ac; ++i) {
      if (!DeleteFile(av[i]))
         perror (av[i]);
   }
   return (1);
}


do_history()
{
   register struct HIST *hist;
   register int i = H_tail_base;

   for (hist = H_tail; hist; hist = hist->prev) {
      printf ("%3d %s\n", i, hist->line);
      ++i;
   }
   return (1);
}

do_mem()
{
   long cfree, ffree;
   extern long AvailMem();

   Forbid();
   cfree = AvailMem (MEMF_CHIP);
   ffree = AvailMem (MEMF_FAST);
   Permit();

   if (ffree)
      printf ("FAST memory:%10ld\n", ffree);
   printf ("CHIP memory:%10ld\n", cfree);
   printf ("Total -----:%5ld K\n", (ffree + cfree)/1024L);
}

/*
 * foreach var_name  ( str str str str... str ) commands
 * spacing is important (unfortunetly)
 *
 * ac=0    1 2 3 4 5 6 7
 * foreach i ( a b c ) echo $i
 * foreach i ( *.c )   "echo -n "file ->";echo $i"
 */

do_foreach()
{
   register int i, cstart, cend, old;
   register char *cstr, *vname, *ptr;

   if (ac < 2) {
      puts ("form: foreach i ( str str str.. ) command");
      return (-1);
   }
   cstart = i = (*av[2] == '(') ? 3 : 2;
   while (i < ac) {
      if (*av[i] == ')')
         break;
      ++i;
   }
   if (i == ac) {
      puts ("')' expected");
      return (-1);
   }
   ++H_stack;
   cend = i;
   vname = strcpy(malloc(strlen(av[1])+1), av[1]);
   cstr = compile_av (av, cend + 1, ac);
   for (i = cstart; i < cend; ++i) {
again:
      ptr = av[i];
      while (*ptr && *ptr != ' ')
         ++ptr;
      old = *ptr;
      *ptr = '\0';
      set_var (LEVEL_SET, vname, av[i]);
      exec_command (cstr);
      if (old) {
         while (*++ptr && (*ptr == ' ' || *ptr == 9));
         av[i] = ptr;
         if (*ptr)
            goto again;
      }
   }
   --H_stack;
   free (cstr);
   unset_var (LEVEL_SET, vname);
   free (vname);
   return (1);
}



