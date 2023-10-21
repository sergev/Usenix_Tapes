/***************************************************************
 *
 *   Program  : memmon
 *   By       : Brent Callaghan
 *              AT&T Information Systems, Lincroft N.J.
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
 *   Function : Uses curses to display a map of 3b2 memory.
 *              Every click of memory is coded by a letter:
 *
 *                K = Kernel
 *                T = Text
 *                D = Data
 *                U = User block
 *                S = Stack
 *                . = Available memory
 *
 *              Memory changed since last snapshot is
 *              highlighted.
 *
 *              Names of processes moved into or out of
 *              memory appear at the bottom of the screen.
 *
 *              memmon takes a snapshot every 5 sec unless
 *              you give it an interval as an invocation 
 *              parameter.
 *                          e.g.  memmon 1
 *
 *              Kill memmon with a terminal interrupt
 *              (del, rubout or break)
 *
 ***************************************************************
 */
#include <stdio.h>
#include <curses.h>
#include <ctype.h>
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

struct PINFO {
   char
      cflg,       /* incore flag  */
      nflg,       /* new flag     */
      name[15] ;  /* name of proc */
   int
      pid ;       /* process id */
   } pinfo [MPROCS] ;

struct var v ;
struct map coremap [CMAPSIZ], *bp ; /* map of avail core */
struct proc mproc ;      /* process table entry */
struct user u     ;      /* user block          */
struct shmid_ds mds ;    /* shared memory data structure */
struct shminfo shminfo ; /* shared memory information structure */
sde_t ksde ;             /* kernel segment descriptor entry */
int
   kmem,                 /* Kernel memory descriptor */
   mem ;                 /* User memory descriptor   */

int
   highlight = 0,        /* curses highlight code */
   procs,                /* Active processes */
   avail ;               /* available memory */

char memmap [MAPSZ] ;    /* the memory map */

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
   {
   register int i ;
   register char *p ;

   for (p = &memmap[addr], i = 0 ; i < len && addr + i < MAPSZ ; p++, i++)
      *p = type | (*p != type ? highlight : 0) ;
   }

clearproc ()
   /* clear flags in my process table */
   {
   struct PINFO *pp ;

   for (pp = pinfo ; pp < pinfo + MPROCS ; pp++) {
      if (pp->cflg == 0) {
         pp->nflg = 0 ;
         pp->name[0] = 0 ;
         pp->pid = 0 ;
         }
      pp->cflg = 0 ;
      pp->nflg = 0 ;
      }
   }

int checkproc (pname, ppid)
   char *pname ; int ppid ;
   /* if process in table set a flag */
   /* otherwise add it to the table  */
   {
   struct PINFO *pp, *ppfree = NULL ;

   for (pp = pinfo ; pp < pinfo + MPROCS ; pp++) {
      if (ppid == pp->pid) {
         pp->cflg = 1 ;
         return 0 ;
         }
      if (pp->pid == 0 && ppfree == NULL)
         ppfree = pp ; /* remember empty slot */
      }
   ppfree->cflg = 1 ;
   ppfree->nflg = 1 ;
   strcpy(ppfree->name, pname) ;
   ppfree->pid = ppid ;
   return 1 ;
   }

deltaproc ()
   /* Print changes to the process table */
   {
   struct PINFO *pp ;

   printw("     Procs In : ") ; 
   for (pp = pinfo ; pp < pinfo + MPROCS ; pp++)
      if (pp->nflg)
         printw("%s ", pp->name) ;

   printw("\n     Procs Out: ") ;
   for (pp = pinfo ; pp < pinfo + MPROCS ; pp++)
      if (pp->cflg == 0 && pp->pid > 0)
         printw("%s ", pp->name) ;
   }

mapdisp ()
   /* display the memory map */
   {
   register int i, j, c ;
   static int oprocs, oavail ;
   long tod ;

   move(0,0) ;
   tod = time((long *)0) ;
   printw("     3b2 Memory Map         %s\n      ", ctime(&tod)) ;
   for (i = 0 ; i < 60 ; i+=10)
      printw("|%02d_______", i) ;
   printw("|60_\n") ;

   /* Print the map */

   for (i = 0 ; i < MAPSZ ; i += 64) {
      printw("%04d [", i) ;
      for (j=0 ; j < MIN(64, MAPSZ-i) ; j++) {
         if ((c = memmap[i+j]) & A_STANDOUT)
            memmap[i+j] &= (~A_STANDOUT) ;

         addch(c) ;
         }
      printw("]\n") ;
      }

   printw("\n     Incore Procs %2d ", procs) ;
   if (procs != oprocs && oprocs)
      printw("(%c%2d)", procs>oprocs?'+':'-', abs(procs-oprocs)) ;
   else
      printw("     ") ;
   oprocs = procs ;

   printw("    Avail Mem %3d ", avail) ;
   if (avail != oavail && oavail)
      printw("(%c%3d)\n", avail>oavail?'+':'-', abs(avail-oavail)) ;
   else
      printw("\n") ;
   oavail = avail ;

   if (highlight)
      deltaproc() ;

   clrtobot() ;
   refresh() ;
   }

proc_sdt (type, first, segs)
   int type, first, segs ;
   /* Get info about memory of type 'type' from 'segs' */
   /* sde_t's starting at 'first' in table.            */
   {
   register int j ;

   for (j = first ; j < first + segs ; j++)
      map(type,
         btoc (u.u_sdt.seg[j].wd2.address) & 0x3FFF,
         motoc(u.u_sdt.seg[j].maxoff)) ;
   }

snapshot ()
   {
   register int i, j ;
   long addr ;
   int dsize, new ;

   /* get core map */

   lseek(kmem, (long)NLCORE, 0) ;
   read (kmem, coremap, sizeof(coremap)) ;

   /* find process table size */

   lseek(kmem, (long)NLV, 0) ;
   read (kmem, (char *)&v, sizeof(v)) ;

   /* locate process table */

   lseek(kmem, (long)NLPROC, 0) ;

   /* read through process table */

   clearproc() ;
   procs = 0 ;
   for (i = 0 ; i < v.v_proc ; i++) {
      read(kmem, (char *)&mproc, sizeof(mproc)) ;
      if (mproc.p_stat == 0)
         continue ;
      if (!(mproc.p_flag & SLOAD)) /* in core ? */
         continue ;
      procs++ ;

      /* get user block */

      addr = ctob((int)mproc.p_addr) & (~VUSER) ;
      lseek(mem, addr, 0) ;
      if (read(mem, (char *)&u, sizeof(u)) != sizeof(u)) {
         fprintf(stderr, "Couldn't read user block\n") ;
         exit(1) ;
         }

      if (mproc.p_pid != 0 && checkproc(u.u_comm, (int)mproc.p_pid))
         new = highlight ;
      else
         new = 0 ;

      proc_sdt('T'|new, TSEG, ctos(mproc.p_tsize)) ;

      dsize = mproc.p_size - (mproc.p_textp ? 0 : mproc.p_tsize) 
                           -  mproc.p_ssize - USIZE ;
      proc_sdt('D'|new, MAX(j,DSEG), ctos(dsize)) ;

      proc_sdt('S'|new, STKSEG, ctos(mproc.p_ssize)) ;

      proc_sdt('U'|new, USEG  , 1) ;
      }

   /* get coremap data */

   avail = 0 ;
   for (bp = mapstart(coremap) ; bp->m_size ; bp++) {
      avail += bp->m_size ;
      map('.', bp->m_addr & 0x3FFF, bp->m_size) ;
      }

   /* get shared memory info */
   
   if (NLSHMI != 0) {
      lseek(kmem, (long)NLSHMI, 0) ;
      read (kmem, &shminfo, sizeof(shminfo)) ;
      
      NLKSDE = gp_align(NLKSDE, 8) ;
      lseek(kmem,(long)NLKSDE, 0) ;
      for (i = 0 ; i < shminfo.shmmni ; i++) {
         read (kmem, &ksde, sizeof(ksde)) ;
         if (isvalid(&ksde) && ispresent(&ksde))
            map('X',
               (btoc (ksde.wd2.address) & 0x3FFF) - 1,
               motoc(ksde.maxoff)) ;
         }
      }

   /* Fill in kernel space (leading space in map) */

   if (highlight == 0)
      map('K', 0, strspn(memmap, " ")) ;

   /* Display the map */

   mapdisp() ;

   highlight = A_STANDOUT ;  /* set highlighting after 1st snapshot */
   }

quit()
   /* clean up curses stuff */
   {
   clear() ;
   refresh() ;
   endwin() ;
   exit(0) ;
   }

main (argc, argv)
   int argc ; char *argv[] ;
   {
   int interval = 5 ;  /* default 5 sec interval */

   if (argc > 1 && isdigit(*argv[1]))
      interval = atoi(argv[1]) ;

   if ((kmem = open("/dev/kmem", 0)) < 0) {
      fprintf(stderr, "Couldn't open /dev/kmem\n") ;
      exit(1) ;
      }

   if ((mem = open("/dev/mem", 0)) < 0) {
      fprintf(stderr, "Couldn't open /dev/mem\n") ;
      exit(1) ;
      }

   nlist("/unix", nl) ;
   if (NLPROC == 0) {
      fprintf(stderr, "No namelist\n") ;
      exit(1) ;
      }

   mpinit() ;  /* initialize memory map */

   signal(SIGINT , quit) ; /* set up             */
   signal(SIGQUIT, quit) ; /*   signals for      */
   signal(SIGTERM, quit) ; /*     graceful death */

   initscr() ;
   crmode() ;
   nonl() ;
   clear() ;

   for (;;) {
      snapshot() ;
      sleep(interval) ;
      }

   }
