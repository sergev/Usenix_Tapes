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

#ifndef STDDEFH

#define STDDEFH

#ifndef TRUE                    // disable if already done #define TRUE 1...
enum bool   
{           
  FALSE = 0,
  TRUE  = 1              
};
#endif

#ifndef L_ctermid
#define L_ctermid	9 
#endif
#ifndef L_cuserid
#define L_cuserid	9
#endif
#ifndef P_tmpdir
#define	P_tmpdir    "/tmp/"
#endif
#ifndef L_tmpnam
#define	L_tmpnam    (sizeof(P_tmpdir) + 15)
#endif


#ifndef NULL
#define NULL ((void *)0)
#endif

typedef void (*one_arg_error_handler_t)(char*);

#endif
