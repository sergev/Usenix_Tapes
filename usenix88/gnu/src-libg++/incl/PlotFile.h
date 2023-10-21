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
   a very simple implementation of a class to output unix "plot"
   format plotter files. See corresponding unix man pages for
   more details. 
*/

#ifndef PLOTFILEH
#define PLOTFILEH

#include <File.h>

/*   
   Some plot libraries have the 'box' command to draw boxes. 
   If yours does, uncomment the following line
*/

// #define HAVE_BOX_CMD

class PlotFile : File
{
  PlotFile& cmd(char c);
  PlotFile& operator << (int x);
  PlotFile& operator << (char *s); 

public:
            File::open;      File::close;     File::raw;       
            File::remove;    File::filedesc;  File::is_open;
            File::iocount;   File::error;     File::name;
            File::setname;   File::rdstate;   File::put;
            File::eof;       File::fail;      File::bad;
            File::good;      File::clear;     File::failif;
            File::setbuf;    File::writable;  File::readable;

            PlotFile() {}
            PlotFile(const char* filename, io_mode m, access_mode a);
            PlotFile(const char* filename, const char* m);
            PlotFile(int filedesc, io_mode m = io_writeonly);
            PlotFile(FILE* fileptr);

           ~PlotFile() {}

  void*     operator void*();

  PlotFile& arc(int xi, int yi, int x0, int y0, int x1, int y1);
  PlotFile& circle(int x, int y, int r);
  PlotFile& cont(int xi, int yi);
  PlotFile& dot(int xi, int yi, int dx, int n, int* pat);
  PlotFile& erase(); 
  PlotFile& label(char* s);
  PlotFile& line(int x0, int y0, int x1, int y1);
  PlotFile& linemod(char* s);
  PlotFile& move(int xi, int yi);
  PlotFile& point(int xi, int yi);
  PlotFile& space(int x0, int y0, int x1, int y1);

#ifdef HAVE_BOX_CMD
  PlotFile& box(int x0, int y0, int x1, int y1);
#endif
};


#endif                          // PLOTFILEH
