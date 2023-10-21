/***************************************************************
 *
 *   Program  : snap
 *   By       : Brent Callaghan
 *              AT&T Information Systems, N.J.
 *              (201)576 3475
 *
 *   Date     : April 1985
 *
 *                  ***  PUBLIC DOMAIN  ***
 *   This program may be freely copied or modified in any way
 *   provided it is not offered for use as, or part of, any
 *   commercial software product and the above accreditation 
 *   remains intact.
 *
 *   Function : Displays a memory map & report, accounting
 *              for who has each click of 3b2 memory and for what.
 *              Every click of memory is coded by a letter:
 *
 *                K = Kernel
 *                T = Text
 *                D = Data
 *                U = User block
 *                S = Stack
 *                . = Available memory
 *                * = Collision (memory moved during snapshot)
 *
 *   Options  : -i n  interval of n secs between snapshots
 *              -n n  take n snapshots (default is 1)
 *              -l n  sort report by length of seg 
 *                    instead of address.
 *              -m    omit memory map
 *              -r    omit report
 *
 ***************************************************************
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/proc.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <nlist.h>
#include <sys/map.h>
#include <sys/var.h>
#include <sys/sysmacros.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/immu.h>

#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define gp_align(x,on)  (unsigned)((uint)(x) + on-1 - ((uint)(x) + on-1) % on)
#define CMAPSIZ   70     /* size of coremap    */
#define MPROCS    60     /* size of proc table */
#define TABSZ     300    /* size of mem table  */
#define NBYTES    512    /* space for proc names */
#define MAPSZ     1024   /* size of memmap     */
#define VUSER     0xF000000

struct nlist nl[] = {
   {"proc"},        /* process table */
   {"v"},           /* system variables */
   {"coremap"},     /* available core map */
   {"shminfo"},     /* shared memory info */
   {"shmem"},       /* shared memory headers */
   {"_ksde"},       /* kernel sde table */
   {""}} ;

#define NLPROC  nl[0].n_value
#define NLV     nl[1].n_value
#define NLCORE  nl[2].n_value
#define NLSHMI  nl[3].n_value
#define NLSHM   nl[4].n_value
#define NLKSDE  nl[5].n_value
#define NLEND   6

struct var v ;
struct map coremap [CMAPSIZ], *bp ; /* map of avail core */
struct proc mproc ;      /* process table entry */
struct user u     ;      /* user block          */
struct shmid_ds mds ;    /* shared memory data structure */
struct shminfo shminfo ; /* shared memory information structure */
sde_t ksde ;             /* kernel segment descriptor entry */
int
   mapflag = 1,          /* print map */
   repflag = 1,          /* print report */
   lenflag ;             /* sort report by seg length */
int
   kmem,                 /* Kernel memory descriptor */
   mem ;                 /* User   memory descriptor */

char memmap [MAPSZ] ;    /* the memory map */
struct t_entry {
   char *pname ;  /* name of process  */
   char mtype  ;  /* type of memory   */
   int  maddr  ;  /* mem addr (click) */
   int  mlen   ;  /* length (clicks)  */
   } table[TABSZ], *tablep ;

char *stralloc (bytes)
   int bytes ;
   /* Allocate some bytes for a string from an internal array */
   {
   static char strings[NBYTES], *p, *strp = strings ;

   if (bytes < 0) {    /* Init str space */
      strp = strings ;
      return(NULL) ;
      }

   if (strp+(bytes+1) > strings + NBYTES) {
      fprintf(stderr, "No space for proc names\n") ;
      exit(1) ;
      }
   p = strp ;
   strp += bytes + 1 ;
   return(p) ;
   }

mpinit ()
   /* Initialize map with spaces */
   {
   register int i ;

   for (i = 0 ; i < MAPSZ ; i++) 
      memmap[i] = ' ' ;
   }

map (type, addr, len)
   char type ; int addr, len ;
   /* Colour in map with type at addr for len clicks */
   /* Substitute a "*" if a collision occurs.        */
   {
   register int i ;
   register char *p ;

   for (p = &memmap[addr], i = 0 ; i < len && addr + i < MAPSZ ; p++, i++)
      *p = (*p==' ' || *p==type)? type : '*' ;
   }

mapprint ()
   {
   register int i ;

   printf("\nMemory Map\n      ") ;
   for (i = 0 ; i < 60 ; i+=10)
      printf("|%02d_______", i) ;
   printf("|60_\n") ;

   /* Print the map */

   for (i = 0 ; i < MAPSZ ; i += 64) {
      printf("%04d [", i) ;
      fwrite(&memmap[i], MIN(64, MAPSZ-i), 1, stdout) ;
      printf("]\n") ;
      }
   }

table_add (name, type, addr, len)
   char *name, type ; int addr, len ;
   {
   tablep->pname = name ;
   tablep->mtype = type ;
   tablep->maddr = addr ;
   tablep->mlen  = len ;
   if (++tablep >= table+TABSZ) {
      fprintf(stderr, "Map table overflow\n") ;
      exit(1) ;
      }
   map(type, addr, len) ;
   }

int m_compare (a,b)
   struct t_entry *a, *b ;
   {
   return(a->maddr <= b->maddr ? -1 : 1) ;
   }

int l_compare (a,b)
   struct t_entry *a, *b ;
   {
   if (a->mlen == b->mlen)  /* keep shared text together */
      return(a->maddr <= b->maddr ? -1 : 1) ;
   return(a->mlen >= b->mlen ? -1 : 1) ;
   }

table_print ()
   {
   register struct t_entry *tp ;
   register int i ;
   int l, count = 0 ;

   qsort(table, tablep-table, sizeof(struct t_entry),
         lenflag ? l_compare : m_compare) ;

   printf("\naddr size") ;
   for (tp = table ; tp < tablep ; tp++) {
      if (tp > table && tp->maddr == (tp-1)->maddr)
         count++ ;
      else {
         if (count > 1)
            printf(" (%d sharing)", count) ;
         count = 1 ;
         printf("\n%4d %4d ", tp->maddr, tp->mlen) ;
         for (l = 0 ; l < tp->mlen ; l++) {
            putchar(tp->mtype) ;
            if ((l+1) % 64 == 0)
               printf("\n          ") ;
            }
         printf("  %s", tp->pname) ;
         }
      }
   putchar('\n') ;
   }

proc_sdt (name, type, first, segs)
   char *name ; int type, first, segs ;
   /* Get info about memory of type 'type' from 'segs' */
   /* sde_t's starting at 'first' in table.            */
   {
   register int j ;
   int addr, len ;

   for (j = first ; j < first + segs ; j++) {
      addr = btoc (u.u_sdt.seg[j].wd2.address) & 0x3FFF ;
      len  = motoc(u.u_sdt.seg[j].maxoff) ;
      table_add(name, type, addr, len) ;
      }
   }

snapshot ()
   {
   register int i, j ;
   long addr ;
   int dsize ;
   char *namep ;
   long tod, time() ;

   mpinit() ;        /* Init map */
   stralloc(-1) ;    /* Init string space */
   tablep = table ;


   /* get core map */

   lseek(kmem, (long)NLCORE, 0) ;
   read (kmem, coremap, sizeof(coremap)) ;

   /* find process table size */

   lseek(kmem, (long)NLV, 0) ;
   read (kmem, (char *)&v, sizeof(v)) ;

   /* locate process table */

   lseek(kmem, (long)NLPROC, 0) ;

   /* read through process table */

   for (i = 0 ; i < v.v_proc ; i++) {
      read(kmem, (char *)&mproc, sizeof(mproc)) ;
      if (mproc.p_stat == 0)
         continue ;
      if (!(mproc.p_flag & SLOAD)) /* in core ? */
         continue ;

      /* get user block */

      addr = ctob((int)mproc.p_addr) & (~VUSER) ;
      lseek(mem, addr, 0) ;
      if (read(mem, (char *)&u, sizeof(u)) != sizeof(u)) {
         fprintf(stderr, "Couldn't read user block\n") ;
         exit(1) ;
         }

      if (mproc.p_pid == 0)
         namep = "(swapper)" ;
      else
         namep = (char *)strcpy(stralloc(strlen(u.u_comm)), u.u_comm) ;

      proc_sdt(namep, 'T', TSEG, ctos(mproc.p_tsize)) ;

      dsize = mproc.p_size - (mproc.p_textp ? 0 : mproc.p_tsize) 
                           -  mproc.p_ssize - USIZE ;
      proc_sdt(namep, 'D', MAX(j,DSEG), ctos(dsize)) ;

      proc_sdt(namep, 'S', STKSEG, ctos(mproc.p_ssize)) ;

      proc_sdt(namep, 'U', USEG  , 1) ;
      }

   /* get coremap data */

   for (bp = mapstart(coremap) ; bp->m_size ; bp++)
      table_add("", '.', bp->m_addr & 0x3FFF, bp->m_size) ;

   /* get shared memory info */
   
   if (NLSHMI != 0) {
      lseek(kmem, (long)NLSHMI, 0) ;
      read (kmem, &shminfo, sizeof(shminfo)) ;
      
      NLKSDE = gp_align(NLKSDE, 8) ;
      lseek(kmem,(long)NLKSDE, 0) ;
      for (i = 0 ; i < shminfo.shmmni ; i++) {
         read (kmem, &ksde, sizeof(ksde)) ;
         if (!(isvalid(&ksde) && ispresent(&ksde)))
            continue ;
         table_add("(Shared Memory)", 'X',
            (btoc (ksde.wd2.address) & 0x3FFF) - 1,
            motoc(ksde.maxoff)) ;
         }
      }

   /* Fill in kernel space (leading space in map) */

   table_add("(Kernel)", 'K', 0, strspn(memmap, " ")) ;

   /* Print the maps */

   tod = time((long *)0) ;
   printf("%s", ctime(&tod)) ;

   if (mapflag)
      mapprint() ;

   if (repflag)
      table_print() ;
   }

main (argc, argv)
   int argc ; char *argv[] ;
   {
   int c ;
   extern int optind ;
   extern char *optarg ;
   int badflag = 0 ;

   int
      interval = 0,   /* Interval (sec) between samples */
      samples  = 1 ;  /* Number of samples */

   while ((c = getopt(argc, argv, "lmri:n:")) != EOF)
      switch(c) {
         case 'l':        /* sort report by length of memory */
            lenflag = 1 ;
            break ;

         case 'm':        /* don't print memory map */
            mapflag = 0 ;
            break ;

         case 'r':        /* don't print report */
            repflag = 0 ;
            break ;

         case 'n':        /* number of samples */
            samples = atoi(optarg) ;
            break ;

         case 'i':        /* interval between samples (sec) */
            interval = atoi(optarg) ;
            break ;

         case '?':
            badflag = 1 ;
            break ;
         }
   if (badflag) {
      fprintf(stderr, "Usage: %s [-l] [-m] [-r] [-i n] [-n n]\n", argv[0]) ;
      exit(1) ;
      }

   if (samples > 1 && interval == 0)
      interval = 5 ;

   if ((kmem = open("/dev/kmem", 0)) < 0) {
      perror("Couldn't open /dev/kmem ") ;
      exit(1) ;
      }

   if ((mem = open("/dev/mem", 0)) < 0) {
      perror("Couldn't open /dev/mem ") ;
      exit(1) ;
      }

   nlist("/unix", nl) ;
   if (NLPROC == 0) {
      fprintf(stderr, "No namelist\n") ;
      exit(1) ;
      }

   if (samples > 1)
      fprintf(stderr, "Taking %d snapshots at %d sec intervals\n", samples, interval) ;

   while (samples--) {
      snapshot() ;
      sleep(interval) ;
      }
   }
