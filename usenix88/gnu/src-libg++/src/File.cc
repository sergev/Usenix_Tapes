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

#include <std.h>
#include <stdarg.h>
#include <File.h>

#include <sys/file.h>           // needed to determine values of O_RDONLY...

#ifndef sun
#include "/usr/include/fcntl.h"
#endif


/*
   Other system dependent parameters:

   HAVE_VPRINTF should be set if vprintf is in libc.a
   HAVE_SETVBUF should be set if setvbuf is in libc.a
   HAVE_SETLINEBUF should be set if setlinebuf in libc.a

   The following are correct for vax BSD4.3 and sun. Others not yet known
*/

#if defined(vax)
//#define HAVE_VPRINTF
//#define  HAVE_SETVBUF  
#define HAVE_SETLINEBUF

#elif defined(sun)
#define  HAVE_VPRINTF
#define  HAVE_SETVBUF  
#define  HAVE_SETLINEBUF
#endif


// error handlers

void verbose_File_error_handler(char* msg)
{
  perror(msg);
  errno = 0;
}

void quiet_File_error_handler(char*)
{
  errno = 0;
}

void fatal_File_error_handler(char* msg)
{
  perror(msg);
  exit(1);
}

one_arg_error_handler_t File_error_handler = verbose_File_error_handler;


one_arg_error_handler_t set_File_error_handler(one_arg_error_handler_t f)
{
  one_arg_error_handler_t old = File_error_handler;
  File_error_handler = f;
  return old;
}


/*

 Opening files. 

 open(filename, io_mode, access_mode) is done via system open 
 command since fopen doesn't handle all of the cases possible 
 with sys open. After a successful open, fdopen is called to 
 attach an _iobuf to the file descriptor.

 All this requires a few decoding routines that can translate among our
 enumerated types, system flags, and fopen modes.

*/


enum sys_open_cmd_io_mode  // These should be correct for most systems
{                        
  sio_read      = O_RDONLY,
  sio_write     = O_WRONLY,
  sio_readwrite = O_RDWR,
  sio_append    = O_APPEND
};

enum sys_open_cmd_access_mode
{
  sa_create     = O_CREAT,
  sa_truncate   = O_TRUNC,
  sa_createonly = O_EXCL
};

  
static int open_cmd_arg(io_mode i, access_mode a) // decode modes
{
  int arg;
  switch(i)
  {
  case io_readonly:   arg = sio_read;                   break;
  case io_writeonly:  arg = sio_write;                  break;
  case io_readwrite:  arg = sio_readwrite;              break;
  case io_appendonly: arg = sio_append | sio_write;     break;
  case io_append:     arg = sio_append | sio_readwrite; break;
  default:            return -1;
  };
  switch(a)
  {
  case a_createonly:  return arg | sa_createonly;
  case a_create:      return arg | sa_create | sa_truncate;
  case a_useonly:     return arg;
  case a_use:         return arg | sa_create;
  default:            return -1;
  }
}

static char* fopen_cmd_arg(io_mode i)
{
  switch(i)
  {
  case io_readonly:  return "r";
  case io_writeonly: return "w";
  case io_readwrite: return "r+";
  case io_appendonly:return "a";
  case io_append:    return "a+";
  default:           return 0;
  }
}


void File::initialize() 
{ 
  fp = 0; nm = 0; stat = 0; state = _bad; rw = 0;
}

// reset class vars after open
// fp->_flag inspection is isolated here

void File::reinitialize(const char* filename)
{
  if (filename != 0)     setname(filename);
  else if (fp == stdin)  setname("(stdin)");
  else if (fp == stdout) setname("(stdout)");
  else if (fp == stderr) setname("(stderr)");
  else setname(0);

  rw = 0;
  if (fp != 0)
  {
    state = _good;
    if (fp->_flag & (_IOREAD|_IORW))
      rw |= 01;
    if (fp->_flag & (_IOWRT|_IORW|_IOAPPEND))
      rw |= 02;
    check_eof();
  }
  else
  {
    state = _fail|_bad;
    error();
  }
}


File& File::open(const char* filename, io_mode m, access_mode a)
{                                   
  close();
  int open_arg = open_cmd_arg(m, a);
  if (open_arg != -1)
  {
    int fd = ::open(filename, open_arg, 0666);
    if (fd >= 0)
      fp = fdopen(fd, fopen_cmd_arg(m));
  }
  reinitialize(filename);
  return *this;
}

File& File::open(const char* filename, const char* m)
{                                   
  close();
  fp = fopen(filename, m);
  reinitialize(filename);
  return *this;
}

File& File::open(FILE* fileptr)
{
  close();
  fp = fileptr;
  reinitialize(0);
  return *this;
}

File& File::open(int filedesc, io_mode m)
{
  close();
  fp = fdopen(filedesc, fopen_cmd_arg(m));
  reinitialize(0);
  return *this;
}

File& File::close()
{
  if (fp != 0)
    fclose(fp);
  fp = 0;
  state = _bad;
  return *this;
}

File& File::remove()
{
  close();
  return failif (nm == 0 || unlink(nm) != 0);
}


File::File()
{ 
  initialize(); 
}

File::File(const char* filename, io_mode m, access_mode a)   
{ 
  initialize(); 
  open(filename, m, a); 
}

File::File(const char* filename, const char* m)   
{ 
  initialize(); 
  open(filename, m); 
}

File::File(int filedesc, io_mode m)
{ 
  initialize(); 
  open(filedesc, m); 
}

File::File(FILE* fileptr)
{ 
  initialize(); 
  open(fileptr); 
}


File::~File()
{
  delete(nm);
  if (fp == stdin || fp == stdout || fp == stderr)
    flush();
  else if (fp != 0)
    File::close();
}

void File::setname(const char* newname)
{
  if (nm != 0)
    delete(nm);
  if (newname != 0)
  {
    nm = new char[strlen(newname) + 1];
    strcpy(nm, newname);
  }
  else
    nm = 0;
}


File& File::setbuf(int buffer_kind)
{                  
  if (!is_open())
  {
    error();
    return *this;
  }
  switch(buffer_kind)
  {
  case _IOFBF:       
#ifdef HAVE_SETVBUF
    setvbuf(fp, 0, _IOFBF, 0);
#endif
    break;           
  case _IONBF:       
    ::setbuf(fp, 0); 
    break;
  case _IOLBF:
#ifdef HAVE_SETLINEBUF
    setlinebuf(fp);
#else
#ifdef HAVE_SETVBUF
    setvbuf(fp, 0, _IOLBF, 0);
#endif
#endif    
    break;
  default:
    break;
  }
  return *this;
}

File& File::setbuf(int size, char* buf)
{
  if (!is_open())
  {
    error();
    return *this;
  }
#ifdef HAVE_SETVBUF
  setvbuf(fp, buf, _IOFBF, size);
#else
  setbuffer(fp, buf, size);
#endif
  return *this;
}

void File::error()
{
  state |= _fail;
  check_eof();
  if (errno != 0)
  {
    char error_string[400];
    strcpy(error_string, "\nerror in File ");
    if (nm != 0)
      strcat(error_string, nm);
    (*File_error_handler)(error_string);
  }
}

//-------------------------------------------------------------------

// All of the following could be inlined if desired
// However, all are a bit long, and cause g++1.20.0 to complain
// or die for various reasons

#define COULD_BE_INLINE

COULD_BE_INLINE void File::check_eof() // ensure fp & state agree about eof
{
  if (fp != 0 && feof(fp))
    state |= _eof;
  else
    state &= ~_eof;
}

COULD_BE_INLINE File& File::failif(int cond)
{ 
  if (cond) error(); else state = _good;
  return *this; 
}

COULD_BE_INLINE File& File::get(char& c)
{ 
  if (readable())
  {
    int ch = getc(fp);
    c = ch;
    failif (ch == EOF);
  }
  return *this;
}

COULD_BE_INLINE File& File::put(char  c) 
{ 
  return failif (!writable() ||  putc(c, fp) == EOF);
}

COULD_BE_INLINE File& File::unget(char c)
{ 
  return failif(!is_open() || !(rw & 01) || ungetc(c, fp) == EOF);
} 

COULD_BE_INLINE File& File::putback(char c)
{ 
  return failif (!is_open() || !(rw & 01) || ungetc(c, fp) == EOF);
}

COULD_BE_INLINE File& File::read(void* x, int sz, int n)
{ 
  return failif (!readable() || (stat = fread(x, sz, n, fp)) != n);
} 

COULD_BE_INLINE File& File::write(void* x, int sz, int n) 
{ 
  return failif (!writable() || (stat = fwrite(x, sz, n, fp)) != n);
}

COULD_BE_INLINE  File& File::put(const char* s)
{ 
  return failif(!writable() || fputs(s, fp) == EOF);
}

COULD_BE_INLINE File& File::flush()
{ 
  return failif(!is_open() || fflush(fp) == EOF);
}

COULD_BE_INLINE File& File::seek(long pos, int seek_mode = 0)
{ 
  return failif (!is_open() || fseek(fp, pos, seek_mode) < 0); 
}

COULD_BE_INLINE long File::tell()
{ 
  failif (!is_open() || (stat = ftell(fp) < 0));
  return stat;
}

//------------------------------------------------------------------

File& File::get(char* s, int n, char terminator = '\n')
{
  if (!readable())
  {
    error();
    return *this;
  }

  char ch;
  stat = n;

  if (n > 0 && (get(ch)))
  {
    if (ch == terminator)
      unget(ch);
    else
    {
      *s++ = ch; --n;
      while (n > 0 && (get(ch)))
      {
        if (ch == terminator)
        {
          unget(ch);
          break;
        }
        else
        {
          *s++ = ch; --n;
        }
      }
    }
  }

  *s = 0;
  return failif((stat != 0) && ((stat -= n) == 0));
}

File& File::getline(char* s, int n, char terminator = '\n')
{
  if (!readable())
  {
    error();
    return *this;
  }

  char ch;
  stat = n;

  while (n > 0 && (get(ch)))
  {
    --n;
    if ((*s++ = ch) == terminator)
      break;
  }

  *s = 0;
  return failif((stat != 0) && ((stat -= n) == 0));
}

char _form_workspace[BUFSIZ];    // formatting buffer - 
                                 // sprintf expects something about this size.


File& File::scan(const char* fmt ...)
{
  va_list args;
  va_start(args, fmt);
  stat = _doscan(fp, fmt, args);
  va_end(args);
  return failif(stat <= 0);
}

File& File::form(const char* fmt ...)
{
  va_list args;
  va_start(args, fmt);
#ifndef HAVE_VPRINTF
  stat = _doprnt(fmt, args, fp);
#else
  stat = vfprintf(fp, fmt, args);
#endif
  va_end(args);
  return failif(stat < 0);
}


char* form(const char* fmt ...)
{
  va_list args;
  va_start(args, fmt);
#ifndef HAVE_VPRINTF
  FILE b;
  b._flag = _IOWRT|_IOSTRG;
  b._ptr = _form_workspace;
  b._cnt = BUFSIZ;
  _doprnt(fmt, args, &b);
  putc('\0', &b);
#else
  vsprintf(_form_workspace, fmt, args);
#endif
  va_end(args);
  return _form_workspace;
}

char* itoa(long x, int base = 10, int width = 0)
{
  char* e = &(_form_workspace[BUFSIZ]);
  char* s = e;
  char sgn = 0;

  *--s = 0;
  
  if (x == 0)
    *--s = '0';
  else
  {
    int z;
    if (x < 0)
    {
      sgn = '-';
      z = -x;
    }
    else
      z = x;
    while (z != 0)
    {
      char ch = z % base;
      z = z / base;
      if (ch >= 10)
        ch += 'a' - 10;
      else
        ch += '0';
      *--s = ch;
    }
  }

  if (sgn) *--s = sgn;
  int w = e - s - 1;
  while (w++ < width && s > _form_workspace)
    *--s = ' ';
  return s;
}

char* hex(long i, int width = 0)
{
  return itoa(i, 16, width);
}

char* oct(long i, int width = 0)
{
  return itoa(i, 8, width);
}

char* dec(long i, int width = 0)
{
  return itoa(i, 10, width);
}


#define SHOULD_BE_INLINE


SHOULD_BE_INLINE int File::filedesc()
{ 
  return fileno(fp);
}

SHOULD_BE_INLINE const char* File::name()
{ 
  return nm; 
}

SHOULD_BE_INLINE int File::rdstate()
{ 
  check_eof();  return state;
}

SHOULD_BE_INLINE void* File::operator void*()
{ 
  return (state & (_bad|_fail))? 0 : this ; 
}

SHOULD_BE_INLINE int File::eof()
{ 
  check_eof(); return state & _eof; 
}

SHOULD_BE_INLINE int File::fail()
{ 
  return state & _fail; 
}

SHOULD_BE_INLINE int File::bad()
{ 
  return state & _bad; 
}

SHOULD_BE_INLINE int File::good()
{ 
  return rdstate() == _good; 
}

SHOULD_BE_INLINE int File::iocount()
{ 
  return stat; 
}

SHOULD_BE_INLINE int File::readable()
{ 
  return ((rw & 01) && (state & (_fail|_bad)) == 0);
}

SHOULD_BE_INLINE int File::writable()
{ 
  return ((rw & 02) && (state & (_fail|_bad)) == 0);
}


SHOULD_BE_INLINE int File::is_open()
{ 
  return (fp != 0);
}

SHOULD_BE_INLINE void File::clear(state_value flag = 0)
{ 
  state = flag;
}

SHOULD_BE_INLINE File& File::raw()
{ 
  return File::setbuf(_IONBF); 
}

