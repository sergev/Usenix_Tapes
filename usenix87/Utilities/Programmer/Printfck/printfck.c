/* printfck.c - check all uses of %d, %ld, %s, %u etc. - 850325 aeb@mcvax*/
/* small fixes, made more robust, process cmdline arg - 850402 guido@boring*/
/* Copyright 1985,1986 Stichting Mathematisch Centrum. Use at own risk. */
/* $Header: printfck.c,v 1.3 86/05/30 12:31:42 liam Exp $
 * $Log:        printfck.c,v $
 * Revision 1.3  86/05/30  12:31:42  liam
 * Added facility to recognise scanf formats as well.
 *
 */

#include        <stdio.h>
#include        <strings.h>

#include "printfck.h"

/* Feed with a list of routine names and descriptions:
 *      printf("",...)
 *      sprintf(s,"",...)
 *      fprintf(f,"",...)
 * and with a source file; produce output in which occurrences of e.g.
 *      sprintf(buf, "%s%ld", s, l)
 * are replaced by
 *      sprintf(buf, "%s%ld", percent_s(s), percent_L(l))
 * Now let lint do the checking.
 * Bugs:
 *      Cases where the format string is not explicitly given (e.g., is the
 *      result of some other routine, or looks like  bool ? "s1" : "s2")
 *      are not handled.
 *      Cases where the preprocessor produces quotes or comment delimiters
 *      or concatenates partial identifiers are not handled.
 *      We do not distinguish two sets of identifiers.
 *      Only the parts lint sees get checked - not parts between (false)
 *      #ifdef's. If the call to printf is outside #ifdef's, but some
 *      args are inside, printfck may get confused. However, this is easy
 *      to avoid:
 *
 *      THIS FAILS                      THIS WORKS
 *      ----------                      ----------
 *              printf("%s%d",                  printf("%s%d", (
 *      #ifdef debug                    #ifdef debug
 *                      "foo"                           "foo"
 *      #else                           #else
 *                      "bar"                           "bar"
 *      #endif debug                    #endif debug
 *                      , num);                         ), num);
 *
 */

char *index();
char *rindex();
char *malloc();

#define MAXIRS 100

struct ir {
        char *rname;
        int pn;         /* number of args preceding format string */
        int type;       /* 0 = printf, 1 = scanf */
} irs[MAXIRS+1] = {     /* should be read in - for now explicit */
        "printf",       0,  0,
        "fprintf",      1,  0,
        "sprintf",      1,  0,
        "scanf",        0,  1,
        "fscanf",       1,  1,
        "sscanf",       1,  1,
};

int nirs;

char *progname;
char *filename = NULL;
FILE *inp;

int eof;
int peekc;
int lastc;
int linenr;

initgetcx()
{
        eof = 0;
        peekc = '\n';   /* recognize # on very first line */
        lastc = 0;      /* result of last getchar() */
        linenr = 1;
}

getcx()
{
        register int c;

        if(peekc) {
                c = peekc;
                peekc = 0;
        } else if(eof) {
                c = EOF;
        } else {
                if(lastc) {
                        putchar(lastc);
                        lastc = 0;
                }
                if((c = getc(inp)) == EOF)
                        eof++;
                else {
                        lastc = c;
                        if(c == '\n')
                                linenr++;
                }
        }

        return(c);
}

/* Note: we do not want to eliminate comments; perhaps they contain
   lint directives. */
getcy()         /* as getcx(), but skip comments */
{
        register int c = getcx();

        if(c == '/') {
                c = getcx();
                if(c == '*') {
                        while(1) {
                                c = getcx();
                                if(c == EOF)
                                        error("unfinished comment");
                                while(c == '*') {
                                        c = getcx();
                                        if(c == '/')
                                                return(getcy());
                                }
                        }
                } else {
                        peekc = c;
                        c = '/';
                }
        }
        return(c);
}

getcz()         /* as getcy(), but skip preprocessor directives */
{
        register int c = getcy();

        while(c == '\n') {
                c = getcx();
                if(c == '#') {
                        while(c != '\n') {
                                c = getcx();
                                if(c == EOF)
                                        error("incomplete line");
                                while(c == '\\') {
                                        (void) getcx(); c = getcx();
                                }
                        }
                } else {
                        peekc = c;
                        return('\n');
                }
        }
        return(c);
}

getcq()         /* as getcz() but skip strings */
{
        register int c = getcz();
        register int delim;

        if(c == '\'' || c == '"') {
                delim = c;
                while(1) {
                        c = getcx();
                        if(c == '\n' || c == EOF)
                                error("Unfinished string (delim %c)", delim);
                        if(c == '\\') {
                                (void) getcx();
                                continue;
                        }
                        if(c == delim)
                                return(getcq());
                }
        }
        return(c);
}

usage()
{
        fprintf(stderr,
          "Usage: %s [-n] [-e function] ... [-f datafile] ... [file.c] ...\n",
          progname);
        exit(2);
}

extern char *optarg;
extern int optind;

main(argc, argv)
int argc;
char **argv;
{
        register int c;
        FILE *fp;

        if (argc > 0) {
                progname = rindex(argv[0], '/');
                if (progname != NULL)
                        ++progname;
                else
                        progname = argv[0];
        }

        for (; irs[nirs].rname != NULL; ) ++nirs; /* Count defaults */

        while ((c = getopt(argc, argv, "e:nf:")) != EOF) {
                switch (c) {
                case '?':
                        usage();
                        /* NOTREACHED */
                case 'e':
                        addir(optarg, 0, 0);
                        break;
                case 'n':
                        nirs = 0;
                        irs[nirs].rname = NULL;
                        break;
                case 'f':
                        getirfile(optarg);
                        break;
                }
        }

        /* tell lint the types of the percent_* routines */

        printf("#include \"%s\"\n", PERCENT_HEADERS);

        /* now process them files.... */

        if (optind == argc)
                treat(stdin, "stdin");
        else {
                for (; optind < argc; ++optind) {
                        if (strcmp(argv[optind], "-") == 0)
                                treat(stdin, "stdin");
                        else {
                                filename = argv[optind];
                                fp = fopen(filename, "r");
                                if (fp == NULL)
                                        filerror(filename);
                                treat(fp, filename);
                                fclose(fp);
                        }
                }
        }

        /* now include the bodies of the routines */

        printf("#include \"%s\"\n", PERCENT_ROUTINES);

        exit(0);
}

treat(fp, file)
FILE *fp;
char *file;
{
        register int c;

        filename = file;
        linenr = 0;
        inp = fp;
        printf("# line 1 \"%s\"\n", file);

        initgetcx();

        while((c = getcq()) != EOF) {

                /* check for (interesting) identifiers */
                if(letter(c))
                        rd_id(c);
        }
        filename = NULL;
        linenr = 0;
        irs[nirs].rname = NULL;
}


rd_id(first)
register int first;
{
        char idf[256];
        register char *ip = idf;
        register int c;

        *ip++ = first;
        while(letdig(c = getcx()))
                if(ip-idf < sizeof(idf)-1)
                        *ip++ = c;
        peekc = c;
        *ip = 0;
        handle(idf);
}

/*VARARGS1*/
error(s, x)
char *s;
{
        printf("\n"); /* Finish incomplete output line */
        fprintf(stderr, "%s: Error (", progname);
        if (filename != NULL) fprintf(stderr, "%s, ", filename);
        fprintf(stderr, "line %d): ", linenr);
        fprintf(stderr, s, x);
        fprintf(stderr, "\n");
        exit(1);
}

/*VARARGS1*/
warning(s, x1, x2)
char *s;
{
        fprintf(stderr, "%s: Warning (", progname);
        if (filename != NULL) fprintf(stderr, "%s, ", filename);
        fprintf(stderr, "line %d): ", linenr);
        fprintf(stderr, s, x1, x2);
        fprintf(stderr, "\n");
}

filerror(file)
char *file;
{
        fprintf(stderr, "%s: can't open ", progname);
        perror(file);
        exit(2);
}

letter(c)
register int c;
{
        return(('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_');
}

digit(c)
register int c;
{
        return('0' <= c && c <= '9');
}

letdig(c)
register int c;
{
        return(letter(c) || digit(c));
}

handle(idf)
register char *idf;
{
        register struct ir *irp = irs;

        while(irp->rname) {
                if(!strcmp(idf, irp->rname)) {
                        doit(irp);
                        return;
                }
                irp++;
        }
}

skipspaces()
{
        register int c;

        while(1) {
                c = getcz();
                if(c == ' ' || c == '\t' || c == '\n')
                        continue;
                peekc = c;
                return;
        }
}

doit(irp)
register struct ir *irp;
{
        register int c, cnt = irp->pn;

        skipspaces();
        if((c = getcz()) != '(') {
                peekc = c;
                warning("%s not followed by '('", irp->rname);
                return;
        }

        while(cnt--) {
                c = skiparg();
                if(c != ',') {
                        peekc = c;
                        warning("arg of %s not followed by comma", irp->rname);
                        return;
                }
        }
        skipspaces();

        /* now parse format string (if present) */
        /* (here we also avoid defining occurrences) */
        if((c = getcx()) != '"') {
                peekc = c;
                return;
        }
        if (irp->type == 0)
                do_printf(irp);
        else
                do_scanf(irp);
}

do_printf(irp)
register struct ir *irp;
{
        char fmt[256];
        register char *fp = fmt;
        register int c;

        while(1) {
                c = getcx();
                if(c == '\n' || c == EOF)
                        error("Unfinished format string");
                if(c == '"')
                        break;
                if(c != '%') {
                        if (c == '\\')
                                (void) getcx();
                        continue;
                }
                c = getcx();
                if(c == '%')
                        continue;
                if(c == '-')
                        c = getcx();
                if(c == '*') {
                        c = getcx();
                        if(fp-fmt < sizeof(fmt)-1)
                                *fp++ = '*';
                } else while(digit(c))
                        c = getcx();
                if(c == '.')
                        c = getcx();
                if(c == '*') {
                        c = getcx();
                        if(fp-fmt < sizeof(fmt)-1)
                                *fp++ = '*';
                } else while(digit(c))
                        c = getcx();
                if(c == '#')
                        c = getcx();
                if(c == 'l') {
                        c = getcx();
                        if('a' <= c && c <= 'z')
                                c -= 'a'-'A';
                        else
                                error("%%l not followed by lowercase");
                }
                if(fp-fmt < sizeof(fmt)-1)
                        *fp++ = c;
                else
                        warning("ridiculously long format");
        }
        *fp = 0;
        fp = fmt;
        skipspaces();
        while((c = getcz()) == ',') {
                if(!*fp)
                        error("%s has too many arguments", irp->rname);
                skipspaces();
                if (*fp == '*') {
                        printf("percent_star(");
                        fp++;
                } else
                        printf("percent_%c(", *fp++);
                c = skiparg();
                printf(")");
                peekc = c;
        }
        if(c != ')')
                error("%s has ill-formed argument list", irp->rname);
        if(*fp)
                error("%s has too few arguments", irp->rname);
}


do_scanf(irp)
register struct ir *irp;
{
        char fmt[256], shorten;
        register char *fp = fmt;
        register int c;
        int dummy = 0;

        while(1) {
                dummy = 0;
                c = getcx();
                if(c == '\n' || c == EOF)
                        error("Unfinished format string");
                if(c == '"')
                        break;
                if(c != '%') {
                        if (c == '\\')
                                (void) getcx();
                        continue;
                }
                c = getcx();
                if(c == '%')
                        continue;       /* %% */

                if(c == '*') {
                        dummy = 1;      /* no corresponding argument */
                        c = getcx();
                }
                while (digit(c) || c == '.' || c == '-') {
                        c = getcx();
                }
                switch(c) {
                case '[':
                        c = skipbracket();      /* scan to close bracket */
                        break;
                case 'l':
                        c = getcx();
                        if('a' <= c && c <= 'z')
                                c -= 'a'-'A';
                        else
                                error("%%l not followed by lowercase");
                        break;
                case 'h':
                        shorten = c;
                        c = getcx();
                        if('a' <= c && c <= 'z')
                                c -= 'a'-'A';
                        else
                                error("%%h not followed by lowercase");
                        break;
                default:
                        /* must be a format letter */
                        break;
                }
                if (!dummy) {
                        if(fp-fmt < sizeof(fmt)-2) {
                               if (shorten == 'h')
                                        *fp++ = 'h';
                                *fp++ = c;
                        } else
                                warning("ridiculously long format");
                }
        }
        *fp = 0;
        fp = fmt;
        skipspaces();
        while((c = getcz()) == ',') {
                if(!*fp)
                        error("%s has too many arguments", irp->rname);
                skipspaces();
                if ( (shorten = *fp) == 'h')
                        fp++;
                switch (*fp) {
                    case 's':  printf("percent_s(");   break;
                    case ']':  printf("percent_bkt("); break;
                    default:
                        if (shorten == 'h')
                                printf("percent_h%c_ptr(", *fp);
                        else
                                printf("percent_%c_ptr(", *fp);
                }
                fp++;
                c = skiparg();
                printf(")");
                peekc = c;
        }
        if(c != ')')
                error("%s has ill-formed argument list", irp->rname);
        if(*fp)
                error("%s has too few arguments", irp->rname);
}

skiparg()
{
        register int parenct = 0;
        register int c;

        parenct = 0;
        while(1) {
                c = getcq();
                if(c == EOF)
                        error("eof in arg list");
                if(!parenct && (c == ',' || c == ')'))
                        return(c);
                if(c == '(' || c == '[') {
                        parenct++;
                        continue;
                }
                if(c == ')' || c == ']') {
                        parenct--;
                        continue;
                }
        }
}

skipbracket()
{
        register int c;

        while(1) {
                c = getcx();
                if(c == EOF)
                        error("eof during %%[ ]");
                if(c == '\\') {
                        c = getcx();
                        continue;       /* avoid escaped char */
                }
                if(c == ']') return c;
        }
}

char *strsave(s)
char *s;
{
        char *t = malloc(strlen(s) + 1);
        if (t == NULL) error("out of memory");
        strcpy(t, s);
        return t;
}

getirfile(irfile)
char *irfile;
{
        FILE *fp = fopen(irfile, "r");
        char line[256];
        char name[256];
        char *end;
        int n;
        int cnt, type;

        if (fp == NULL) filerror(irfile);
        filename = irfile;
        linenr = 0;
        while (fgets(line, sizeof line, fp)) {
                ++linenr;
                end = index(line, '#');
                if (end) *end = '\0';
                n= sscanf(line, " %s %d %d %s", name, &cnt, &type, name+1);
                if (n == 0 || name[0] == '\0')
                        continue; /* Skip empty line or comment */
                if (n != 3 || cnt < 0 || (type != 0 && type != 1) )
                        error("bad format (must be %%s %%u %%u)");
                /* Should also check for valid name... */
                addir(strsave(name), cnt, type);
        }
        fclose(fp);
        filename = NULL;
        linenr = 0;
}

addir(name, cnt, type)
char *name;
int cnt, type;
{
        if (nirs >= MAXIRS) error("table overflow");
        irs[nirs].rname = name;
        irs[nirs].pn = cnt;
        irs[nirs].type = type;
        ++nirs;
}

/*
 * get option letter from argument vector
 */
int     opterr = 1,             /* useless, never set or used */
        optind = 1,             /* index into parent argv vector */
        optopt;                 /* character checked for validity */
char    *optarg;                /* argument associated with option */

#define BADCH   (int)'?'
#define EMSG    ""
#define tell(s) fputs(*nargv,stderr);fputs(s,stderr); \
                fputc(optopt,stderr);fputc('\n',stderr);return(BADCH);

getopt(nargc,nargv,ostr)
int     nargc;
char    **nargv,
        *ostr;
{
        static char     *place = EMSG;  /* option letter processing */
        register char   *oli;           /* option letter list index */
        char    *index();

        if(!*place) {                   /* update scanning pointer */
                if(optind >= nargc || *(place = nargv[optind]) != '-' || !*++place) return(EOF);
                if (*place == '-') {    /* found "--" */
                        ++optind;
                        return(EOF);
                }
        }                               /* option letter okay? */
        if ((optopt = (int)*place++) == (int)':' || !(oli = index(ostr,optopt))) {
                if(!*place) ++optind;
                tell(": illegal option -- ");
        }
        if (*++oli != ':') {            /* don't need argument */
                optarg = NULL;
                if (!*place) ++optind;
        }
        else {                          /* need an argument */
                if (*place) optarg = place;     /* no white space */
                else if (nargc <= ++optind) {   /* no arg */
                        place = EMSG;
                        tell(": option requires an argument -- ");
                }
                else optarg = nargv[optind];    /* white space */
                place = EMSG;
                ++optind;
        }
        return(optopt);                 /* dump back option letter */
}

