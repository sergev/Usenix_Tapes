#include <stdio.h>

#define EOL '\0'

int width = 132, length = 66, sep   = 1, justify = 0, bufsiz, headers = 0,
    equal = 0,   dont   = 0,  ncols = 0, form = 0, tabsize = 8, maxcols = 0;

int widest, eof, this_col, feed, *col_width;

char ***cols, *buffer, *malloc (), *getline ();

char *left_top, *centre_top, *right_top;
char *left_bot, *centre_bot, *right_bot;
int top_space = -1, bot_space = -1;

main (argc, argv) char **argv;
 {
    char *arg;

    for (++argv; argc > 1; argc--, argv++)
	if (*(arg = *argv) != '-' || *++arg == EOL)
	    break;
	else switch (*arg)
	 {
	    case 'm': maxcols = atoi (++arg); break;
	    case 'j': justify = 1; break; /* removes spaces to right */
	    case 't': tabsize = atoi (++arg); break;
	    case 'f': form = 1; break;		/* request true form-feed */
	    case 'w': width = atoi (++arg); break;
	    case 'l': length = atoi (++arg); break;
	    case 's': sep = atoi (++arg); break;
					    /* minimum column separation */
	    case 'e': equal = 1; break;	    /* request equal width columns */
	    case 'd': dont = 1; break;	/* don't break a page on \f */
	    case 'H':
		headers = 1;
		if (header (++arg, argv[1]))
		    argv++, argc--;
	      break;
	    case 'F':
		headers = 1;
		if (footer (++arg, argv[1]))
		    argv++, argc--;
	      break;

	    default:
		if ((*argv)[1] != 'h')
		    fprintf (stderr, "pf: unknown option '%s'\n", *argv );
		fprintf (stderr, "\
Valid options (with defaults if appropriate) are:-\n\
\n\
    -F	    produce footers\n\
    -H	    produce headers\n\
    -d	    don't break a page between files - break column instead\n\
    -e	    equal width columns\n\
    -f	    true form feeds - cause a new page instead of new column\n\
    -h	    produces this listing and exits\n\
    -m	    maximum number of columns (defaults to paper width)\n\
    -j	    remove spaces to the right (old behavior)\n\
    -l66    length of paper\n\
    -s1	    minimum separation of columns\n\
    -t8	    tab size\n\
    -w132   width of paper\n\
\n\
-H and -F are normally turned off and can be appended by the following args\n\
\n\
    l	    following argument is left header\n\
    c	    following argument is centre header\n\
    r	    following argument is right header\n\
    s3	    number of lines used for headers\n\
\n\
");
		    exit (1);
	 }
    if (maxcols == 0) maxcols = width;
    if (top_space < 0) top_space = 3;
    if (bot_space < 0) bot_space = 3;
    if (tabsize <= 0 || length <= 0 || width <= 0)
	fprintf (stderr,
	    "pf: non-positive or non-numeric flag (-t, -l or -w)\n"),
	exit (1);
    cols = (char ***) malloc (width * sizeof (char ***));
    col_width = (int *) malloc (width * sizeof (int *));
    buffer = malloc (bufsiz = width + 1);
    if (argc == 1)
	*argv = "-", argc = 2;
    if (headers)
	if (length <= top_space + bot_space)
	    fprintf (stderr, "pf: page too short for headers\n"),
	    exit (1);
	else
	    length -= top_space + bot_space;
    else
	;
    for (; argc > 1; argc--, argv++)
     {
	int fd;
	FILE *in;

	if (**argv == '-' && (*argv)[1] == EOL)
	    in = stdin;
	else if ((fd = open (*argv, 0)) == -1)
	    fprintf (stderr, "pf: cannot open '%s'\n", *argv), exit (1);
	else
	    close (fd), in = fopen (*argv, "r");
	readfrom (in);
	if (in != stdin)
	    fclose (in);
	if (!dont) output_page ();
     }
    if (ncols > 0) output_page ();
 }

header (ch, s) char *ch, *s;
 {
    switch (*ch)
     {
	case 'l': left_top = s; break;
	case 'c': centre_top = s; break;
	case 'r': right_top = s; break;
	case 's': top_space = atoi (++ch); return 0;
	case '\0': return 0;
	
	default: fprintf (stderr, "pf: -H not followed by l, c, r or s\n");
		exit (1);
     }
    return 1;
 }

footer (ch, s) char *ch, *s;
 {
    switch (*ch)
     {
	case 'l': left_bot = s; break;
	case 'c': centre_bot = s; break;
	case 'r': right_bot = s; break;
	case 's': bot_space = atoi (++ch); return 0;
	case '\0': return 0;
	
	default: fprintf (stderr, "pf: -F not followed by l, c, r or s\n");
		exit (1);
     }
    return 1;
 }

readfrom (f) FILE *f;
 {
    eof = 0;
    while (!eof)
     {
	read_col (f, ncols++);
	if (width_so_far () > width || form && feed || ncols >= maxcols)
	    output_page ();
     }
 }

read_col (f, n) FILE *f; int n;
 {
    int line;
    char **this = cols[n] = (char **) malloc (length * sizeof (char *));

    this_col = n; feed = 0; col_width [n] = 0;
    for (line = 0; line < length; line++)
	this [line] = getline (f);
 }

addch (b, c) char **b; int c;
 {
    char *realloc ();
    
    if (&buffer [bufsiz] == *b)
     {
	buffer = realloc (buffer, bufsiz + width);
	*b = &buffer [bufsiz];
	bufsiz += width;
     }
    *(*b)++ = c;
 }

char *
getline (f) FILE *f;
 {
    int w = 0, *cw = &col_width [this_col], m = 0, c;
    char *result, *b = buffer;


    if (feof (f))
     {
	eof = 1;
	return NULL;
     }
    if (feed)
	return NULL;
    else
     {
	while ((c = getc (f)) != '\n' && c != '\f' && c != EOF)
	    switch (c)
	     {
		case '\r': if (m < w) m = w; w = 0; addch (&b, '\r'); break;
		case '\b': if (m < w) m = w; w--; addch (&b, '\b'); break;
		case '\t': do addch (&b, ' '), ++w; while (w % tabsize != 0); break;
		case 27:   addch (&b, c); if (m < w) m = w; --w; break;
		default:   addch (&b, c); if (c >= ' ' && c < 127) ++w; break;
	     }
	w = m > w ? m : w;
	if (w > width) w = width;
	addch (&b, EOL);
	feed |= (c == '\f'); eof = c == EOF;
     }
    *cw = *cw < w ? w : *cw;
    result = malloc (b - buffer);
    strcpy (result, buffer);
    return result;
 }

width_so_far ()
 {
    int i, sum = 0;

    widest = 0;
    for (i = 0; i < ncols; i++)
     {
	int cwi = col_width [i];

	sum += cwi, widest = widest < cwi ? cwi : widest;
     }
    sum = (equal ? widest * ncols : sum) + (ncols - justify -1 /*REQ*/ ) * sep;
    return (sum);
 }

output_page ()
 {
    int cum = 0, n = 0, new, real_sep, extra, line, col;
    
    if (ncols <= 0) return;	/*REQ*/
    if (headers)
     {
	space (top_space / 2 - 1 + top_space % 2, '\n');
	if (top_space)
	 {
	    printf ("%s", left_top);
	    space (width / 2 - strlen (centre_top) / 2 - strlen (left_top),
									' ');
	    printf (centre_top);
	    space (width / 2 - strlen (right_top) - strlen (centre_top) / 2,
									' ');
	    printf ("%s\n", right_top);
	 }
	space (top_space / 2, '\n');
     }
    
    --ncols;
    if (width_so_far () > width)	    /* executed for side-effect of */
	fprintf (stderr, "pf: internal error"),	    /* recomputing widest */
	exit (1);
    ++ncols;
    
    
    for (n = 0; n < ncols; n++)		/* n is number of columns to print */
	if ((new = cum + (equal ? widest : col_width [n]) + sep)
							     > width + sep ||
		equal && col_width [n] > widest)
	    break;
	else
	    cum = new;

    if (n > 1)
	real_sep = sep + (width + sep - cum) / (n - justify),
	extra = (width + sep - cum) % (n - justify);

    for (line = 0; line < length; line++)
	for (col = 0; col < n; col++)
	 {
	    char *text = cols[col][line];
	    putline ((text != NULL ? text : ""),
		     (col == n ? 0 : equal ? widest : col_width [col]),
		     col + 1 != n);
	    if (text != NULL)
		free (text);
	    if (col + 1 < n)
		space (real_sep + (col < extra ? 1 : 0), ' ');
	    else
		putchar ('\n');
	 }
    for (col = 0; col < n; col++)
	free (cols[col]);
    for (col = n; col < ncols; col++)
	cols[col-n] = cols[col],
	col_width[col-n] = col_width[col];
    ncols -= n;
    
    if (headers)
     {
	space (bot_space / 2, '\n');
	if (bot_space)
	 {
	    printf ("%s", left_bot);
	    space (width / 2 - strlen (centre_bot) / 2 - strlen (left_bot),
									' ');
	    printf (centre_bot);
	    space (width / 2 - strlen (right_bot) - strlen (centre_bot) / 2,
									' ');
	    printf ("%s\n", right_bot);
	 }
	space (bot_space / 2 - 1 + bot_space % 2, '\n');
     }
 }

space (n, c)
 {
    int i;

    for (i = 0; i < n; i++)
	putchar (c);
 }

putline (text, width, pad) char *text; int width, pad;
 {
    int cols = 0;
    
    for (; *text != EOL; text++)
     switch (*text)
      {
	case 27:
		putchar (*text++);
		if (*text == EOL)
		    --text;
		else
		    putchar (*text);
		break;
		
	case '\b':
		if (cols > 0 && cols <= width)
		    putchar (*text);
		--cols;
	    break;
	    
	case '\r':
		cols = cols > width ? width : cols;
		while (cols > 0)
		    --cols, putchar ('\b');
	    break;
	    
	default:
		if (cols >= 0 && cols < width || *text < ' ' || *text >= 127)
		    putchar (*text);
		if (*text >= ' ' && *text < 127)
		    ++cols;
	    break;
      }
    if (pad)
	while (cols++ < width)
	    putchar (' ');
 }
