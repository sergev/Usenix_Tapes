/*Tcpg - c program source listing formatter */
/*******************************************************************
 *
 *                           cpg.c
 *
 *    DESCRIPTION OF FILE CONTENTS:
 *      C source program listing formatter source.
 *
 *  Cpg provides the facility to print out a C language source file
 *  with headers, nesting level indicators, and table of contents.
 *  It makes use of "triggers" for page headings, titles and
 *  subtitles, and pagination.  It also recognizes function
 *  declarations and form feeds and treats them appropriately.
 *
 *******************************************************************/
/*S includes, defines, and globals */
/*P*/
#include    <stdio.h>
#include    <ctype.h>
#ifdef BSD
#include    <sys/time.h>
#else BSD
#include    <time.h>
#endif 

#define EQ ==
#define NE !=
#define GT >
#define GE >=
#define LT <
#define LE <=
#define OR ||
#define AND &&

#define TRUE 1
#define FALSE 0
#define YES 1
#define NO 0

#define SPACE ' '
#define NUL '\0'

typedef short   BOOL;

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define LINESINHEAD 7
#define MAXWIDTH    130

#define notend(ll) ((ll[0] EQ SLASH AND ll[1] EQ STAR AND ll[2] EQ 'E') ? FALSE : TRUE)
#define SLASH   '/'
#define STAR    '*'
#define DQUOTE '"'
#define SQUOTE '\''
#define BSLASH '\\'

#ifdef BSD
#define strrchr rindex
#define strchr index
#endif BSD

#ifndef VMS
#define delete unlink
#endif

extern char *strrchr ();
extern char *strchr ();

char    *basename ();

char    tim_lin[40];
char    *file_name;
char    fh_name[50] = "";
char    fnc_name[40] = "";
char    subttl[70] = "";
char    title[70] = "";
char    tocname[] = "/tmp/toc_XXXXXX";

int     nlvl = 0;
int	outfd;
int     pageno = 1;
int     LPP = 60;
int     page_line = 67;
int     tabstop = 8;

int     bnflag = FALSE;
int     tocflag = FALSE;
int     incomment = FALSE;
int     insquote = FALSE;
int     indquote = FALSE;

char    specline = FALSE;

FILE    *tocfile,*fd,*dest;

char    *pgm;

char    *ReservedWord[]  = { 
     "auto", "bool", "break", "case", "char", "continue",
     "default", "do", "double", "else", "entry", "enum",
     "extern", "float", "for", "goto", "if",
     "int", "long", "register", "return", "short",
     "sizeof", "static", "struct", "switch",
     "typedef", "union", "unsigned", "void", "while",
     NULL };

/*S main function */
/*Hmain */

main (ac, argv)
int     ac;
char    *argv[];
{
    char    *std_input = "standard_input";  /* input file name      */

    long    cur_time;               /* place for current raw time   */

    long    *time();                /* return raw time from system  */

    register int i;                 /* temporary for indexes, etc.  */

    struct tm *tim;                 /* return from localtime        */
    struct tm *localtime ();

    char    cmdbuf[40];             /* place to format sort command */

    int   optind;                   /* option index                 */

    char *p,*outfile;
    pgm = basename (argv[0]);
    
    if (ac EQ 1)
    {
      fprintf(stderr,"<%s>Usage: %s -b -tn -ln -c file1 ...\n",pgm,pgm);
      exit(1);
    }
    else {
	dest=stdout;
	tocfile=stdout;
	for(optind=1;(optind LT ac) AND (*argv[optind] EQ '-');optind++)
        {
	  p=argv[optind];
          switch(*++p) {
            case 'b':     bnflag = TRUE;
                          break;
            case 'l':     LPP = atoi(++p);
			  if (LPP LE 0)LPP = 999999;
                          break;
            case 't':     tabstop = atoi(++p);
                          break;
            case 'c':     tocflag = TRUE;
                          break;
	    case 'o': 	  outfile = ++p;
 			  if((dest=fopen(outfile,"w"))==NULL){
			    fprintf(stderr,"%s\
: Couldn't open output file '%s'.",outfile);
			    exit(2);
 			  }
			  break;
            default:      exit(2);
          }
      }
    }
    outfd = fileno(dest);
    /* ------------------------------------------------------------ */
    /* set up the date/time portion of page headings                */
    /* ------------------------------------------------------------ */

    time(&cur_time);

    tim = localtime (&cur_time);
    sprintf (tim_lin, "Printed: %02d/%02d/%02d at %2d:%02d %s",
        tim->tm_mon + 1, tim->tm_mday, tim->tm_year,
        tim->tm_hour GT 12 ? tim->tm_hour - 12 : tim->tm_hour,
        tim->tm_min,
        tim->tm_hour GE 12 ? "PM" : "AM" );

    /* ------------------------------------------------------------ */
    /* create the temporary file for the table of contents          */
    /*   don't bother if output is to a terminal                    */
    /* ------------------------------------------------------------ */

    mktemp (tocname);
    if (tocflag)
    {
        tocfile = fopen (tocname, "w");
        if (!tocfile)
        {
            fprintf (stderr, "%s: unable to create tocfile %s\n",
                pgm, tocname);
            exit (2);
        }
    }

    /* ------------------------------------------------------------ */
    /* if no file names, read standard input                        */
    /* ------------------------------------------------------------ */

    if (optind EQ ac)
    {
        fd = stdin;
        file_name = std_input;
        strcpy (fh_name,file_name);
        dofile (fd);
        fclose(fd);
    }
    else
    {
    /* ------------------------------------------------------------ */
    /* process each file named on the command line                  */
    /* ------------------------------------------------------------ */

        for (i = optind; i LT ac; i++)
        {
    /* ------------------------------------------------------------ */
    /* special file name `-' is standard input                      */
    /* ------------------------------------------------------------ */

            if (strcmp (argv[i], "-") EQ 0)
            {
                fd = stdin;
                file_name = std_input;
                strcpy (fh_name,file_name);
            }
            else
            {
                file_name = argv[i];
                fd = fopen (argv[i], "r");
                if (bnflag) strcpy (fh_name,basename(argv[i]));
                else strcpy (fh_name,argv[i]);
                if (fd EQ NULL)
                {
                    fprintf (stderr,
                        "cpg: unable to open %s\n", argv[i]);
                }
            }
            if (fd NE NULL)
            {
                dofile (fd);
                fclose (fd);
            }
        }
    }

    fflush (dest);

    /* ------------------------------------------------------------ */
    /* sort and print the table of contents - straight alpha order  */
    /* on function and file name                                    */
    /* ------------------------------------------------------------ */

    if (tocflag)
    {
        fclose (tocfile);
        sprintf (cmdbuf, "sort +1 -2 +0 -1 -u -o %s %s", tocname, tocname);
        system (cmdbuf);
        tocfile = fopen (tocname, "r");
        if (!tocfile)
        {
            fprintf (stderr, "%s: unable to read tocfile\n", pgm);
            exit (2);
        }
        else
        {
            tocout (tocfile);
            fclose (tocfile);
            delete (tocname);
        }
    }

    fprintf (dest, "\f");

    exit (1);
}
/*Sdofile - process an input file */
/*Hdofile*/
dofile (ifd)
FILE    *ifd;
{

    int     lineno = 1;             /* line number in current file  */

    register char *line;            /* current line pointer         */

    char    ibuf[MAXWIDTH];         /* original input line          */
    char    ebuf[MAXWIDTH];         /* line with tabs expanded      */

    /* ------------------------------------------------------------ */
    /* initialize the function name to `.' - unknown                */
    /* ------------------------------------------------------------ */

    strcpy (fnc_name, ".");

    /* ------------------------------------------------------------ */
    /* if building TOC, add this entry                              */
    /* ------------------------------------------------------------ */

    if (tocflag)
        fprintf (tocfile,
            "%s %s %d %d\n", fh_name, fnc_name, pageno, lineno);

    /* ------------------------------------------------------------ */
    /* if tabs are to be expanded, use the expansion buffer         */
    /* ------------------------------------------------------------ */

    if (tabstop) line = ebuf;
    else         line = ibuf;

    /* ------------------------------------------------------------ */
    /* process each line in the file, looking for triggers          */
    /* ------------------------------------------------------------ */

    while (fgets (ibuf, MAXWIDTH, ifd) NE NULL)
    {
    /* ------------------------------------------------------------ */
    /* expand the input line                                        */
    /* ------------------------------------------------------------ */

        expand (ebuf, ibuf);

        if (line[0] EQ SLASH AND line[1] EQ STAR)
        {
    /* ------------------------------------------------------------ */
    /* comment found - could be a trigger                           */
    /* ------------------------------------------------------------ */

            switch (line[2])
            {
                case 'F':
                case 'H':
                {
                    header (&lineno, line);
                    break;
                }
                case 'P':
                {
                    print_head ();
                    break;
                }
                case 'S':
                {
                    nameget (line, subttl);
                    break;
                }
                case 'T':
                {
                    nameget (line, title);
                    break;
                }
                default:
                {
                    print (&lineno, line);
		    lineno--;
                    break;
                }
            }
        lineno++;
        }
        else
        {
    /* ------------------------------------------------------------ */
    /* not a comment - check for function declaration               */
    /* if a form feed is found, start a new page with header        */
    /* ------------------------------------------------------------ */

            if (!nlvl AND  ckfunc (lineno, line)) print_head();
            if (*line EQ '\f') print_head ();
            else print (&lineno, line);
        }
    }

    page_line = LPP+1;      /* force new page after file            */
    title[0] = NUL;         /* clear title and subtitle             */
    subttl[0] = NUL;

    return;
}
/*Sheader - construct and print header box */
header  (lineno, line)
register int     *lineno;
register char    *line;
{

    if (line[2] EQ 'F')
    {
        nameget (line, fh_name);
        if (bnflag) strcpy (fh_name, basename (fh_name));
        strcpy (fnc_name, ".");
    }
    else if (line[2] EQ 'H')
    {
        nameget (line, fnc_name);
    }

    if (tocflag)
        fprintf (tocfile,
            "%s %s %d %d\n", fh_name, fnc_name, pageno, *lineno);

    print_head ();

    return;
}
/*Snameget - get a string from a signal line */
nameget (line, name)
register char    *line;
register char    *name;
{
    register int     i;
    register int     j;

    /* ------------------------------------------------------------ */
    /* skip leading spaces in the trigger line                      */
    /* copy up to trailing asterisk or end-of-line                  */
    /* strip trailing spaces                                        */
    /* ------------------------------------------------------------ */

    for (i = 3; isspace(line[i]) AND i LT MAXWIDTH; i++);

    for (j = 0; line[i] AND line[i] NE '*'; i++, j++)
    {
        name[j] = line[i];
    }

    while (j-- GT 0 AND isspace (name[j]));

    name[++j] = NUL;

    return;
}
/*Sprint - print a line with line number */
print (lineno, line)
register int     *lineno;
register char    *line;
{
    register int llen = strlen (line);
    register int i;

    register char sc = specline ? '*' : ' ';

    int     j = 0;

    register char    dc = NUL;

    /* ------------------------------------------------------------ */
    /* new page with header if page length is exceeded              */
    /* ------------------------------------------------------------ */

    if (page_line GT LPP)
    {
        print_head ();
    }

    /* ------------------------------------------------------------ */
    /* if brace(s) found,                                           */
    /*   modify the nesting level by the next nesting delta          */
    /*   select the indicator according to the next delta            */
    /*   if nexting is back to zero (none), clear function name     */
    /* ------------------------------------------------------------ */

    if (fnd (line, &j))
    {
        nlvl += j;

        if (j LT 0) dc = '<';
        else if (j EQ 0) dc = '*';
        else dc = '>';

        i = nlvl;
        if (j LT 0) i++;
        fprintf (dest, "%4d%c%2d%c ",
            (*lineno)++, sc, i, dc);
        if (nlvl EQ 0) strcpy (fnc_name, ".");
    }
    else
    {
        fprintf (dest, "%4d%c    ", (*lineno)++, sc);
    }

    /* ------------------------------------------------------------ */
    /* break up long lines by finding the first space from the end  */
    /* ------------------------------------------------------------ */

    if (llen GT 71)
    {
        for (i = 70; i GE 0; i--)
        {
            if (line[i] EQ SPACE)
            {
                fprintf (dest, "%*.*s \\\n", i, i, line);
                page_line++;
                break;
            }
        }

        j = 79 - (llen - i);

        for (j; j GE 0; j--) putc (SPACE, dest);

        fprintf (dest, "%s", &line[i+1]);
    }
    else
    {
        fprintf (dest, "%s", line);
    }

    page_line++;

    specline = FALSE;       /* true if function declaration     */

    return;
}
/*Sprint_head - print the page heading with page number */
print_head ()
{
    char    headbuf[80];
    register int len;

    sprintf (headbuf, "[ %s | %s <- %s",
        tim_lin, fh_name, fnc_name);

    for (len = strlen (headbuf); len LT 68; len++) headbuf[len] = SPACE;

    sprintf (&headbuf[68], "Page %-4d ]", pageno++);
    fprintf (dest, "\f\n");
    if (!isatty(outfd))
        fprintf (dest, "_______________________________________\
________________________________________");
    fprintf (dest, "\n%s\n", headbuf);
    fprintf (dest, "[-------------------------------+------\
---------------------------------------]\n");

    if (*title)
    {
        sprintf (headbuf, "[    %s", title);
    }
    else
    {
        sprintf (headbuf, "[    %s", fh_name);
    }
    for (len = strlen (headbuf); len LT 78; len++) headbuf[len] = SPACE;
    headbuf[78] = ']';
    fprintf (dest, "%s\n", headbuf);

    if (*subttl)
    {
        sprintf (headbuf, "[    %s", subttl);
    }
    else
    {
        sprintf (headbuf, "[    %s", fnc_name);
    }
    for (len = strlen (headbuf); len LT 78; len++) headbuf[len] = SPACE;
    headbuf[78] = ']';
    fprintf (dest, "%s", headbuf);

    if (!isatty(outfd))
        fprintf (dest, "\r_______________________________________\
________________________________________");
    fprintf (dest, "\n\n");

    page_line = LINESINHEAD;

    return;
}
/*S fnd - return true if a brace is found */
fnd (in, nchg)
register char *in;
register int    *nchg;
{
#   define LBRACE   '{'
#   define RBRACE   '}'
#   define SHARP    '#'
#   define COLON    ':'

    register found = FALSE;         /* true if keyword found        */

    register char blank = TRUE;     /* used to check for shell/make */
                                    /* comments beginning with #/:  */
    register int inshcomment = FALSE;   /* true if in shell comment */

    *nchg = 0;              /* initialize net change to zero        */

    /* ------------------------------------------------------------ */
    /* check each character of the line                             */
    /* ------------------------------------------------------------ */

    for (in; *in; in++)
    {
        if (!incomment AND !inshcomment AND !indquote AND !insquote)
        {
            if (*in EQ SLASH AND *(in+1) EQ STAR)
            {
                incomment = TRUE;
                blank = FALSE;
            }
            else if (blank AND
                     ((*in EQ SHARP OR *in EQ COLON) AND
                     (*(in+1) NE LBRACE AND *(in+1) NE RBRACE))
                    )
            {
                inshcomment = TRUE;
                blank = FALSE;
            }
            else if (*in EQ DQUOTE AND
                    (*(in-1) NE BSLASH OR *(in-2) EQ BSLASH))
            {
                indquote = TRUE;
                blank = FALSE;
            }
            else if (*in EQ SQUOTE AND
                    (*(in-1) NE BSLASH OR *(in-2) EQ BSLASH))
            {
                insquote = TRUE;
                blank = FALSE;
            }
            else if (*in EQ LBRACE)
            {
                (*nchg)++;
                found = TRUE;
                blank = FALSE;
            }
            else if (*in EQ RBRACE)
            {
                (*nchg)--;
                found = TRUE;
                blank = FALSE;
            }
            else if (!isspace (*in))
            {
                blank = FALSE;
            }
        }
        else if (incomment AND *in EQ STAR AND *(in+1) EQ SLASH)
            incomment = FALSE;
        else if (indquote AND *in EQ DQUOTE AND
                (*(in-1) NE BSLASH OR *(in-2) EQ BSLASH))
            indquote = FALSE;
        else if (insquote AND *in EQ SQUOTE AND
                (*(in-1) NE BSLASH OR *(in-2) EQ BSLASH))
            insquote = FALSE;
    }
    
    return found;
}
/*Stocout - print out the table of contents */
tocout (toc)
FILE    *toc;
{
    char    filenam[80];
    char    fncnam[80];
    int     page;
    int     line;

    register int toclines = LPP + 1;

    while (fscanf (toc, "%s%s%d%d", filenam, fncnam, &page, &line) EQ 4)
    {
        if (toclines GT LPP-7)
        {
	  fprintf(dest,"\f\n\n");
          if (!isatty(outfd))  fprintf (dest,"\r\
                             _____________________");
	  fprintf(dest,"\n\
                             [ TABLE OF CONTENTS ]");
	  if (!isatty(outfd)) fprintf (dest,"\r\
                             _____________________");
	  fprintf(dest,"\n\n\
                File -> Function                     Page    Line");
	  if (!isatty(outfd)) fprintf (dest,"\r\
________________________________________\
________________________________________");
	  fprintf(dest,"\n\n");
            toclines = 0;
        }

        toclines++;

        fprintf (dest,"\
    %16s -> %-16.16s ............ %3d   %5d\n",
            filenam, *fncnam EQ '.' ? "START" : fncnam, page, line);
    }
    return;
}
/*S expand - expand tabs to tabstop */
expand (to, from)
register char *to;
register char *from;
{
    register int i;
    register int tofill;

#   define BACKSPACE '\b'
#   define FORMFEED '\f'
#   define NEWLINE '\n'
#   define RETURN '\r'
#   define TAB '\t'
    
    i = 0;

    while (*from)
    {
        switch (*from)
        {
            case    TAB:
                tofill = tabstop - (i % tabstop);
                i += tofill;
                while (tofill--) *(to++) = SPACE;
                break;
            case    NEWLINE:
            case    RETURN:
                i = 0;
            case    FORMFEED:
                *(to++) = *from;
                break;
            case    BACKSPACE:
                i--;
                *(to++) = *from;
                break;
            default:
                i++;
                *(to++) = *from;
                break;
        }

        from++;
    }

    *to = NUL;

    return;
}
/*S ckfunc - check line for function declaration */

#define isidchr(c) (isalnum(c) || (c == '_'))

ckfunc (lineno, s)
register int lineno;
register char   *s;
{
    register char *p;
    register int  i;
    register char found = FALSE;

    char FunctionName[40];

    if(!strcmp (fnc_name, ".") AND !incomment && !indquote && !insquote)
    {
        found = TRUE;
        while (found)
        {
		p = FunctionName;
            found = FALSE;
            for (s; isascii (*s) && isspace (*s) && *s; s++);
            if( *s == '*' )
            {
                for (++s; isascii (*s) && isspace (*s) && *s; s++);
            }

            if ((*s == '_') || isalpha(*s))
            {
                while (isidchr (*s)) *p++ = *s++;

                *p = '\0';

                for (found = FALSE, i = 0;
                     !found AND ReservedWord[i]; i++)
                {
                    if (strcmp (FunctionName, ReservedWord[i]))
                        found = TRUE;

                }
            }
        }

        for (s; isascii (*s) && isspace (*s) && *s; s++);

        if (*s EQ '(')
        {
            for (found = FALSE; *s AND !found; s++)
                found = *s EQ ')';
            
            if (found)
            {
                for (; *s AND isspace (*s); s++);

                found = (*s NE ';') AND (*s NE ',');
                
                if (found)
                {
                    strcpy (fnc_name, FunctionName);
                    fprintf (tocfile,
                        "%s %s %d %d\n",
                        fh_name, fnc_name, pageno-1, lineno);
                    specline = TRUE;
                }
            }
        }
    }
    return found;
}
/*S basename - return the basename part of a pathname */
/*******************************************************************
*
*                                   basename
*
*  given a (presumed) pathname, return the part after the last slash
*
*********************************************************************/
char *basename (str)
register char *str;
{
    register char *ret,*mid;

    if (ret = strrchr (str, '/')) ret++;
    else if (ret = strrchr (str, ']')) ret++;
    else if (ret = strrchr (str, ':')) ret++;
    else ret = str;
    if(mid = strchr(ret,';')) 
	*mid='\0';

    return ret;
}
