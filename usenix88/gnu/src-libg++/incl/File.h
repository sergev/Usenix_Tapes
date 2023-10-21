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

#ifndef FILEH 
#define FILEH

#include <stdio.h>
#include <stddef.h>

enum io_mode                    // known unix file IO modes
{
  io_readonly   = 0,            
  io_writeonly  = 1,
  io_readwrite  = 2, 
  io_appendonly = 3,
  io_append     = 4,            // append, plus allow reads
};

enum access_mode                // ways to open a file
{
  a_createonly  = 0,            // create, fail if file exists
  a_create      = 1,            // create if doesn't exist, else truncate
  a_useonly     = 2,            // use (no truncate)  fail if doesn't exist
  a_use         = 3,            // use (no truncate), create if doesn't exist
};

enum state_value                // File states
{ 
  _good         = 0,            // all is well
  _eof          = 1,            // at eof
  _fail         = 2,            // logical or physical IO error
  _bad          = 4             // unopened/corrupted
};

class String;                   // to allow get/getline for Strings

class File
{
  FILE*         fp;              // _iobuf file pointer
  char*         nm;              // file name (dynamically allocated)
  char          rw;              //  1 = read; 2 = write; 3 = readwrite
  state_value   state;           // _good/_eof/_fail/_bad
  long          stat;            // last read/write/... return value

  void          initialize();
  void          reinitialize(const char*);
  void          check_eof();

public:
                File();
                File(const char* filename, io_mode m, access_mode a);
                File(const char* filename, const char* m);   
                File(int filedesc, io_mode m);
                File(FILE* fileptr);

                ~File();

// binding, rebinding, unbinding to physical files

  File&         open(const char* filename, io_mode m, access_mode a);
  File&         open(const char* filename, const char* m);
  File&         open(int  filedesc, io_mode m);
  File&         open(FILE* fileptr);

  File&         close();
  File&         remove();

// class variable access

  int           filedesc();
  const char*   name();
  void          setname(const char* newname);
  int           iocount();

  int           rdstate();
  int           eof();
  int           fail();
  int           bad();
  int           good();

// other status queries

  int           readable();
  int           writable();
  int           is_open();

  void*         operator void*();

// error handling

  void          error();
  void          clear(state_value f = 0); // poorly named
  File&         failif(int cond);

// character IO

  File&         get(char& c);
  File&         put(char  c);
  File&         unget(char c);
  File&         putback(char c); // a synonym for unget

// char* IO

  File&         put(const char* s);
  File&         get    (char* s, int n, char terminator = '\n');
  File&         getline(char* s, int n, char terminator = '\n');

// String IO

  File&         get    (String& x, int n, char terminator = '\n');
  File&         getline(String& x, int n, char terminator = '\n');

// binary IO

  File&         read(void* x, int sz, int n);
  File&         write(void* x, int sz, int n);

// formatted IO

  File&         form(const char* fmt, ...);
  File&         scan(const char* fmt, ...);

// file control

  File&         seek(long pos, int seek_mode=0); // default seek mode=absolute
  long          tell();

  File&         flush();
  File&         setbuf(int buffer_kind); // legal vals: _IONBF, _IOFBF, _IOLBF
  File&         setbuf(int size, char* buf);
  File&         raw();
};

// related non-class fcts

char*        itoa(long x, int base = 10, int width = 0);

char*        hex(long x, int width = 0);
char*        oct(long x, int width = 0);
char*        dec(long x, int width = 0);

char*        form(const char* fmt ...);

// error handlers

extern void  verbose_File_error_handler(char*);
extern void  quiet_File_error_handler(char*);
extern void  fatal_File_error_handler(char*);
extern one_arg_error_handler_t File_error_handler;

extern  one_arg_error_handler_t 
             set_File_error_handler(one_arg_error_handler_t f);
#endif // FILEH
