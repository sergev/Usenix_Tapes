
/*
 *  globals.h
 *
 *  Global variables used by dpr and associated software.
 *  Copyright (c) Rich Burridge, Sun Australia 1986.
 *
 *  Version 1.1.
 *
 *  No responsibility is taken for any errors inherent either in the comments
 *  or the code of this program, but if reported to me then an attempt will
 *  be made to fix them.
 */

#include "dpr.h"

struct sparams sp ;                     /* Screen image copy (from screen or file). */

unsigned char color_map[3][MAXCMAP] ;   /* Colormap values. */

struct dumpcap dc[MAXCAP] =             /* Default printer capabilities. */
         {
           "br", NUMBER,  9600,  "",
           "co", NUMBER,  1,     "",
           "dd", BOOLEAN, 0,     "",
           "ed", STRING,  0,     "",
           "el", STRING,  0,     "",
           "fn", STRING,  0,     "",
           "ha", NUMBER,  NONE,  "",
           "lp", STRING,  0,     "/dev/lp",
           "ma", NUMBER,  0,     "",
           "nb", NUMBER,  8,     "",
           "ro", BOOLEAN, 0,     "",
           "sb", STRING,  0,     "",
           "sd", STRING,  0,     "",
           "sh", NUMBER,  900,   "",
           "sl", STRING,  0,     "",
           "sw", NUMBER,  1152,  "",
           "sx", NUMBER,  0,     "",
           "sy", NUMBER,  0,     "",
           "vs", NUMBER,  1,     "",
         } ;
