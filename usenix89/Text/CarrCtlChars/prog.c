
#include <stdio.h>
#include <ctype.h>
#include <bool.h>

#define SH_TRUE		1
#define SH_FALSE	0

#define NEWPAGE		'1'
#define OVRSTRK		'+'
#define SNG_SPC		' '
#define DBL_SPC		'0'
#define TRP_SPC		'-'
#define TABLENGTH	8
#define PAGELENGTH	66

char	*pname;
int	tablength = TABLENGTH,
	pglength  = PAGELENGTH,
	cctrl;

usage()
{
    fprintf(stderr,"usage: %s [-t#] [ file1 [ file2 .. filen ] ]\n",
	    pname);
    fprintf(stderr,"\t-t#  -- tabstops at multiples of #\n");
    exit(SH_FALSE);
}

main(argc,argv)
int	argc;
char	**argv;
{
    extern char	*optarg;
    extern int	optind, opterr;
    FILE	*f;
    int		errflg = FALSE,
		tmp,
		c;
    char	*sp;

    pname=*argv;

    while ( (c=getopt(argc,argv,"l:t:")) != EOF )
	switch(c) {
	    case 'l':	errflg=(errflg || sscanf(optarg,"%d",&tmp)!=1 || tmp<0);
			if (! errflg) {
			    pglength = tmp;
			    while(isdigit(*optarg))	/* kludgy stuff */
				optarg++;
			    if (*optarg != '\0') {	/* kludgy stuff */
				argv[--optind]  = optarg - 1;
				argv[optind][0] = '-';
			    }
			}
			break;
	    case 't':	errflg=(errflg || sscanf(optarg,"%d",&tmp)!=1 || tmp<0);
			if (! errflg) {
			    tablength = tmp;
			    while(isdigit(*optarg))	/* kludgy stuff */
				optarg++;
			    if (*optarg != '\0') {	/* kludgy stuff */
				argv[--optind]  = optarg - 1;
				argv[optind][0] = '-';
			    }
			}
			break;
	    default:	errflg = TRUE;
			break;
	}

    argc -= optind;
    argv = &(argv[optind]);

    if (errflg)
	usage();

    if (argc==0) {
	cctrl = NEWPAGE;
	mkcc(stdin);
    } else {
	while (argc--) {
	    if ((f=fopen(*argv,"r")) == NULL) {
		fprintf(stderr,"%s: cannot open file \"%s\".\n",pname,*argv);
		exit(SH_FALSE);
	    } else {
		cctrl = NEWPAGE;
		mkcc(f);
		fclose(f);
	    }
	    argv++;
	}
    }
    exit(SH_TRUE);
}

/**********************************************************************
 *                                                                    *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

#define max(x,y)	((x)>(y) ? (x) : (y))

#define BUFCOLS	2048
#define BUFROWS	  16

static char	buffer[BUFROWS][BUFCOLS];
static int	lines,
		width[BUFROWS],
		bufc,
		bufr[BUFCOLS],
		buftouched;

mkcc(f)
FILE *f;
{
    int	c;

    initbuf();

    while ((c=getc(f))!= EOF) {
	buftouched = TRUE;
	switch(c) {
	    case '\r' :	printout();
			cctrl = OVRSTRK;
			break;
	    case '\f' :	printout();
			cctrl = NEWPAGE;
			break;
	    case '\n' :	printout();
			cctrl = SNG_SPC;
			break;
	    case '\b' :	bufc--;
			break;
	    case ' ' :	bufc++;
			break;
	    case '\t' : if (tablength > 0)
			    bufc += tablength - (bufc % tablength);
			break;
	    default:	if isprint(c)  {
			    buffer[bufr[bufc]][bufc]=c;
			    width[bufr[bufc]]
				=max(width[bufr[bufc]],bufc+1);
			    bufr[bufc]++;
			    lines=max(lines,bufr[bufc]);
			    bufc++;
			} else {
			    fprintf(stderr,
				"%s: can't handle control characters.\n",
				pname);
			    exit(SH_FALSE);
			}
			break;
	}
    }
    printout();
}

printout()
{
    int	i,
	j;

    if (buftouched) {
	putchar(cctrl);
	for (i=0; i<lines; i++) {
	    if (i)
		printf("\n%c",OVRSTRK);
	    for (j=0; j<width[i]; j++)
		if (i<bufr[j])
		    putchar(buffer[i][j]);
		else
		    putchar(' ');
	}
	initbuf();
	putchar('\n');
    }
}


initbuf()
{
    for (bufc=0; bufc<BUFCOLS; bufc++)
	bufr[bufc] = 0;
    for (lines=0; lines<BUFROWS; lines++)
        width[lines] = 0;
    bufc  = 0;
    lines = 0;
    buftouched = FALSE;
}
