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

#ifndef STREAMH 
#define STREAMH

#include <File.h>

class whitespace                // a class used only to input and
{                               // discard white space characters
  char filler;                     
};

class ostream: File
{
public:
           File::open;      File::close;
           File::remove;    File::filedesc;  File::is_open;
           File::raw;       File::put;       File::form;
           File::iocount;   File::error;     File::name;
           File::setname;   File::rdstate;   File::flush;
           File::eof;       File::fail;      File::bad;
           File::good;      File::clear;     File::failif;
           File::setbuf;    File::writable;  File::readable;
           

           ostream() {}
           ostream(const char* filename, io_mode m, access_mode a);
           ostream(const char* filename, const char* m);
           ostream(int filedesc, io_mode m = io_writeonly);
           ostream(FILE* fileptr);

           ~ostream() {}

  void*    operator void*();

  ostream& operator << (char   c);
  ostream& operator << (short  n);
  ostream& operator << (unsigned short n);
  ostream& operator << (int    n);
  ostream& operator << (unsigned int n);
  ostream& operator << (long   n);
  ostream& operator << (unsigned long n);
  ostream& operator << (float  n);
  ostream& operator << (double n);
  ostream& operator << (const char* s);
};


class istream: File
{
  ostream* tied_to;
public:
           File::open;      File::close;     File::get;
           File::remove;    File::filedesc;  File::is_open;
           File::raw;       File::unget;     File::scan;
           File::iocount;   File::error;     File::name;
           File::setname;   File::rdstate;   File::putback;
           File::eof;       File::fail;      File::bad;
           File::good;      File::clear;     File::failif;
           File::setbuf;    File::writable;  File::readable;


           istream() { tied_to = 0; }
           istream(const char* filename, io_mode m, access_mode a);
           istream(const char* filename, const char* m);
           istream(int filedesc, io_mode m = io_readonly, ostream* t = 0);
           istream(FILE* fileptr);

           ~istream() {}

  void*    operator void*();
  ostream* tie(ostream* s) { ostream* t = tied_to; tied_to = s; return t; }

  istream& operator >> (char&   c);
  istream& operator >> (short&  n);
  istream& operator >> (unsigned short& n);
  istream& operator >> (int&    n);
  istream& operator >> (unsigned int& n);
  istream& operator >> (long&   n);
  istream& operator >> (unsigned long& n);
  istream& operator >> (float&  n);
  istream& operator >> (double& n);
  istream& operator >> (char*   s);
  istream& operator >> (whitespace& w);
};


// pre-declared streams

extern istream  cin;             // stdin
extern ostream  cout;            // stdout
extern ostream  cerr;            // stderr

extern whitespace WS;            // for convenience

#endif // STREAMH
