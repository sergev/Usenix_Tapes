
/*
 *    help.h: header file for the VMS-help emulator
 */
#define HELPVER         "HELP - VMS-like help utility\nVersion 1.0.014NL  -  6 January 1986\nRobert P. Scott\nDTS Engineering & Consulting\n29 Heritage Circle\nHudson, NH  03051-3427\n(603)886-1383\n"

#define STDDELIM        " \r\n\t;"          /* delimiters for parsing */
#define HELPMAIN        "HELP.HLM"          /* main help text filename */
#define MAXENT          128                 /* maximum number of help entries*/
#define HELPEX          ".HLP"              /* help extension */
#define SWITCHR1        '-'                 /* unix like switch char */
#define SWITCHR2        '/'                 /* PC/MS-DOS std switch char */
#define PROMPT          "Topic? "
#define DEFSRCH1        ".\\*.*"             /* Default search string */
#define R_OK            04
#define MAXLINELEN      90
#define MAXNAMELEN      12                  /* max length of NAME */
#define COLUMNSPACE     4
#define TERMWID         76
#define HELPDIR         "/usr/help"
#define ALL             0x37
#define DEFSCRLEN       17

int         scrnlen;
int         status;
int         second;                     /* first time indicator for allsel */
char        dent[MAXENT][MAXNAMELEN];
char        lent1[MAXENT][MAXLINELEN];
char        lent2[MAXENT][MAXLINELEN];
char        progname[MAXNAMELEN+4];
char        olddir[MAXLINELEN];
char        newdir[MAXLINELEN];
char        currdir[MAXLINELEN];
char        prompt[MAXLINELEN];
char        *helpdir;

/*global*/  int help(char *);
/*global*/  int outhelp(char *);
/*global*/  int allsel(void);
/*global*/  void set_dta(char *);
/*global*/  int dir_get(char *,int );
/*global*/  char *gettok(char *,char *,char *,char *);
/*global*/  int isdelim(char ,char *);
/*global*/  int more(char *);
/*global*/  int pgbrk(void);
/*global*/  void stripnl(char *);
/*global*/  void setprompt(void);
/*global*/  void fstobs(char *);
/*global*/  void handler(void);
/*global*/  int kget(void);
======================= END of HELP.H ====================================

The following file is 'help.c':
======================= Beginning of HELP.C ==============================
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <conio.h>
#include <signal.h>
#include <ctype.h>
#include <direct.h>
#include <process.h>
#include <string.h>
#include <dos.h>
#include "help.h"
 
/*****************************************************************************
 *  HELP
 *
 *  This program provides a HELP environment similar to the VMS help facility.
 *  Instead of using a help library, however, this version uses a directory 
 *  tree of text files.  This was done primarily to allow easy editing and 
 *  addition of help entries.
 *
 *  The set of arguments passed from DOS is first scanned for switches.  Then 
 *  the remaining arguments are passed to the main help routine as a single
 *  string which is reparsed.  This was done to allow easy recursion.
 *
 *  Similar to the public domain help utility found on many UNIX systems, a 
 *  help directory contains five possible entries in it.
 *        main help text:          HELP.HLM
 *        manual page name:        HELP.MAN
 *        subtopic texts:          <topicname>.HLP
 *        subtopic directories:    <topicname>
 *
 *  Due to the filename size limitation of DOS, command line tokens are
 *  limited to eight characters.
 */
void main(argc, argv)
int argc;
char *argv[];
    {
    char        inbuf[MAXLINELEN];
    int         il;                         /* input buffer length */
    int         sl;                         /* token length */
    int         done;                       /* temporary flag */
    char        c;                          /* temporary char holder */
    char        *op;                        /* single argument pointer */
        
    strcpy( progname,*argv++ );         /* Always save the program name */

    second = 1;                         /* to tell allsel "first pass" */
    
    helpdir = getenv( "HELP" );         /* use EV for directory if set */    
    getcwd( olddir, MAXLINELEN );       /* save our startind directory */

    if ( signal( SIGINT, handler ) < 0 )    /* uh oh */
        {
        fprintf( stderr, "%s could not set SIGINT.  Action undefined upon INT\n", progname );
        }
        
    argc--;
    while (( argc ) && ( ( c = *(*argv) ) == SWITCHR1 ) || ( c == SWITCHR2 ))
        {        /* must be options */
        for (op = *argv; *op != '\0'; op++) 
            {
            switch(*op) 
                {
                case SWITCHR1 :    /* ignore first and extras */
                case SWITCHR2 :
                    break;
 
                case 'd' :    /* specifying help directory path explicitly */
                    helpdir = *++argv;
                    break;
 
                case 'h' :
                case 'v' :      /* show version number */
                    fprintf( stdout, HELPVER );
                    exit( 0 );
                    
                default:
                    fprintf(stderr,"%s: %c: bad option.\n",
                    progname,*op);
                    break;
                }   /* switch */
            }   /* for ( steps through token ) */
        argc--; argv++;
        }   /* while ( loops on argv arguments ) */
 
    if (helpdir == NULL)        /* if all else fails, use default */
        helpdir = HELPDIR;
 
    fstobs( helpdir );           /* normalize to dos directory seps */
    
    if (chdir(helpdir) < 0)     /* move to the help root */
        {
        fprintf(stderr,"%s: %s: help directory not found.\n",
            progname,helpdir);
        exit(1);
        }
 
    done = il = 0;                             /* reset input length */
    inbuf[0] = 0;                       /* start with a null buffer */
    while ( ( argc ) && ( *argv ) && !done )   /* concat. all remaining args */
        {
        sl = strlen( *argv );           /* new token length */
        if ( ( il + sl ) < MAXLINELEN ) /* if enough room for added token */
            {
            if ( il && ( il < MAXLINELEN ) )    /* if not the first token */
                {
                strcat( inbuf, " " );   /* add a space */
                strcat( inbuf, *argv ); /* and add the token */
                }
            else                        /* the first token */
                strcpy( inbuf, *argv ); /* copy in the first token */
            il += sl + 1;               /* adjust the inbuf length ctr */
            }
        else                            /* no more room for added tokens */
            done = 1;                   /* and we don't want junk added */            
        argv++;
        argc--;
        }

    strupr( inbuf );                    /* convert to upper case */    
    help( inbuf );
         
    chdir( olddir );                    /* go back to our origin */
    exit(0);
    }
 

/****************************************************************************
 */
help( buf )
char    *buf;
    {
    int     done = 0;
    int     subj = 0;
    char    lbuf[MAXLINELEN];           /* local temporary copy */
    char    tbuf[MAXLINELEN];           /* local token buffer */
    int     lbufl, bufl;                /* buffer lengths */
    
    while( !done )                      /* loop until user is tired */
        {
        if ( !buf || !*buf ) /* was input specified? */
            {
            if ( !subj )
                outhelp( NULL );
            subj = 0;                   /* no DEFAULT menu after subject */
            allsel();
            setprompt();
            fputs( prompt, stdout );        /* prompt user for input */
            if ( fgets( buf, MAXLINELEN, stdin ) == NULL )   /* EOF or error */
                return( -1 );
            }
        strupr( buf );                      /* convert to upper case */
        stripnl( buf );                     /* get rid of new line */
        if ( gettok( buf, STDDELIM, tbuf, lbuf ) == NULL ) /* no input token */
            return( 0 );

        strcpy( buf, lbuf );                /* for next pass */
        if ( tbuf[0] == '?' )           /* redisplay current main help */
            {
            }
        else
            {
/* go ahead and assume the input is a request for more help! */
            if ( ( status = outhelp( tbuf ) ) == -1 )
                {                       /* no help found, sorry */
                fprintf( stdout, "\nSorry, no help found on '%s'\n", tbuf );
                }
            else if ( status == 1 )
                {                       /* it was a subdir! */
                if ( ( status = help( buf ) ) < 0) /* parse further but... */
                    return( status );
                else                                /* otherwise, print main text */
                    {
                    chdir( ".." );              /* go back up for dos */
                    }
                }
            else
                subj = 1;
            }
        }    
    }


/****************************************************************************
 *  outhelp( str )      -   Output the given help file or ...
 *  char    *str;           A text file name, directory name, or NULL
 *
 *  If str is NULL, then print the HELP.HLM file in the current directory
 *  or append a .HLP to the str and print that file.
 */
outhelp( str )
char    *str;
    {
    char filename[MAXLINELEN];

    getcwd( currdir, MAXLINELEN );       /* save our startind directory */
        
    if ( !str || !*str ) /* if no input specified */
        strcpy( filename, HELPMAIN );       /* assume the default main file */
    else
        strcpy( filename, str );
        
    if ( chdir( filename ) == 0 )            /* see if it is a subdir by */
        {                                   /* trying to go to it */
        return( 1 );                        /* tell caller we are there */
        }
                
    strcat( currdir, "\\" );                 /* cause we gonna add toit */
    strcat( currdir, filename );
    
    if ( access(currdir, R_OK ) == 0 )     /* we got a valid file */
        {   
        more( filename );                   /* output the file */
        return( 0 );
        }
 
    strcat( currdir, HELPEX );              /* msdos kludge */
    strcat(filename,HELPEX);                /* add the default extension */
    
    if ( access( currdir, R_OK ) == 0 )     /* we got a valid file */
        {   
        more( filename );                   /* output the file */
        return( 0 );
        }
 
    return( -1 );                           /* not a help available */
    }
 

/****************************************************************************
 * allsel()                     print the topics available in this directory
 *
 *  This function prints a directory of the help files and dirs within this
 *  dir.  Extensions are stripped off.
 *    
 */
allsel()
    {
    static char     lastdir[MAXLINELEN];
    static int      totalnames;
    char            dta[80];
    char            *np = { DEFSRCH1 };
    int             counter;
    int             j;
    int             longlen;
    int             rowcnt;
    int             colcnt;
    int             namewidth;
    char            *s1;
    char            tmpbuf[MAXLINELEN];
    char            row[TERMWID+1];

    set_dta( dta );             /* for MSDOS */

    getcwd( currdir, MAXLINELEN );       /* save our startind directory */

    if ( second || ( strcmp( lastdir, currdir ) != 0 ) )
        {
        counter = 0;
        totalnames = 0;
        while( ( dir_get( np, ALL ) == 0 ) && ( counter < (MAXENT-1) ) )
            {
            np = NULL;
            strcpy ( tmpbuf, &dta[30] );
            if ( tmpbuf[0] != '.' )  /* if it is a directory or root file */
                {
                if ( ( s1 = strchr( tmpbuf, '.' ) ) == NULL )
                    {                               /* look for ext */
                    strcpy( dent[counter++], tmpbuf );
                    totalnames++;
                    }
                else if ( strcmp( s1, HELPEX ) == 0 )   /* its a .HLP file */
                    {                               /* look for ext */
                    *s1 = 0;                        /* terminate after fn */
                    strcpy( dent[counter++], tmpbuf );
                    totalnames++;
                    }
                }
            }
        dent[counter][0] = 0;           /* terminating entry */

        /* sort the names in ascending order with exchange algorithm */
        for(counter=0; counter < totalnames-1; counter++)
            for(j=counter+1; j <totalnames; j++)
                if (strcmp(dent[counter],dent[j]) > 0) 
                    {
                    strcpy( tmpbuf, dent[counter] );
                    strcpy( dent[counter], dent[j] );
                    strcpy( dent[j], tmpbuf );
                    }
        }
    if (totalnames == 0) 
        return(0);

/* this next section will be made into a subroutine */
/* rebuild code from here --\/ */
    longlen = 0;
    for(counter=0; counter < totalnames; counter++ )
        longlen = ((j=strlen(dent[counter]))>longlen)?j:longlen;
 
    /* here print the names out in nice columns */
    namewidth = longlen + COLUMNSPACE;
    rowcnt = TERMWID / namewidth;
    colcnt = (totalnames + (rowcnt-1)) / rowcnt ;
    if (colcnt <= 0) 
        colcnt = 1;
    
/*    if (col_flag && rowcnt > 1)   */
    if ( rowcnt > 1 ) 
        {
        fprintf( stdout, "\n Topics:\n\n");
        for(counter=0; counter < colcnt ; counter++ ) 
            {
            for(j=0; j < TERMWID; row[j++] = ' ');
            row[j] = '\0';
            for(j=0, s1 = row; (counter+j) < totalnames; j += colcnt) 
                {
                row[strlen(row)] = ' ';
                strcpy(s1,dent[counter+j]);
                s1 = s1 + namewidth;
                }
            fprintf( stdout, "    %s\n",row);
            }
        putchar('\n');
        }
    else 
        {
        for(counter=0; counter < totalnames; counter++)
            fprintf( stdout, "%s\n", dent[counter] );
        }
/*                to here --/\ */
 
    return( 0 );
    }
 


void set_dta( dta )
char    *dta;
    {
    union REGS rg;

    rg.h.ah = 0x1a;
    rg.x.dx = ( unsigned int )dta;

    int86( 0x21, &rg, &rg );
    }  

dir_get( fn, type )
char    *fn;
int     type;
    {
    union REGS rg;

    if ( fn )
        {
        rg.x.dx = ( unsigned int )fn;
        rg.x.cx = type;
        rg.h.ah = 0x4e;

        int86( 0x21, &rg, &rg );
        if ( rg.x.cflag & 0x01 )
            return( rg.x.ax );
        }
    else
        {
        rg.h.ah = 0x4f;
            
        int86( 0x21, &rg, &rg );
        if ( rg.x.cflag & 0x01 )
            return( rg.x.ax );
        }
    return( 0 );
    }
    

/****************************************************************************
 *  gettok( is, delim, tb, rb )     - Find token in input string        
 *  char    *is;        Pointer to input string
 *  char    *delim;     Pointer to set of delimiting characters
 *  tb      *tb         Pointer to buffer to place token
 *  rb      *rb         Pointer to buffer to place ( input string - token )
 *
 *  This function reads the input string looking for a token delimited by
 *  the beginning of the input buffer or a character in the delimiting set
 *  on the leading edge, and the end of the input buffer or a delimiting 
 *  character on the trailing edge.  The first character of the buffer 
 *  which is not a character in the set of delimiting characters is taken as
 *  the first character of the token.
 *
 *  If a pointer is passed in tb and/or rb, it is assumed to be a buffer of
 *  adequate size to hold the resultant string.  A NULL may be passed in these
 *  variables if the result is not desired.
 *
 *  Returns:    A pointer to the first not delimiting character.
 */
char *gettok( is, delim, tb, rb )
char    *is;     
char    *delim;  
char    *tb;
char    *rb;
    {
    char    *fc;                    /* pointer to first character of token */

    fc = NULL;                      /* default no token found */
        
    if ( tb )                       /* if user gave us a token buffer */
        *tb = 0;                    /* terminate the token buffer */        
    if ( rb )                       /* if user gave us a remainder buffer */
        *rb = 0;                    /* terminate the token buffer */        

    if ( is == NULL )               /* oops, got a 2001 */
        return( NULL );
        
    while ( *is && isdelim( *is, delim ) ) /* while leading delimiter */
        is++;                       /* skip it */
    if ( !*is )
        return( NULL );
    fc = is;                        /* save the pointer the first char */
    while ( *is && !isdelim( *is, delim ) )    /* while not a delimiter */
        {
        if ( tb )                   /* if user gave us a token buffer */
            {
            *tb = *is;              /* move the character */
            tb++;                   /* increment token buffer ptr */
            }
        is++;
        }
    if ( tb )                       /* if user gave us a token buffer */
        *tb = 0;                    /* terminate the token buffer */        

    if ( rb )                       /* if user gave us a remainder buffer */
        strcpy( rb, is );           /* copy the remainder */

    return( fc );
    }
            
isdelim( c, delim )
char    c;
char    *delim;
    {
    int     dc;                     /* delimiter count */
    int     i;                      /* temporary counter */
    char    *tc;                    /* termporary pointer */
    
    tc = delim;                     /* time to count the donuts */
    dc = 0;
    while  ( *tc++ )    
        dc++;

    for ( i=0; i<dc; i++ )          /* loop through the delimiters */
        {
        if ( c == delim[i] )        /* if a delimiter */
            return( 1 );            /* return found */
        }
    return( 0 );                    /* return not found */
    }            


more( fn )
char    *fn;
    {
    FILE    *sfd;
    char    buf[512];
    int     lct;
        
    lct = 0;
    scrnlen = DEFSCRLEN;
    
    if ( ( sfd = fopen( fn, "r" ) ) != NULL ) /* got a file */
        {
        while( fgets( buf, 512, sfd ) )
            {
            if ( lct == scrnlen )
                {
                lct = 0;
                if ( pgbrk() )
                    {
                    fclose( sfd );
                    return( 1 );
                    }
                }
            fputs( buf, stdout );
            lct++;
            }
        }
    fclose( sfd );
    return( 0 );
    }
    
pgbrk()
    {
    char    c;
    
    fprintf( stdout, "More (Y|N)" );
    c = ( char )kget();
    fputc( '\n', stdout );
    if ( c == 'N' )
        return( 1 );
    return( 0 );
    }

void stripnl( buf )
char    *buf;
    {
    while ( *buf )
        {
        if ( *buf == '\n' )
            *buf = 0;
        buf++;
        }
    }    
    

void setprompt()
    {
    char    *hs;                        /* pointer to the first / in helpdir */
    char    *cs;                        /* pointer to first / in current dir */
    char    *pp;                        /* pointer into prompt */
    
    getcwd( currdir, MAXLINELEN );       /* save our startind directory */

    hs = strchr( helpdir, '\\' );       /* ignore drive names */
    cs = strchr( currdir, '\\' );       /* ignore drive names */

    while ( *hs && ( *hs == *cs ) )     /* skip help directory root */
        {
        hs++;
        cs++;
        }
    
    pp = prompt;
    while ( *cs )                       /* use the rest for the prompt */
        {
        if ( *cs == '\\' )              /* convert to spaces for readablty */
            *cs = ' ';
        *pp++ = *cs++;
        }
    if ( pp != prompt )                 /* not at root... */
        *pp++ = ' ';                    /* add a pleasing space */

    *pp = 0;                            /* terminate the string */
    
    strcat( prompt, PROMPT );           /* and the default ending */
    }
      
void fstobs( str )                   
char    *str;
    {
    strupr( str );                  /* dos only understands one case */
    while ( *str )
        {
        if ( *str == '/' )          /* convert UNIX to MSDOS seps */
            *str = '\\';
        str++;
        }
    }
    
void handler()
    {
    char    c;
    
    if ( signal( SIGINT, handler ) < 0 )    /* uh oh */
        {
        fprintf( stderr, "%s could not set SIGINT.  Action undefined upon INT\n", progname );
        }

    fprintf( stdout, "Terminate process [y|N]" );
    c = ( char )kget();    
    fputc( '\n', stdout );
    if ( c == 'Y' )
        {
        chdir( olddir );                    /* go back to our origin */
        exit(0);
        }
    }
    
kget()
    {
    char c;
    
    while ( kbhit() == 0 )
        {
        }

    c = ( char )getche();
    c = toupper( c );

    return( c );
    }
======================= END of HELP.C ====================================
    
The following file is a MSC V4.0 makefile:
======================= Beginning of HELP ================================
OBJS = 	help.obj 
 
LIBFLAGS = 
# LIBFLAGS = /CO /MAP /LINE
 
# /AL means use large model
CFLAGS = /AS /Ot
# CFLAGS = /AS /Od /Zd /Zi 
 
# default rules
.c.obj:
	msc $(CFLAGS) $*;
 
.asm.obj:
	masm $*;
 
 
help.obj: help.c help.h
 
help.exe: $(OBJS)
        link$(LIBFLAGS) help,help;

#   help.exe: $(OBJS)
#		link$(LIBFLAGS) help,help ;
#	        mapsym /L help.map
        
