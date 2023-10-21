
                   /* Datelib Declarations */

#if defined(lint) || defined(BSD) || defined(SYSV)
long   uxdate(), uxldate(), uxtime(), uxeaster();
int    yrday(),  weeknum(), daynum();
Bool   leap();
void   datex(),  ldatex(),  timex();
#else /* XENIX is presumed */
long   uxdate(int,int,int), uxldate(int,int,int), uxtime(int,int,int);
long   uxeaster(int);
int    yrday(int,int,int), weeknum(long), daynum(long);
Bool   leap(int);
void   datex(long,int*,int*,int*,int*,int*);
void   ldatex(long,int*,int*,int*,int*,int*);
void   timex(long,int*,int*,int*);
#endif
