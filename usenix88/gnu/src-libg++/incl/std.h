// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of GNU CC.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU CC General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU CC, but only under the conditions described in the
GNU CC General Public License.   A copy of this license is
supposed to have been given to you along with GNU CC so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  
*/


#ifndef STDH
#define STDH


extern void     _exit(int);
extern void     abort();
extern int      abs(int);
extern int      access(const char*, int);
extern int      acct(const char*);
extern unsigned alarm(unsigned);
extern void*    alloca(int);
extern double   atof(const char*);
extern int      atoi(const char*);
extern long     atol(const char*);
extern int      bcmp(const void*, const void*, int);
extern int      bcopy(const void*, void*, int);
extern void*    brk(void*);
extern int      bzero(void*, int);
extern void*    calloc(unsigned, unsigned);
extern void     cfree(void*);
extern int      chdir(const char*);
extern int      chmod(const char*, int);
extern int      chown(const char*, int, int);
extern long     clock();
extern int      close(int);
extern int      creat(const char*, int);
extern char*    crypt(const char*, const char*);
extern char*    ctermid(char*);
extern char*    cuserid(char*);
extern double   drand48();
extern int      dup(int);
extern int      dup2(int, int);
extern int      dysize(int);
extern char*    ecvt(double, int, int*, int*);
extern char*    encrypt(char*, int);
extern char**   environ;
extern double   erand(short*);
extern int      errno;
extern int      execl(const char*, ...);
extern int      execle(const char*, ...);
extern int      execlp(const char*,  ...);
extern int      exect(const char*,  char**,  char**);
extern int      execv(const char*,  char**);
extern int      execve(const char*, const char**, const char**);
extern int      execvp(const char*,  char**);
extern void     exit(int);
extern int      fchmod(int, int);
extern int      fchown(int, int, int);
extern int      fcntl(int, int, int);
extern char*    fcvt(double, int, int*, int*);
extern int      ffs(int);
extern int      flock(int, int);
extern int      fork();
extern void     free(void*);
extern int      fsync(int);
extern long     ftok(const char*, int);
extern int      ftruncate(int, unsigned long);
extern char*    gcvt(double, int, char*);
extern char*    getcwd(char*, int);
extern int      getdomainname(char*, int);
extern int      getdtablesize();
extern int      getegid();
extern char*    getenv(const char*);
extern int      geteuid();
extern int      getgid();
extern int      getgroups(int, int*);
extern long     gethostid();
extern int      gethostname(char*, int);
extern char*    getlogin();
extern int      getopt(int, char**, const char*);
extern int      getpagesize();
extern char*    getpass(const char*);
extern int      getpgrp();
extern int      getpid();
extern int      getppid();
extern int      getpriority(int, int);
extern int      getpw(int, char*);
extern int      getuid();
extern char*    getwd(const char*);
extern char*    index(const char*, int);
extern char*    initstate(unsigned, char*, int);
extern int      ioctl(int, int, char*);
extern int      isatty(int);
extern long     jrand48(short*);
extern int      kill(int, int);
extern int      killpg(int, int);
extern void     lcong48(short*);
extern int      link(const char*, const char*);
extern int      lock(int, int, long);
extern long     lrand48();
extern long     lseek(int, long, int);
extern void*    malloc(unsigned);
extern void*    memalign(unsigned, unsigned);
extern void*    memccpy(void*, const void*, int, int);
extern void*    memchr(const void*, int, int);
extern int      memcmp(const void*, const void*, int);
extern void*    memcpy(void*, const void*, int);
extern void*    memset(void*, int, int);
extern int      mkdir(const char*, int);
extern int      mknod(const char*, int, int);
extern char*    mkstemp(char*);
extern char*    mktemp(char*);
extern long     mrand48();
extern int      nice(int);
extern long     nrand48(short*);
extern int      open(const char*, int, int);
extern char*    optarg;
extern int      opterr;
extern int      optind;
extern void     pause();
extern void     perror(const char*);
extern int      pipe (int);
extern void     profil(char*, int, int, int);
extern int      psignal(unsigned, char*);
extern int      ptrace(int, int, int, int);
extern int      putenv(const char*);
extern int      rand();
extern long     random();
extern int      read(int, void*, unsigned);
extern int      readlink(const char*, char*, int);
extern void*    realloc(void*, unsigned);
extern int      rename(const char*, const char*);
extern char*    rindex(const char*, int);
extern int      rmdir(const char*);               
extern void*    sbrk(int);              
extern short*   seed48(short*);
extern int      send(int, char*, int, int);
extern int      setgid(int);
extern int      sethostname(char*, int);
extern int      setkey(const char*);
extern int      setpgrp(int);
extern int      setpriority(int, int, int);
extern int      setregid(int, int);
extern int      setreuid(int, int);
extern char*    setstate(char*);
extern int      setuid(int);
extern int      sigblock(int);
extern int      siginterrupt(int, int);
extern int      sigpause(int);
extern int      sigsetmask(int);
extern unsigned sleep(unsigned);
extern int      srand(int);
extern void     srand48(long);
extern int      stime(long*);
extern char*    strcat(char*, const char*);
extern char*    strchr(const char*, int);
extern int      strcmp(const char*, const char*);
extern char*    strcpy(char*, const char*);
extern int      strcspn(const char*, const char*);
extern char*    strdup(const char*);
extern int      strlen(const char*);
extern char*    strncat(char*, const char*, int);
extern int      strncmp(const char*, const char*, int);
extern char*    strncpy(char*, const char*, int);
extern char*    strpbrk(const char*, const char*);
extern char*    strrchr(const char*, int);
extern int      strspn(const char*, const char*);
extern double   strtod(const char*, char**);
extern char*    strtok(char*, const char*);
extern long     strtol(const char*, char**, int);
extern void     swab(void*, void*, int);
extern int      symlink(const char*, const char*);
extern char*    sys_errlist[];
extern int      sys_nerr;                  
extern int      syscall(int, ...);
extern int      system(const char*);
extern char*    tempnam(char*, char*);
extern long     time(long*);
extern char*    timezone(int, int);
extern char*    tmpnam(char*);
extern int      truncate(const char*, unsigned long);
extern char*    ttyname(int);
extern int      ttyslot();
extern unsigned ualarm(unsigned, unsigned);
extern long     ulimit(int, long);
extern int      umask(int);
extern int      unlink(const char*);
extern unsigned usleep(unsigned);
extern int      vadvise(int);
extern void*    valloc(unsigned);
extern int      vfork();
extern int      vhangup();
extern int      wait(int*);
extern int      write(int, const void*, unsigned);


#endif                          // STDH
