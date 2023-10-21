/*
**  MAKEKIT
**  Split up source files into reasonably-sized shar lists.
**
**  Options:
**	-e		Leave our output file out
**	-h #		Number of header lines in input file
**	-i name		Input file name
**	-k #		Maximum number of archives desired
**	-m		Same as "-i Manifest -o Manifest -s2"
**	-n name		Name for resultant archives
**	-o name		Output file name
**	-p		Preserve original input order
**	-s #[k]		Maximum size of each archvie
**	-t text		Set final instructions after all are unpacked
**	-x		Don't actually do the shar'ing
*/
#include "shar.h"
RCS("$Header: makekit.c,v 1.15 87/03/13 12:56:41 rs Exp $")


/*
**  Our block of information about the files we're doing.
*/
typedef struct {
    char	*Name;			/* Filename			*/
    char	*Text;			/* What it is			*/
    int		 Where;			/* Where it is			*/
    int		 Type;			/* Directory or file?		*/
    long	 Size;			/* Size in bytes		*/
} BLOCK;


/*
**  Our block of information about the archives we're making.
*/
typedef struct {
    int		 Count;			/* Number of files		*/
    long	 Size;			/* Bytes used by archive	*/
} ARCHIVE;


/*
**  We re-use parts of one buffer to hold three different strings in our
**  argument vector to exec().
*/
#define SEG0		 0		/* Ending archive number	*/
#define SEG1		10		/* Current archive number	*/
#define SEG2		20		/* Output name (rest of buff)	*/

/*
**  Format strings; these are strict K&R so you shouldn't have to change them.
*/
#define FORMAT1		" %-25s%2d\t%s\n"
#define FORMAT2		"%s%2.2d"


/*
**  Global variables.
*/
char	*InName;			/* File with list to pack	*/
char	*OutName;			/* Where our output goes	*/
char	*SharName = "Part";		/* Prefix for name of each shar	*/
char	*Trailer;			/* Text for shar to pack in	*/
char	 TEMP[] = "/tmp/arkXXXXXX";	/* Temporary manifest file	*/
int	 ArchCount = 20;		/* Max number of archives	*/
int	 ExcludeIt;			/* Leave out the output file?	*/
int	 Header;			/* Lines of prolog in input	*/
int	 Preserve;			/* Preserve order for Manifest?	*/
int	 Working = TRUE;		/* Call shar when done?		*/
long	 Size = 55000;			/* Largest legal archive size	*/


/*
**  Sorting predicate to put README first, then directories, then large
**  files, then smaller files, which is how we want to assign things to
**  the archives.
*/
static int
SizeP(t1, t2)
    BLOCK	*t1;
    BLOCK	*t2;
{
    long	 i;

    if (EQ(t1->Name, "README") || EQ(t1->Name, "readme"))
	return(-1);
    if (EQ(t2->Name, "README") || EQ(t1->Name, "readme"))
	return(1);
    if (t1->Type != t2->Type)
	return(t1->Type == F_DIR ? 1 : -1);
    return((i = t1->Size - t2->Size) == 0L ? 0 : (i < 0L ? -1 : 1));
}


/*
**  Sorting predicate to get things in alphabetical order, which is how
**  we write the Manifest file.
*/
static int
NameP(t1, t2)
    BLOCK	*t1;
    BLOCK	*t2;
{
    int		 i;

    return((i = *t1->Name - *t2->Name) ? i : strcmp(t1->Name, t2->Name));
}


/*
**  Skip whitespace.
*/
static char *
Skip(p)
    register char	*p;
{
    while (*p && WHITE(*p))
	p++;
    return(p);
}


/*
**  Signal handler.  Clean up and die.
*/
static
Catch(s)
    int		 s;
{
    int		 e;

    e = errno;
    (void)unlink(TEMP);
    fprintf(stderr, "Got signal %d, %s.\n", s, Ermsg(e));
    exit(1);
}


main(ac, av)
    register int	 ac;
    char		*av[];
{
    register FILE	*F;
    register FILE	*In;
    register BLOCK	*t;
    register ARCHIVE	*k;
    register char	*p;
    register int	 i;
    register int	 lines;
    register int	 Value;
    BLOCK		*Table;
    BLOCK		*TabEnd;
    ARCHIVE		*Ark;
    ARCHIVE		*ArkEnd;
    char		 buff[BUFSIZ];
    int			 LastOne;
    int			 Start;

    /* Collect input. */
    Value = FALSE;
    while ((i = getopt(ac, av, "eh:i:k:n:mop:s:t:x")) != EOF)
	switch (i) {
	    default:
		exit(1);
	    case 'e':
		ExcludeIt = TRUE;
		break;
	    case 'h':
		Header = atoi(optarg);
		break;
	    case 'i':
		InName = optarg;
		break;
	    case 'k':
		ArchCount = atoi(optarg);
		break;
	    case 'm':
		InName = OutName = "MANIFEST";
		Header = 2;
		break;
	    case 'n':
		SharName = optarg;
		break;
	    case 'o':
		OutName = optarg;
		break;
	    case 'p':
		Preserve = TRUE;
		break;
	    case 's':
		Size = atoi(optarg);
		if (IDX(optarg, 'k') || IDX(optarg, 'K'))
		    Size *= 1024;
		break;
	    case 't':
		Trailer = optarg;
		break;
	    case 'x':
		Working = FALSE;
		break;
	}
    ac -= optind;
    av += optind;

    /* Write the file list to a temp file. */
    F = fopen(mktemp(TEMP), "w");
    SetSigs(TRUE, Catch);
    if (av[0])
	/* Got the arguments on the command line. */
	while (*av)
	    fprintf(F, "%s\n", *av++);
    else {
	/* Got the name of the file from the command line. */
	if (InName == NULL)
	    In = stdin;
	else if ((In = fopen(InName, "r")) == NULL) {
	    fprintf(stderr, "Can't read %s as manifest, %s.\n",
		    InName, Ermsg(errno));
	    exit(1);
	}
	/* Skip any possible prolog, then output rest of file. */
	while (--Header >= 0 && fgets(buff, sizeof buff, In))
	    ;
	if (feof(In)) {
	    fprintf(stderr, "Nothing but header lines in list!?\n");
	    exit(1);
	}
	while (fgets(buff, sizeof buff, In))
	    fputs(buff, F);
	if (In != stdin)
	    (void)fclose(In);
    }
    (void)fclose(F);

    /* Count number of files, allow for NULL and our output file. */
    F = fopen(TEMP, "r");
    for (lines = 2; fgets(buff, sizeof buff, F); lines++)
	;
    rewind(F);

    /* Read lines and parse lines, see if we found our OutFile. */
    Table = NEW(BLOCK, lines);
    for (t = Table, Value = FALSE, lines = 0; fgets(buff, sizeof buff, F); ) {
	/* Read line, skip first word, check for blank line. */
	if (p = IDX(buff, '\n'))
	    *p = '\0';
	else
	    fprintf(stderr, "Warning, line truncated:\n%s\n", buff);
	p = Skip(buff);
	if (*p == '\0')
	    continue;

	/* Copy the line, snip off the first word. */
	for (p = t->Name = COPY(p); *p && !WHITE(*p); p++)
	    ;
	if (*p)
	    *p++ = '\0';

	/* Skip <spaces><digits><spaces>; remainder is the file description. */
	for (p = Skip(p); *p && isdigit(*p); )
	    p++;
	t->Text = Skip(p);

	/* Get file type. */
	if (!GetStat(t->Name)) {
	    fprintf(stderr, "Can't stat %s (%s), skipping.\n",
		    t->Name, Ermsg(errno));
	    continue;
	}
	t->Type = Ftype(t->Name);

	/* Guesstimate it's size when archived. */
	t->Size = strlen(t->Name) * 3 + 200;
	if (t->Type == F_FILE) {
	    i = Fsize(t->Name);
	    t->Size += i + i / 60;
	}
	if (t->Size > Size) {
	    fprintf(stderr, "At %ld bytes, %s is too big for any archive!\n",
		    t->Size, t->Name);
	    exit(1);
	}

	/* Is our ouput file there? */
	if (!Value && OutName && EQ(OutName, t->Name))
	    Value = TRUE;

	/* All done -- advance to next entry. */
	t++;
    }
    (void)fclose(F);
    (void)unlink(TEMP);
    SetSigs(S_RESET, (int (*)())NULL);

    /* Add our output file? */
    if (!ExcludeIt && !Value && OutName) {
	t->Name = OutName;
	t->Text = "This shipping list";
	t->Type = F_FILE;
	t->Size = lines * 60;
	t++;
    }

    /* Sort by size, get archive space. */
    lines = t - Table;
    TabEnd = &Table[lines];
    if (!Preserve)
	qsort((char *)Table, lines, sizeof Table[0], SizeP);
    Ark = NEW(ARCHIVE, ArchCount);
    ArkEnd = &Ark[ArchCount];

    /* Loop through the pieces, and put everyone into an archive. */
    for (t = Table; t < TabEnd; t++) {
	for (k = Ark; k < ArkEnd; k++)
	    if (t->Size + k->Size < Size) {
		k->Size += t->Size;
		t->Where = k - Ark;
		k->Count++;
		break;
	    }
	if (k == ArkEnd) {
	    fprintf(stderr, "'%s' doesn't fit -- need more then %d archives.\n",
		    t->Name, ArchCount);
	    exit(1);
	}
	/* Since our share doesn't build sub-directories... */
	if (t->Type == F_DIR && k != Ark)
	    fprintf(stderr, "Warning:  directory '%s' is in archive %d\n",
		    t->Name, k - Ark + 1);
    }

    /* Open the output file. */
    if (OutName == NULL)
	F = stdout;
    else {
	if (GetStat(OutName)) {
	    /* Handle /foo/bar/VeryLongFileName.BAK for non-BSD sites. */
	    (void)sprintf(buff, "%s.BAK", OutName);
	    p = (p = RDX(buff, '/')) ? p + 1 : buff;
	    if (strlen(p) > 14)
		/* ... well, sort of handle it. */
		(void)strcpy(&p[10], ".BAK");
	    fprintf(stderr, "Renaming %s to %s\n", OutName, buff);
	    (void)unlink(buff);
	    (void)link(OutName, buff);
	    (void)unlink(OutName);
	}
	if ((F = fopen(OutName, "w")) == NULL) {
	    fprintf(stderr, "Can't open '%s' for output, %s.\n",
		    OutName, Ermsg(errno));
	    exit(1);
	}
    }

    /* Sort the shipping list, then write it. */
    if (!Preserve)
	qsort((char *)Table, lines, sizeof Table[0], NameP);
    fprintf(F, "   File Name\t\tArchive #\tDescription\n");
    fprintf(F, "-----------------------------------------------------------\n");
    for (t = Table; t < TabEnd; t++)
	fprintf(F, FORMAT1, t->Name, t->Where + 1, t->Text);

    /* Close output.  Are we done? */
    if (F != stdout)
	(void)fclose(F);
    if (!Working)
	exit(0);

    /* Find last archive number. */
    for (i = 0, t = Table; t < TabEnd; t++)
	if (i < t->Where)
	    i = t->Where;
    LastOne = i + 1;

    /* Find archive with most files in it. */
    for (i = 0, k = Ark; k < ArkEnd; k++)
	if (i < k->Count)
	    i = k->Count;

    /* Build the fixed part of the argument vector. */
    av = NEW(char*, i + 10);
    av[0] = "shar";
    i = 1;
    if (Trailer) {
	av[i++] = "-t";
	av[i++] = Trailer;
    }
    (void)sprintf(&buff[SEG0], "%d", LastOne);
    av[i++] = "-e";
    av[i++] = &buff[SEG0];
    av[i++] = "-n";
    av[i++] = &buff[SEG1];
    av[i++] = "-o";
    av[i++] = &buff[SEG2];

    /* Call shar to package up each archive. */
    for (Start = i, i = 0; i < LastOne; i++) {
	(void)sprintf(&buff[SEG1], "%d", i + 1);
	(void)sprintf(&buff[SEG2], FORMAT2, SharName, i + 1);
	for (lines = Start, t = Table; t < TabEnd; t++)
	    if (t->Where == i)
		av[lines++] = t->Name;
	av[lines] = NULL;
	fprintf(stderr, "Packing kit %d...\n", i + 1);
	if (lines = Execute(av))
	    fprintf(stderr, "Warning:  shar returned status %d.\n", lines);
    }

    /* That's all she wrote. */
    exit(0);
}
