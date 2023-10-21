
/*
 * GLOBALS.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  (C) 1985  Matthew Dillon
 *
 *  Declarations for most global variables.
 *
 */

#include <stdio.h>
#include "dmail.h"

FILE *m_fi;                         /* open file ptr to spool file  */
char *mail_file;                    /* name of from (spool) file    */
char *output_file;                  /* name of out file (i.e. mbox) */
char *user_name;                    /* user name from password entry*/
char *home_dir;                     /* home directory of user       */
char *visual;                       /* visual editor path           */
char Buf[MAXFIELDSIZE];             /* Scratch Buffer               */
char Puf[MAXFIELDSIZE];             /* Another Scratch Buffer       */
jmp_buf env[LONGSTACK];             /* Holds longjump stack         */
int  Debug;                         /* Debug mode                   */
int  _ls, Longstack, Breakstack;    /* longjump/break level stack   */
int  Entries, Current;              /* #Entries and Current entry   */
int  ac;                            /* internal argc, from/to stat  */
int  No_load_mail;                  /* disable loading of mail      */
int  Silence;                       /* -s command option status     */
struct ENTRY *Entry;                /* Base of Entry list           */
char *av[128];                      /* internal argv[]              */

int width[MAXLIST]  = { 18, 38, 10 };   /* Default setlist params       */
int header[MAXLIST] = {  0,  2,  1 };
int Listsize = 3;

/* The following are globals variables definable from the 'set' command */

char  *S_sendmail;                  /* sendmail program path        */
int   S_page;                       /* Paging status                */
int   S_novibreak;                  /* vi-break status              */
int   S_verbose;                    /* sendmail verbose status      */
int   S_ask;


