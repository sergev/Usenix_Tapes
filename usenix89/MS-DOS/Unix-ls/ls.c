/*
*
*!AU: Michael A. Shiels
*!CD: 1-Jun-86
*!FR: Dr. Dobbs July 1985
*
*/

#define LINT_ARGS
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dos.h>
#include	<malloc.h>
#include	"getargs.h"
#include	"masdos.h"

#define MAXDIR  500

#define EOS(s)  while(*s) s++

static	int		Arg_Long_Fmt	=	0;
static	int		Arg_Unsorted	=	0;
static	int		Arg_Files_Only	=	0;
static	int		Arg_No_Graphics	=	0;
static	int		Arg_Dirs_Only	=	0;
static	int		Arg_List_All	=	0;
static	int		Arg_Num_Cols	=	5;
static	int		Arg_Ext_Sort	=	0;
static	int		Arg_Time_Sort	=	0;
static	int		Arg_Size_Sort	=	0;
static	int		Arg_Exp_Dir		=	1;
static	int		Arg_Mul_Exp		=	0;
static	int		Arg_Debug		=	0;
static	int		Arg_Back_Sort	=	0;
static	int		Arg_Vert_Prt	=	0;

void usage()
{
	fprintf( stderr, "Usage: ls [%c[ABDEFLMNSTUVX][C<num>][Z]] [files]...\n", ARG_Switch );
	fprintf( stderr, "\n");
	fprintf( stderr, "Set the environment variable SWITCHAR to the character\n");
	fprintf( stderr, "you wish to use for the Switch Character\n");
	fprintf( stderr, "\n" );
	fprintf( stderr, "Case of the command line switches %s important\n",
		 ARG_ICase ? "is not" : "is" );
	fprintf( stderr, "\n" );
}

ARG Argtab[] =
{
	{ 'A', ARG_FBOOLEAN,	&Arg_List_All,		"List all files (hidden to)" },
	{ 'B', ARG_FBOOLEAN,	&Arg_Back_Sort,		"Backwards sort (reverse)" },
	{ 'C', ARG_INTEGER,		&Arg_Num_Cols,		"Number of Columns" },
	{ 'D', ARG_FBOOLEAN,	&Arg_Dirs_Only,		"Directories Only" },
	{ 'E', ARG_FBOOLEAN,	&Arg_Ext_Sort,		"Sort by Extension" },
	{ 'F', ARG_FBOOLEAN,	&Arg_Files_Only,	"Files Only" },
	{ 'L', ARG_FBOOLEAN,	&Arg_Long_Fmt,		"Long Format" },
	{ 'M', ARG_FBOOLEAN,	&Arg_Mul_Exp,		"Expand multiple parameters" },
	{ 'N', ARG_FBOOLEAN,	&Arg_No_Graphics,	"Suppress ANSI graphics" },
	{ 'S', ARG_FBOOLEAN,	&Arg_Size_Sort,		"Sort by File Size" },
	{ 'T', ARG_FBOOLEAN,	&Arg_Time_Sort,		"Sort by Date and Time" },
	{ 'U', ARG_FBOOLEAN,	&Arg_Unsorted,		"Suppress Sorting" },
	{ 'V', ARG_FBOOLEAN,	&Arg_Vert_Prt,		"Print Vertically" },
	{ 'X', ARG_TBOOLEAN,	&Arg_Exp_Dir,		"Expand single directories" },
	{ 'Z', ARG_FBOOLEAN,	&Arg_Debug,			"Debug Mode" }
};

#define ARG_TABSIZE ( sizeof(Argtab) / sizeof(ARG) )

static char     *Dirs[MAXDIR];
static char     **Dirv = Dirs;
static int      Dirc = 0;
static long     Total = 0L;
static int      Numfiles = 0;
static int      Numdirs = 0;

static struct FILEINFO *dta_save;
static struct FILEINFO *dta1;

argvcmp( s1p, s2p )
char            **s1p, **s2p;
{
        int ercode;
        D_IN("argvcmp");
        ercode = strcmp( *s1p, *s2p );
        D_OUT("argvcmp");
        return( ercode );
}

scmp( s1p, s2p )
char	**s1p, **s2p;
{
	char	*s1, *s2;
	int	c1,c2;
	int		rval;

	s1 = *s1p;
	s2 = *s2p;

	if(!Arg_No_Graphics && *s1 == Q_ESC)
		for( c1 = 1 ; c1 <= 8 ; s1++, c1++ );

	if(!Arg_No_Graphics && *s2 == Q_ESC)
		for( c2 = 1 ; c2 <= 8 ; s2++, c2++ );

	for( c1 = 1 ; c1 <= 13 ; s1++, c1++ );
	
	for( c2 = 1 ; c2 <= 13 ; s2++, c2++ );

	if( rval = strncmp( s1, s2, 6 ) )
		return rval;

	rval = strcmp( *s1p, *s2p );
	return( rval );
}

tcmp( s1p, s2p )
char	**s1p, **s2p;
{
	char	*s1, *s2;
	int	c1,c2;
	int		rval;

	s1 = *s1p;
	s2 = *s2p;

	if(!Arg_No_Graphics && *s1 == Q_ESC)
		for( c1 = 1 ; c1 <= 8 ; s1++, c1++ );

	if(!Arg_No_Graphics && *s2 == Q_ESC)
		for( c2 = 1 ; c2 <= 8 ; s2++, c2++ );

	for( c1 = 1 ; c1 <= 20 ; s1++, c1++ );
	
	for( c2 = 1 ; c2 <= 20 ; s2++, c2++ );

	rval = strncmp( s1, s2, 17 );
	return( rval * -1 );
}

ecmp( s1p, s2p )
char	**s1p, **s2p;
{
	char	*s1, *s2;
	int	c1,c2;
	int		rval;

	for( s1 = *s1p, c1 = 1 ; *s1 && *s1 != '.' && *s1 != ' ' ; s1++, c1++ );
	
	for( s2 = *s2p, c2 = 1 ; *s2 && *s2 != '.' && *s2 != ' ' ; s2++, c2++ );

	if( rval = strncmp( s1, s2, min(c1,c2 ) ) )
		return rval;

	for( s1 = *s1p, s2 = *s2p ; *s1 == *s2 && *s1 && *s1 != '.' ; s1++, s2++ );

	return( ( *s1 == '.' ? 0 : *s1 ) - ( *s2 == '.' ? 0 : *s2 ) );
}

comp( s1p, s2p )
char	**s1p, **s2p;
{
	int	ercode;
	D_IN("comp");
	if(Arg_Ext_Sort)
	{
		ercode = ecmp( s1p, s2p );
		D_OUT("comp(ecmp)");
		return( Arg_Back_Sort ? ercode * -1 : ercode );
	}
	else
	{
		if(Arg_Time_Sort)
		{
			ercode = tcmp( s1p, s2p );
			D_OUT("comp(tcmp)");
			return( Arg_Back_Sort ? ercode * -1 : ercode );
		}
		else
		{
			if( Arg_Size_Sort )
			{
				ercode = scmp( s1p, s2p );
				D_OUT("comp(ssrt)");
				return( Arg_Back_Sort ? ercode * -1 : ercode );
			}
			else
			{
				ercode = argvcmp( s1p, s2p );
				D_OUT("comp(argvcmp)");
				return( Arg_Back_Sort ? ercode * -1 : ercode );
			}
		}
	}
}


void set_dta( dta )
struct FILEINFO *dta;
{
        struct REGS     regs;

        D_IN("set_dta");
        regs.x.ax = B_GETDTA;
        intdos( &regs, &regs );
        dta_save = (struct FILEINFO *) regs.x.bx;
         
        regs.x.ax = B_SETDTA;
        regs.x.dx = (unsigned int) dta;
        intdos( &regs, &regs );
        D_OUT("set_dta");
}

void reset_dta( )
{
        struct REGS	regs;

        D_IN("reset_dta");
        regs.x.ax = B_SETDTA;
        regs.x.dx = (unsigned int) dta_save;
        intdos( &regs, &regs );
        D_OUT("reset_dta");
}

find_first( filespec, attributes, dta )
char            *filespec;
unsigned int    attributes;
struct FILEINFO *dta;
{
        struct REGS regs;
        int         ercode;

        D_IN("find_first");
        set_dta( dta );
        regs.x.ax = B_FINDFIRST;
        regs.x.dx = (int) filespec;
        regs.x.cx = attributes;
        ercode = intdos( &regs, &regs );
        reset_dta();
        if( ercode == 0 )
        {
                D_S("find_first","dta->fi_name=",dta->fi_name);
                D_PRT("find_first","no error");
        }
        D_OUT("find_first");
        return( ercode );
}

int find_next( dta )
struct FILEINFO *dta;
{
        struct REGS regs;
        int         ercode;

        D_IN("find_next");
        set_dta( dta );
        regs.x.ax = B_FINDNEXT;
        ercode = intdos( &regs, &regs );
        reset_dta();
        if( ercode == 0 )
        {
                D_S("find_next","dta->fi_name=",dta->fi_name);
                D_PRT("find_next","no error");
        }
        D_OUT("find_next");
        return( ercode );
}

dirtoa( s, p )
char            *s;
struct FILEINFO *p;
{
        char    *startstr = s;
        int     i;
         
        D_IN("dirtoa");
        D_S("dirtoa","p->fi_name=",p->fi_name);

        if ( !Arg_No_Graphics && ( IS_LABEL(p) || IS_SUBDIR(p) ) )
                sprintf( s, "%s%s%s", IS_LABEL(p) ? P_REVERSE : P_BOLDFACE,
                        p->fi_name, P_ALL_OFF );
        else
                sprintf( s, "%s", p->fi_name );

        EOS(s);

        if( Arg_Long_Fmt )
        {
                for( i = strlen(p->fi_name) ; i++ < 12 ; *s++ = ' ' );
                sprintf( s, " %6ld ", p->fi_fsize );
                 
                EOS(s);
                sprintf( s, "%2d-%02d-%02d %2d:%02d:%02d",
                        C_YEAR(p)-1900, C_MONTH(p), C_DAY(p),
                        C_HOUR(p), C_MIN(p), C_SEC(p));

                s += 17;
                *s++ = ' ';

                if( IS_READONLY(p) ) { *s++ = 'r'; } else { *s++ = '.'; }
                if( IS_HIDDEN(p) )   { *s++ = 'h'; } else { *s++ = '.'; }
                if( IS_SYSTEM(p) )   { *s++ = 's'; } else { *s++ = '.'; }
                if( IS_LABEL(p) )    { *s++ = 'l'; } else { *s++ = '.'; }
                if( IS_SUBDIR(p) )   { *s++ = 'd'; } else { *s++ = '.'; }
                if( IS_DIRTY(p) )    { *s++ = 'm'; } else { *s++ = '.'; }
                *s = 0;
        }
        D_OUT("dirtoa");
        return( s - startstr );
}


haswild( s )
char            *s;
{
        D_IN("haswild");
        for( ; *s ; s++ )
                if( *s == '*' || *s == '?' )
                {
                        D_S("haswild","filename=","wildcharacters");
                        D_OUT("haswild");
                        return 1;
                }
        D_OUT("haswild");
        return 0;
}

printdir( dirc, dirv )
int             dirc;
char            **dirv;
{
        D_IN("printdir");
        if( !Arg_Unsorted )
                qsort( dirv, dirc, sizeof(*dirv), comp);
        if( Arg_Vert_Prt )
			ptextv( dirc, dirv, Arg_Num_Cols, 79 / Arg_Num_Cols,
				(dirc/Arg_Num_Cols) + (dirc % Arg_Num_Cols != 0) );
		else
			ptexth( dirc, dirv, Arg_Num_Cols, 79 / Arg_Num_Cols,
				(dirc/Arg_Num_Cols) + (dirc % Arg_Num_Cols != 0) );
        D_OUT("printdir");
}

char            *fixup_name( name, dta )
char            *name;
struct FILEINFO *dta;
{
        static char buf[80], *p;



        D_IN("fixup_name");
        if( !find_first( name, A_SUBDIR, dta ) && Arg_Exp_Dir )
        {
                D_S("fixup_name","dta->fi_name=",dta->fi_name);
                if( !IS_SUBDIR(dta) )
                {
                        D_S("fixup_name","newname=",name);
                        D_PRT("fixup_name","no name change");
                        D_OUT("fixup_name");
                        return( name );
                }

                strncpy( buf, name, 80 );
                strncat( buf, "\\*.*", 80 );
                D_S("fixup_name","newname=",buf);
                D_PRT("fixup_name","directory");
                D_OUT("fixup_name");
                return( buf );
        }
        else
        {
                if( name[1] == ':' )
                {
                        buf[0] = name[0];
                        buf[1] = ':';
                         
                        if( !*(name += 2) )
                        {
                                buf[2] = '*';
                                buf[3] = '.';
                                buf[4] = '*';
                                buf[5] = 0;
                                D_S("fixup_name","newname=",buf);
                                D_PRT("fixup_name","disk");
                                D_OUT("fixup_name");
                                return( buf );
                        }
                        else
                                buf[2] = 0;
                }
                 
                for( p = name ; *p ; p++ )
                        if( !( *p == '.' || *p == '\\' || *p == '/' ) )
                        {
                                D_S("fixup_name","newname=",name);
                                D_PRT("fixup_name",". or \\ or /");
                                D_OUT("fixup_name");
                                return( name );
                        }
                 
                strncat( buf, "\\*.*", 80 );
                D_S("fixup_name","newname=",buf);
                D_PRT("fixup_name","fall through");
                D_OUT("fixup_name");
                return( buf );
        }
}

add_entry( dta )
struct FILEINFO *dta;
{
        char            buf[64];
        register int    strlen;

        D_IN("add_entry");
        if( !Arg_List_All && ( IS_HIDDEN(dta) || *dta->fi_name == '.') )
        {
                D_PRT("add_entry","Not /A but hidden/.");
                D_OUT("add_entry");
                return 1;
        }
        
        if( IS_SUBDIR(dta) )
        {
                Numdirs++;
        }
        else if( Arg_Dirs_Only )
        {
                D_PRT("add_entry","only directories");
                D_OUT("add_entry");
                return 1;
        }
        else if( !IS_LABEL(dta) )
        {
                Numfiles++;
        }
        strlen = dirtoa( buf, dta );
        if( ++Dirc > MAXDIR || !( *Dirv = malloc( strlen + 1 ) ) )
        {
                D_PRT("add_entry","too many dirs or malloc");
                D_OUT("add_entry");
                return 0;
        }
        
        strcpy( *Dirv++, buf );
        Total += dta->fi_fsize;
        D_PRT("add_entry","entry added");
        D_OUT("add_entry");
        return 1;
}

main( argc, argv )
int             argc;
char            **argv;
{
        static char     *name = "*.*";

        D_IN("main");
        ctlc();
        ARG_ICase = 1;
        argc = getargs( argc, argv, Argtab, ARG_TABSIZE );

        if( Arg_Time_Sort || Arg_Size_Sort )
			Arg_Long_Fmt = 1;
                 
        if( Arg_Long_Fmt )
			Arg_Num_Cols = 1;
                 
		if( !isatty() )
        	Arg_No_Graphics = 1;
                 
        if( argc >= 2 )
        {
                name = *(++argv);
        		if( argc == 2 && !haswild( name ) && !Arg_Mul_Exp )
					name = fixup_name( name, &dta1 );
        }
        do
        {
        		if( !haswild( name ) && Arg_Mul_Exp )
					name = fixup_name( name, &dta1 );
                if( !find_first( name, Arg_Files_Only ? A_ALL_FILES : A_ALL,
                        &dta1  ) )
                {
                        if( !add_entry( &dta1 ) )
                                break;
                        
                        if( haswild( name ) )
                                while( !find_next( &dta1 ) )
                                        if( !add_entry( &dta1 ) )
                                                goto abort;
                }
                name = *(++argv);
        } while ( --argc > 1 );

abort:  if(Dirc != 0)
			printf("\n");
        printdir( Dirc, Dirs );
        if(Dirc != 0)
        	printf("\n");
        if( Numfiles )
        {
                printf("%d file%s (%ld bytes, %d K)",
                        Numfiles, Numfiles == 1 ? "" : "s",
                        Total, Total/1024 );
        }
        if( Numdirs )
        {
                if( Numfiles )
                        printf(", ");
                printf("%d director%s", Numdirs, Numdirs == 1 ? "y" : "ies" );
        }
        printf("\n");
        D_OUT("main");
}

