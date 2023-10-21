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

#include <PlotFile.h>

/*
 PlotFile implementation module
*/



PlotFile::PlotFile(const char* filename, io_mode m, access_mode a) 
:(filename, m, a) {}

PlotFile::PlotFile(const char* filename, const char* m)
:(filename, m) {}

PlotFile::PlotFile(int filedesc, io_mode m = io_writeonly)
:(filedesc, m) {}

PlotFile::PlotFile(FILE* fileptr)
:(fileptr) {}

void* PlotFile::operator void*()
{ 
  return (fail() || bad())? 0 : this ; 
}


PlotFile& PlotFile:: cmd(char c)
{ 
  put(c); 
  return *this; 
}

PlotFile& PlotFile:: operator<<(int x)
{ 
  put((char)(x&0377)); 
  put((char)(x>>8)); 
  return *this; 
}

PlotFile& PlotFile:: operator<<(char *s) 
{ 
  put(s); 
  return *this;
}


PlotFile& PlotFile:: arc(int xi, int yi, int x0, int y0, int x1, int y1)
{ 
  return cmd('a') << xi << yi << x0 << y0 << x1 << y1; 
}

#ifdef HAVE_BOX_CMD

PlotFile& PlotFile:: box(int x0, int y0, int x1, int y1)
{ 
  return cmd('b') << x0 << y0 << x1 << y1; 
}

#endif

PlotFile& PlotFile:: circle(int x, int y, int r)
{ 
  return cmd('c') << x << y << r; 
}

PlotFile& PlotFile:: cont(int xi, int yi)
{ 
  return cmd('n') << xi << yi;
}

PlotFile& PlotFile:: dot(int xi, int yi, int dx, int n, int* pat)
{ 
  cmd('d') << xi << yi << dx << n;
  while (n-- > 0) *this << *pat++;
  return *this; 
}

PlotFile& PlotFile:: erase()
{ 
  return cmd('e'); 
}

PlotFile& PlotFile:: label(char* s)
{ 
  return cmd('t') << s << "\n"; 
}

PlotFile& PlotFile:: line(int x0, int y0, int x1, int y1)
{ 
  return cmd('l') << x0 << y0 << x1 << y1; 
}

PlotFile& PlotFile:: linemod(char* s)
{ 
  return cmd('f') << s << "\n"; 
}

PlotFile& PlotFile:: move(int xi, int yi)
{ 
  return cmd('m') << xi << yi;
}

PlotFile& PlotFile:: point(int xi, int yi)
{ 
  return cmd('p') << xi << yi; 
}

PlotFile& PlotFile:: space(int x0, int y0, int x1, int y1)
{ 
  return cmd('s') << x0 << y0 << x1 << y1; 
}
