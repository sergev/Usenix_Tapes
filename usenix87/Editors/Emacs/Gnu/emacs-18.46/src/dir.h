/* GNU Emacs VMS directory definition file.
   Copyright (C) 1986 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU Emacs General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU Emacs, but only under the conditions described in the
GNU Emacs General Public License.   A copy of this license is
supposed to have been given to you along with GNU Emacs so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */

/*
 * Files-11 Ver. 2 directory structure (VMS V4.x - long names)
 */
#ifndef DIR$K_LENGTH

#define	DIR$C_FID	0
#define	DIR$C_LINKNAME	1
#define	DIR$K_LENGTH	6
#define	DIR$C_LENGTH	6
#define	DIR$S_DIRDEF	6
#define	DIR$W_SIZE	0
#define	DIR$W_VERLIMIT	2
#define	DIR$B_FLAGS	4
#define	DIR$S_TYPE	3
#define	DIR$V_TYPE	0
#define	DIR$V_NEXTREC	6
#define	DIR$V_PREVREC	7
#define	DIR$B_NAMECOUNT	5
#define	DIR$S_NAME	80
#define	DIR$T_NAME	6

#define	DIR$K_VERSION	8
#define	DIR$C_VERSION	8
#define	DIR$S_DIRDEF1	8
#define	DIR$W_VERSION	0
#define	DIR$S_FID	6
#define	DIR$W_FID	2
#define	DIR$W_FID_NUM	2
#define	DIR$W_FID_SEQ	4
#define	DIR$W_FID_RVN	6
#define	DIR$B_FID_RVN	6
#define	DIR$B_FID_NMX	7

#define	DIR$S_DIRDEF2	1
#define	DIR$T_LINKNAME	0

typedef
struct dir$_name {
/*short   dir$w_size;		/* if you read with RMS, RMS eats this */
  short   dir$w_verlimit;
  union {
    unsigned char    dir_b_flags;
#define dir$b_flags dir__b_flags.dir_b_flags
    unsigned char   
      dir_v_type : DIR$S_TYPE,
#define dir$v_type dir__b_flags.dir_v_type
      : 3,
      dir_v_nextrec : 1,
#define dir$v_nextrec dir__b_flags.dir_v_nextrec
      dir_v_prevrec : 1;
#define dir$v_prevrec dir__b_flags.dir_v_prevrec
  } dir__b_flags;
  char    dir$b_namecount;
  char    dir$t_name[];
} dir$_dirdef;      /* only the fixed first part */

typedef
struct dir$_version {
  short dir$w_version;
  short	dir$w_fid_num;
  short dir$w_fid_seq;
  union {
    short dir_w_fid_rvn;
#define dir$w_fid_rvn dir_w_fid_rvnmx.dir_w_fid_rvn
    short
      dir_b_fid_rvn : 8,
#define dir$b_fid_rvn dir_w_fid_rvnmx.dir_b_fid_rvn
      dir_b_fid_nmx : 8;
#define dir$b_fid_nmx dir_w_fid_rvnmx.dir_b_fid_nmx
  } dir_w_fid_rvnmx;
} dir$_dirdef1;      /* one for each version of the file */

typedef
struct dir$_linkname {
  char    dir$t_linkname[];
} dir$_dirdef2;

#endif
