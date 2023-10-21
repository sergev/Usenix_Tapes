
/*
 * GLOBALS.C
 *
 * Matthew Dillon, 24 Feb 1986
 *
 */


#include "shell.h"

struct HIST *H_head, *H_tail;

struct PERROR Perror[] = {
   103,  "insufficient free storage",
   104,  "task table full",
   120,  "argument line invalid or too long",
   121,  "file is not an object module",
   122,  "invalid resident library during load",
   203,  "object already exists",
   204,  "directory not found",
   205,  "object not found",
   206,  "invalid window",
   210,  "invalid stream component name",
   212,  "object not of required type",
   213,  "disk not validated",
   214,  "disk write protected",
   215,  "rename across devices",
   216,  "directory not empty",
   218,  "device not mounted",
   220,  "comment too long",
   221,  "disk full",
   222,  "file delete protected",
   223,  "file write protected",
   224,  "file read protected",
   225,  "not a DOS disk",
   226,  "no disk in drive",
   209,  "packet request type unknown",
   211,  "invalid object lock",
   219,  "seek error",
   232,  "no more entries in directory",
     0,  NULL
};

char *av[MAXAV];
int   H_len, H_tail_base, H_stack;
int   ac;
int   Debug;

int   S_histlen = 20;



