
/*  dump.c  (10/83)  Debug style dump of a file from any starting position.
*/
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#define FAIL        1
#define SUCCESS     0
#define TRUE      	(1)
#define FALSE       0
#define FOREVER     for (;;)
#define PAUSE       if (getch()=='\0') getch();
#define STDIN       0
#define STDOUT      1
#define STDERR      2
char *dumpusage[] =
{
  "Usage: dump filespec [block [page]] | [segment:[offset]] [count]\r\n",
  "Where a block is 64K bytes and a page is 256 bytes.\r\n",
  "Segment:offset are standard 8086 notation in hexadecimal.\r\n",
  "Count is the number of bytes to dump in decimal.\r\n",
  NULL
};
char *index();		/* suppress warning about conversion to int */
#define BUFSIZE 512
typedef unsigned    ushort;

static int _fmode = 0x8000;            /* Lattice 'c' - forces binary I/O */
extern short errno;             /* DOS 2.0 error number */
long lseek();

static char   buffer[BUFSIZE];         /* input buffer */
static ushort block = 0;               /* block number ( 64k bytes/block ) */
static ushort page = 0;                /* page number ( 256 bytes/page ) */
static ushort segment = 0;
static ushort offset = 0;
static long   filpos = 0;              /* beginning file position */
static long   count = 0x7FFFFFFFL;     /* number of bytes to dump */

void ohw(),ohb(),onib(),abort();

static jmp_buf env;
void (*signal())();
static void (*oldsig)();
static onintr()
{
	signal(SIGINT,SIG_IGN);	/* disable interrupts from kbd */
	longjmp(env,-1);
}

#ifdef MAIN
main
#else
dump
#endif
(argc,argv)                 /* DUMP ENTRY */
int  argc;
char *argv[];
{
   char c;
   ushort i, numin, tot, file, cfrom;
   char *flag = 0;
   char *index();
   oldsig=signal(SIGINT,onintr);
   if (-1 == setjmp(env))
   {
   		close(file);
		write(2,"Interrupted\r\n",13);
		signal(SIGINT,oldsig);
		return -1;
   }
   if (argc < 2)
   {
   		char **u = (char **) dumpusage;
		while(*u)
		{
			write (2,*u,strlen(*u));
			++u;
		}
		return -1;
   }
   if ((file = open( argv[1], 0 )) == -1)
      abort( "cannot open", argv[1], errno );

   if (argc > 2) {
      if ((flag = index( argv[2], ':' )) != NULL) {
         i = stch_i( argv[2], &segment );
         stch_i( argv[2]+i+1, &offset );
      }
      if (sscanf( argv[2], "%d", &block ) != 1)
         abort( "invalid block", argv[2], 0 );
   }
   if (argc > 3)
      if (sscanf( argv[3], "%d", &page ) != 1)
         abort( "invalid page", argv[3], 0 );

   if ( flag ) {
      filpos = (long)segment*16L + (long)offset;
      tot = offset;
   }
   else {
      filpos = (block * 65536L) + (page * 256);
      tot = page * 256;
      segment = block * 4096;
   }

   if (lseek( file, filpos, 0 ) == -1L)
      abort( "positioning to", argv[2], errno );

   if (argc > 4)
      if (sscanf( argv[4], "%ld", &count ) != 1)
         abort( "invalid count", argv[4], 0 );


   do {                                    /* read & dump BUFSIZE bytes */
      numin = read( file, buffer, BUFSIZE );
      if (numin == -1)
         abort( "cannot read", argv[1], errno );
      cfrom=0;
      while (cfrom < numin) {

         ohw(segment);                     /* print offset in hex */
         putchar(':');
         ohw(cfrom+tot);
         putchar(' ');

         for (i=0; i < 16; i++) {          /* print 16 bytes in hex */
            putchar(' ');
            ohb(buffer[cfrom++]);
         }

         putchar(' '); putchar(' '); putchar(' ');

         cfrom -= 16;
         for (i=0; i < 16; i++) {          /* print 16 bytes in ASCII */
            c = buffer[cfrom] & 0x7f;
            if ( isprint(c) )              /* if printable character */
               putchar(c);
            else
               putchar('.');               /* else print period */
            cfrom++;
         }

         putchar('\r'); putchar('\n');     /* print CR/LF */

         if ((count -= 16) <= 0)             /* is count exhausted? */
            exit(0);
      }                                    /* end of while */
      tot += numin;
      if ( tot == 0 )
         segment += 4096;
   }                                       /* end of do */
   while (numin == BUFSIZE);
	signal(SIGINT,oldsig);					/* restore signal */
	return 0;
}                                          /* end of main */

static void ohw(wrd)                        /*      print a word in hex     */
ushort wrd;
{
   ohb( wrd>>8 );
   ohb( wrd );
}

static void ohb(byt)                        /*      print a byte in hex     */
char byt;
{
   onib( byt>>4 );
   onib( byt );
}

static void onib(nib)            /*      print a nibble as a hex character   */
char nib;
{
   nib &= 15;
   putchar((nib >= 10) ? nib-10+'A': nib+'0');
}

static void abort( msg1 ,msg2 ,errno)     /*  print error msg1, msg2, and nbr */
char *msg1,*msg2;                  /*   Does not close files.  */
short errno;
{
   char stemp[10];

   write( STDERR, "ERR: ", 5 );
   if (msg1)
      write( STDERR, msg1, strlen(msg1) );
   if (msg2)
      write( STDERR, " ", 1 );
      write( STDERR, msg2, strlen(msg2) );
   if (errno)   {
      sprintf( stemp," #%d", errno );
      write( STDERR, stemp, strlen(stemp) );
   }
   write( STDERR, "\r\n", 2 );
   longjmp(env,-1);
}
/** END DUMP **/

#define BDOS_IN   7     /* input function for "getch" */
#define BDOS_OUT  6     /* output function for "putch" */
#define BDOS_CKS  11    /* check keyboard status for "kbhit" */
#define BDOS_BKI  10    /* buffered keyboardd input for "cgets" */
#define BDOS_PRT  9     /* print string for "cputs" */

static char pushback;   /* character save for "ungetch" */

static getch()
{
int c;

if (pushback != '\0')
   {                    /* character was pushed back */
   c = pushback;
   pushback = '\0';
   return(c);
   }
return(bdos(BDOS_IN, 0xFF) & 127);
}
static putch(c)
char c;
{
bdos(BDOS_OUT, c&127);
return(c);
}
static ungetch(c)
char c;
{

if (pushback != '\0') return(-1);
pushback = c;
return(c);
}
static char *cgets(s)
char *s;
{
char *p;

if (*s == 0) *s = 250;          /* do not allow zero byte count */
bdos(BDOS_BKI, s);
p = s+2;
p[s[1]] = '\0';                 /* set terminating byte */
return(p);
}
static cputs(s)
char *s;
{
char *p;

for (p = s; *p != '\0'; p++) ;          /* find string terminator */
*p = '$';
bdos(BDOS_PRT, s);
*p = '\0';
return;
}


static int stch_i(p,r)
	char *p;
	int	*r;
{
	int count;
	int acc;
	int hdtoi();
	count = 0;
	*r = 0;
	while (-1 != (acc = hdtoi(*p++)))
	{
		++count;
		*r = (*r << 4) | acc;
	}
	return count;
}

static hdtoi(c)
	char c;
{
	c = toupper(c);
	if (!isxdigit(c)) 
		return -1;
	if (isdigit(c))
		return (c - '0');
	return (c - 'A' + 10);
}
