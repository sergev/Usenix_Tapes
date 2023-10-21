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

/*
 *  Please check the following before installing this file:
 *
 *  If you are running AT&T System V or similar systems in which
 *  there is no _bufsiz field in _iobuf, then comment out the
 *  #define HAVE_BUFSIZ line below.
 *
 *  Check whether your libc.a sprintf function returns
 *  an int (as do most) versus a char* (BSD), and (un)comment
 *  the corresponding SPRINTF_RETURNS_INT line.
 *
 *  Check the value of BUFSIZ against the one in your /usr/include/stdio.h.
 *
 *  Carefully check the fields and order of _iobuf declaration against
 *  the one in your /usr/include/stdio.h. Xenix-based systems
 *  may need some re-ordering of _iobuf. fields.
 *
 *  Note that some _IOXXX #defines may not be present in your 
 *  /usr/include/stdio.h. This is ok, so long as the ones that
 *  are present in both are set to the same values.
 *
 *  Some of the prototypes refer to functions that may not be
 *  present in your libc.a. This is ok so long as you do not
 *  actually call such functions.
 *
 */

#ifndef FILE

/* check and possibly comment out the following */
#define HAVE_BUFSIZ 
#define SPRINTF_RETURNS_INT

/* check and possibly redefine the following */
#define BUFSIZ  1024            

extern  struct  _iobuf {
    int      _cnt;
    char*    _ptr;
    char*    _base;
#   ifdef HAVE_BUFSIZ
    int     _bufsiz;
    short   _flag;
#   else
    char    _flag;
#   endif
    char    _file;
} _iob[];

#define FILE     struct _iobuf

#define _IOFBF    00000
#define _IOREAD   00001
#define _IOWRT    00002
#define _IONBF    00004
#define _IOMYBUF  00010
#define _IOEOF    00020
#define _IOERR    00040
#define _IOSTRG   00100
#define _IOLBF    00200
#define _IORW     00400
#define _IOAPPEND 01000

#define EOF       (-1)

#ifndef NULL
#define NULL      0
#endif

#define stdin     (&_iob[0])
#define stdout    (&_iob[1])
#define stderr    (&_iob[2])

#define getc(p) (--(p)->_cnt>=0?(int)(*(unsigned char*)(p)->_ptr++):_filbuf(p))
#define putc(x,p) (--(p)->_cnt>=0? ((int)(*(p)->_ptr++=(unsigned)(x))):_flsbuf((unsigned)(x),p))

#define clearerr(p) ((p)->_flag &= ~(_IOERR|_IOEOF))
#define getchar()   getc(stdin)
#define putchar(x)  putc(x,stdout)
#define feof(p)     (((p)->_flag&_IOEOF)!=0)
#define ferror(p)   (((p)->_flag&_IOERR)!=0)
#define fileno(p)   ((p)->_file)

extern int    _doprnt(const char*, void*, FILE*);
extern int    _doscan(FILE*, const char*, void*);
extern int    _filbuf(FILE*);
extern int    _flsbuf(unsigned, FILE*);
extern int    fclose(FILE*);
extern FILE*  fdopen(int, const char*);
extern int    fflush(FILE*);
extern int    fgetc(FILE*);
extern char*  fgets(char*, int, FILE *);
extern FILE*  fopen(const char*, const char*);
extern int    fprintf(FILE*, const char* ...);
extern int    fputc(int, FILE*);
extern int    fputs(const char*, FILE*);
extern int    fread(void*, int, int, FILE*);
extern FILE*  freopen(const char*, const char*, FILE*);
extern int    fscanf(FILE*, const char* ...);
extern int    fseek(FILE*, long, int);
extern long   ftell(FILE *);
extern int    fwrite(const void*, int, int, FILE*);
extern char*  gets(char*);
extern int    getw(FILE*);
extern int    pclose(FILE*);
extern FILE*  popen(const char*, const char*);
extern int    printf(const char* ...);
extern void   puts(const char*);
extern int    putw(int, FILE*);
extern int    scanf(const char* ...);
extern void   setbuf(FILE*, char*);
extern void   setbuffer(FILE*, char*, int);
extern void   setlinebuf(FILE*);
extern void   setvbuf(FILE*, char*, int, int);
extern int    sscanf(char*, const char* ...);
extern FILE*  tmpfile();
extern int    ungetc(int, FILE*);
extern int    vfprintf(FILE*, const char*, void* ap);
extern int    vprintf(const char*, void* ap);
extern int    vsprintf(char*, const char*, void* ap);

#ifdef SPRINTF_RETURNS_INT
extern int    sprintf(char*, const char* ...);
#else
extern char*  sprintf(char*, const char* ...);
#endif

#endif // FILE
