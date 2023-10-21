/*******************************************************
 *
 *	      Calendar Generation Program
 *	    Copyright 1986 David H. Brierley
 *
 * Permission is granted to anyone to use this software for any
 * purpose on any computer system, and to redistribute it freely,
 * subject to the following restrictions:
 * 1. The author is not responsible for the consequences of use of
 *	this software, no matter how awful, even if they arise
 *	from defects in it.
 * 2. The origin of this software must not be misrepresented, either
 *	by explicit claim or by omission.
 * 3. Altered versions must be plainly marked as such, and must not
 *	be misrepresented as being the original software.
 *
 * David H. Brierley
 * Portsmouth, RI
 * {allegra,ihnp4,linus}!rayssd!dhb
 *
 ********************************************************/

#include <stdio.h>

#define TRUE	1
#define FALSE	0

#ifndef PICDATA
#define PICDATA "/staff/dhb/data/picdata"
#endif

struct holiday_data
{
    int     h_month;
    int     h_day;
    char    h_name[14];
    struct holiday_data *h_link;
};
typedef struct holiday_data HDATA;

int     month1;
int     year1;
int     month2;
int     year2;
int     this_day;
int     this_year;
int     this_month;
int     copies;
int     points[13];
int     days[42];
int     julian_days[42];
int     first;
int     rawdata;
int     daycnt[] =
{
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
int     pic_flag;
int     std_flag;
int     hday_flag;

int     easter[100] =
{
    3, 25, 4, 13, 4, 5, 4, 18, 4, 10, 4, 1, 4, 21, 4, 6, 3, 29, 4, 17,
    4, 2, 4, 22, 4, 14, 3, 29, 4, 18, 4, 10, 3, 26, 4, 14, 4, 6, 3, 29,
    4, 11, 4, 2, 4, 22, 4, 14, 3, 30, 4, 18, 4, 10, 3, 26, 4, 15, 4, 6,
    4, 19, 4, 11, 4, 3, 4, 22, 4, 7, 3, 30, 4, 19, 4, 3, 3, 26, 4, 15,
    3, 31, 4, 19, 4, 11, 4, 3, 4, 16, 4, 7, 3, 30, 4, 12, 4, 4, 4, 23
};

char    sday_file[1024];
char    hday_file[1024];
char    output_file[1024];
char    input_file[1024];

char  **line;
char   *monames[12][7];
char   *nums[10][5];
char   *holidays[12][32];
char   *special[4][12][32];

HDATA * hday_head = NULL;

FILE   *calout;

extern  FILE   *popen ();
extern  char   *index ();
extern  char   *strcpy ();
extern  char   *strncpy ();
extern  char   *gets ();
extern  char   *calloc ();
extern  char   *malloc ();
extern  char   *optarg;
extern  int     optind;

#define alloc_3d(base, x, y, z, size) {\
    register unsigned  total; register int i, j, k; register char *area;\
    total = (x * y * z) * size; area = malloc (total);\
    if (area == NULL) { perror ("calndr"); exit (1); }\
    for (i = 0; i < x; i++) for (j = 0; j < y; j++) for (k = 0; k < z; k++) { base[i][j][k] = area; area += size; }\
}

#define alloc_2d(base, x, y, size) {\
    register unsigned  total; register int i, j; register char   *area;\
    total = (x * y) * size; area = malloc (total);\
    if (area == NULL) { perror ("calndr"); exit (1); }\
    for (i = 0; i < x; i++) for (j = 0; j < y; j++) { base[i][j] = area; area += size; }\
}

#define alloc_1d(base, x, size) {\
    register unsigned total; register int i; register char *area;\
    total = x * size; area = malloc (total);\
    if (area == NULL) { perror ("calndr"); exit (1); }\
    for (i = 0; i < x; i++) { base[i] = area; area += size; }\
}

#define zap3(base, x, y, z) {\
    register int i, j, k;\
    for (i = 0; i < x; i++) for (j = 0; j < y; j++) for (k = 0; k < z; k++) *base[i][j][k] = '\0';\
}

#define zap2(base, x, y) {\
    register int i, j;\
    for (i = 0; i < x; i++) for (j = 0; j < y; j++) *base[i][j] = '\0';\
}

main (argc, argv)
int     argc;
char   *argv[];
{
    int     optch;
    int     rc;

    calout = NULL;
    rawdata = 0;
    (void) strcpy (input_file, PICDATA);
    while ((optch = getopt (argc, argv, "r:f:o:P:")) != EOF) {
	switch (optch) {
	case 'r':
	    (void) sprintf (input_file, "pic_h2m %s /tmp/picdata%d",
			    optarg, getpid ());
	    fprintf (stderr, "Please wait while raw data file is transformed\n");
	    rc = system (input_file);
	    if (rc != 0) {
		fprintf (stderr, "Unable to transform raw data file\n");
		exit (1);
	    }
	    (void) sprintf (input_file, "/tmp/picdata%d", getpid ());
	    rawdata = 1;
	    break;
	case 'f':
	    (void) strcpy (input_file, optarg);
	    rawdata = 0;
	    break;
	case 'o':
	    calout = fopen (optarg, "w");
	    if (calout == NULL) {
		fprintf (stderr, "Unable to open output file %s\n", optarg);
		exit (1);
	    }
	    break;
	case 'P':
	    (void) sprintf (output_file, "lpr -P%s", optarg);
	    calout = popen (output_file, "w");
	    if (calout == NULL) {
		fprintf (stderr, "Unable to open pipe to printer %s\n", optarg);
		exit (1);
	    }
	    break;
	default:
	    exit (1);
	}
    }
    if (calout == NULL) {
	fprintf (stderr, "Usage: %s [-r rawdatafile] [-f datafile]\n", argv[0]);
	fprintf (stderr, "       [-o outputfile] [-P printer]\n");
	exit (1);
    }

    alloc_2d (monames, 12, 7, 81);
    alloc_2d (nums, 10, 5, 6);
    alloc_2d (holidays, 12, 32, 14);
    alloc_3d (special, 4, 12, 32, 17);
    read_picdata ();

    while (cardscan () == TRUE) {
	while (copies-- > 0) {
	    make_calndr ();
	}
    }

    if (rawdata == 1) {
	(void) unlink (input_file);
    }

}

make_calndr ()
{
    int     i;
    int     j;

    this_year = year1;
    this_month = month1 - 1;
    daycnt[1] = 28 + leap ();
    gen_hdays ();
    this_day = 0;
    for (i = 0; i < this_month; i++) {
	this_day += daycnt[i];
    }

    while (this_year < year2 || (this_year == year2 && this_month < month2)) {
	this_month++;
	if (this_month < 1) {
	    this_month = 1;
	}
	if (this_month > 12) {
	    this_month = 1;
	    this_year++;
	    this_day = 0;
	    daycnt[1] = 28 + leap ();
	    gen_hdays ();
	}
	if (pic_flag) {
	    print_picture (this_month);
	}
	caltop ();
	first = zeller (this_month, this_year);
	for (i = 0; i < 42; i++) {
	    days[i] = 0;
	    julian_days[i] = 0;
	}
	for (i = 0; i < daycnt[this_month - 1]; i++) {
	    days[i + first] = i + 1;
	    julian_days[i + first] = (i + 1) + this_day;
	}
	this_day += daycnt[this_month - 1];
	dash_line (1);

	for (i = 0; i < 6; i++) {
	    vbar_line ();
	    date_line (i);
	    for (j = 0; j < 4; j++) {
		special_line (j, this_month, i);
	    }
	    jdate_line (this_month, i);
	    dash_line (0);
	}
    }
}

leap ()
{
    int     yr;

    yr = this_year;

    if ((yr / 4) * 4 != yr) {
	return 0;
    }
    if ((yr / 100) * 100 != yr) {
	return 1;
    }
    if ((yr / 400) * 400 != yr) {
	return 0;
    }
    return 1;

}

zeller (month, year)
int     month;
int     year;
{
    int     result;
    int     m,
            c,
            y,
            f;

    y = year;
    m = month - 2;
    if (m <= 0) {
	m += 12;
	y--;
    }

    c = y / 100;
    y -= (c * 100);

    f = ((26 * m - 2) / 10) + 1 + ((5 * y) / 4) + (c / 4) - (2 * c);

    result = f % 7;
    return result;

}

cardscan ()
{
    char    input_line[80];
    char   *ptr;
    int     plus_minus;
    int     n;

    if (isatty (0)) {
	printf ("Enter command line, enter '?' for help, 'q' to quit\n");
	printf (">> ");
	(void) fflush (stdout);
    }

    if (getline (0, input_line) == -1) {
	if (isatty (0)) {
	    printf ("\n");
	}
	return FALSE;
    }

    if (input_line[0] == 'q') {
	return FALSE;
    }

    if (input_line[0] == '?') {
	help ();
	return cardscan ();
    }

    n = sscanf (input_line, "%d/%d,%d/%d", &month1, &year1, &month2, &year2);
    if (n != 4) {
	printf ("Error: badly formed input line\n");
	return cardscan ();
    }
    ptr = index (input_line, ' ');
    ptr++;
    zap3 (special, 4, 12, 32);
    plus_minus = TRUE;
    copies = 1;
    pic_flag = TRUE;
    std_flag = TRUE;
    hday_flag = TRUE;
    (void) strcpy (sday_file, "");

    while (*ptr) {
	switch (*ptr) {
	case '+': 
	    plus_minus = TRUE;
	    break;
	case '-': 
	    plus_minus = FALSE;
	    break;
	case 'p': 
	    pic_flag = plus_minus;
	    break;
	case 'h': 
	    hday_flag = plus_minus;
	    if (hday_flag) {
		read_holidays (ptr + 1);
	    }
	    if (hday_file[0] == '\0') {
		hday_flag = FALSE;
	    }
	    ptr = index (ptr, ' ');
	    break;
	case 's': 
	    std_flag = plus_minus;
	    break;
	case 'x': 
	    if (plus_minus) {
		read_sdays (ptr + 1);
	    }
	    ptr = index (ptr, ' ');
	    break;
	case 'c': 
	    copies = atoi (ptr + 1);
	    if (copies < 1) {
		copies = 1;
	    }
	    ptr = index (ptr, ' ');
	    break;
	}
	ptr++;
    }

    printf ("options: begin = %02d/%d, end = %02d/%d, copies = %d\n",
	    month1, year1, month2, year2, copies);

    if (pic_flag) {
	printf ("\t\tpictures");
    }
    else {
	printf ("\t\tnopictures");
    }

    if (std_flag) {
	printf (", standard");
    }
    else {
	printf (", nostandard");
    }

    if (hday_flag) {
	printf (", holidays(%s)", hday_file);
    }
    else {
	printf (", noholidays");
    }

    if (sday_file[0] != '\0') {
	printf (", specialdays(%s)", sday_file);
    }
    else {
	printf (", nospecialdays");
    }

    printf ("\n");

    return TRUE;
}

read_sdays (ptr)
char   *ptr;
{
    char    sd_line[256];
    char   *line_ptr;
    char   *msg;
    int     fd_sday;
    int     sp_indx;
    int     month;
    int     day;
    int     i,
            j,
            k,
            n;

    (void) strcpy (sd_line, ptr);
    msg = index (sd_line, ' ');
    if (msg != NULL) {
	*msg = '\0';
    }
    fd_sday = open (sd_line, 0);
    if (fd_sday == -1) {
	return;
    }
    (void) strcpy (sday_file, sd_line);

    while (getline (fd_sday, sd_line) != -1) {
	line_ptr = sd_line;
	while (*line_ptr && (*line_ptr == ' ')) {
	    line_ptr++;
	}
	if (!*line_ptr) {
	    continue;
	}
	n = sscanf (line_ptr, "%d%*c%d%*c%d", &i, &j, &k);
	switch (n) {
	case 3: 
	    if ((i < 1) || (i > 4)) {
		printf ("ignoring special day line '%s'\n", sd_line);
		continue;
	    }
	    if (i == 4) {
		printf ("Warning: special day '%s' ", sd_line);
		printf ("might be overwritten by two line holiday\n");
	    }
	    sp_indx = i - 1;
	    month = j - 1;
	    day = k;
	    break;
	case 2: 
	    sp_indx = 1;
	    month = i - 1;
	    day = j;
	    break;
	default: 
	    printf ("ignoring special day line '%s'\n", sd_line);
	    continue;
	}
	if (*special[sp_indx][month][day] != '\0') {
	    printf ("Warning: special day conflicts with previous entry\n");
	    printf ("currently processing file %s\n", sday_file);
	    printf ("input line = '%s'\n", sd_line);
	    printf ("previous entry = '%s'\n", special[sp_indx][month][day]);
	}
	msg = index (line_ptr, ' ');
	while (*msg == ' ') {
	    msg++;
	}
	(void) strcpy (special[sp_indx][month][day], msg);
    }
    (void) close (fd_sday);
}

special_line (indx, month, week)
int     indx;
int     month;
int     week;
{
    int     day;

    fprintf (calout, "  ");
    month--;
    for (day = week * 7; day < (week + 1) * 7; day++) {
	fprintf (calout, "| %-16s", special[indx][month][days[day]]);
    }
    fprintf (calout, "|\n");
}

date_line (week)
int     week;
{
    int     day;

    fprintf (calout, "  |");
    for (day = week * 7; day < (week + 1) * 7; day++) {
	if (days[day] != 0) {
	    fprintf (calout, "%16d |", days[day]);
	}
	else {
	    fprintf (calout, "%16s |", " ");
	}
    }
    fprintf (calout, "\n");
}

jdate_line (month, week)
int     month;
int     week;
{
    int     day;

    fprintf (calout, "  ");
    month--;
    for (day = week * 7; day < (week + 1) * 7; day++) {
	if (days[day] != 0) {
	    fprintf (calout, "|%03d %-13s", julian_days[day],
		    holidays[month][days[day]]);
	}
	else {
	    fprintf (calout, "|%17s", " ");
	}
    }
    fprintf (calout, "|\n");
}

dash_line (n)
int     n;
{
    while (n-- > 0) {
	fprintf (calout, "\n");
    }

    fprintf (calout, "  ");
    for (n = 0; n < 7; n++) {
	fprintf (calout, "+-----------------");
    }
    fprintf (calout, "+\n");
}

vbar_line ()
{
    int     n;

    fprintf (calout, "  ");
    for (n = 0; n < 7; n++) {
	fprintf (calout, "%-18s", "|");
    }
    fprintf (calout, "|\n");
}

caltop ()
{
    int     i;
    int     y_thou;
    int     y_hund;
    int     y_ten;
    int     y_one;

    y_thou = this_year / 1000;
    y_hund = (this_year % 1000) / 100;
    y_ten = (this_year % 100) / 10;
    y_one = (this_year % 10);

    fprintf (calout, "\f");

    for (i = 0; i < 8; i++) {
	switch (i) {
	case 0: 
	case 1: 
	    fprintf (calout, "%26s%s\n", " ", monames[this_month - 1][i]);
	    break;
	case 2: 
	    fprintf (calout, "%6s%-20s%-94s%s\n", " ",
		    nums[y_hund][i - 2], monames[this_month - 1][i],
		    nums[y_ten][i - 2]);
	    break;
	default: 
	    fprintf (calout, "%-6s%-20s%-94s%-6s%s\n",
		    nums[y_thou][i - 3], nums[y_hund][i - 2],
		    monames[this_month - 1][i],
		    nums[y_ten][i - 2], nums[y_one][i - 3]);
	    break;
	case 7: 
	    fprintf (calout, "%-126s%s\n",
		    nums[y_thou][i - 3], nums[y_one][i - 3]);
	    break;
	}
    }
    fprintf (calout, "\n%-9s%-18s%-18s%-18s%-18s%-18s%-18s%s\n",
	    " ", "Sunday", "Monday", "Tuesday", "Wednesday",
	    "Thursday", "Friday", "Saturday");

}



print_picture (month)
int     month;
{
    int     i;
    char   *outline;

    for (i = points[month - 1]; i < points[month]; i++) {
	outline = line[i];
	switch (*outline) {
	case '1': 
	    fprintf (calout, "\f");
	    break;
	case '-': 
	    fprintf (calout, "\n\n\n");
	    break;
	case '0': 
	    fprintf (calout, "\n\n");
	    break;
	case ' ': 
	    fprintf (calout, "\n");
	    break;
	case '+': 
	    fprintf (calout, "\r");
	    break;
	default: 
	    fprintf (calout, "\n");
	    break;
	}
	fprintf (calout, "%s", ++outline);
    }
    fprintf (calout, "\r");
}

read_picdata ()
{
    int     picdata;
    unsigned    numlines;

    picdata = open (input_file, 0);
    if (picdata == -1) {
	perror ("calndr");
	fprintf (stderr, "Unable to open data file\n");
	exit (1);
    }

    if (read (picdata, monames[0][0], 12 * 7 * 81) != (12 * 7 * 81)) {
	fprintf (stderr, "Incomplete read on data file while reading sss\n");
	exit (1);
    }
    if (read (picdata, nums[0][0], 10 * 5 * 6) != (10 * 5 * 6)) {
	fprintf (stderr, "Incomplete read on data file while reading sss\n");
	exit (1);
    }
    if (read (picdata, (char *) points, sizeof points) != sizeof points) {
	fprintf (stderr, "Incomplete read on data file while reading sss\n");
	exit (1);
    }
    numlines = (unsigned) points[12];
    line = (char **) calloc (numlines, sizeof (char *));
    alloc_1d (line, numlines, 134);
    if (read (picdata, line[0], (int) (numlines * 134)) != (numlines * 134)) {
	fprintf (stderr, "Incomplete read on data file while reading sss\n");
	exit (1);
    }
    (void) close (picdata);
    return;

}

/*----- subroutine get_line					-----*/

#define BUFFSIZE 4096
#define BUFCOUNT 6

getline (fd, buf)
int     fd;
char   *buf;
{

    static char buffer[BUFCOUNT][BUFFSIZE];
    static int  buf_pos[BUFCOUNT];
    static int  buf_size[BUFCOUNT];
    static int  init_flag = 1;
    int     i;


    if (init_flag) {
	for (i = 0; i < BUFCOUNT; i++) {
	    buf_pos[i] = BUFFSIZE;
	    buf_size[i] = BUFFSIZE;
	}
	init_flag = 0;
    }

    if (fd >= BUFCOUNT) {
	printf ("Error: get_line: fd = %d\n", fd);
	exit (1);
    }


    i = 0;

    while (1) {
	if (buf_pos[fd] >= buf_size[fd]) {
	    buf_size[fd] = read (fd, buffer[fd], BUFFSIZE);
	    buf_pos[fd] = 0;
	    if (buf_size[fd] == 0) {
		break;
	    }
	}

	if (buffer[fd][buf_pos[fd]] == '\n')
	    break;
	if (buffer[fd][buf_pos[fd]] == 12) {
	    buf_pos[fd]++;
	    continue;
	}
	*buf = buffer[fd][buf_pos[fd]++];
	if (*buf == '\t') {
	    *buf = ' ';
	    while (i % 8 != 7) {
		*++buf = ' ';
		i++;
	    }
	}
	buf++;
	i++;

    }

    *buf = '\0';
    if (i == 0 && buf_size[fd] == 0 && buf_pos[fd] == 0) {
	i = -1;
    }
    buf_pos[fd]++;

    return (i);

}

#define	VU(s)	fprintf(vu, s)

help ()
{
    FILE   *vu;
    FILE   *popen ();
    char    junk[256];

#ifdef PAGER
    if ((vu = popen (PAGER, "w")) == NULL)
#endif
	vu = stdout;

    VU ("The format of a command input line is as follows:\n");
    VU (" mm/yyyy,mm/yyyy [options]\n\n");
    VU ("The first mm/yyyy specifies the month and year the\n");
    VU ("calendar is to begin with and the second mm/yyyy specifies\n");
    VU ("the ending month and year.  The options that may be given\n");
    VU ("are as follows:\n");
    VU ("\n");
    VU ("   +p        = print pictures\n");
    VU ("   -p        = dont print pictures\n");
    VU ("   +hFILE    = read list of special holidays from FILE\n");
    VU ("   -h        = no special holidays\n");
    VU ("   +s        = include standard holidays on calendar\n");
    VU ("   -s        = dont include standard holidays\n");
    VU ("   +xFILE    = read list of special days from FILE\n");
    VU ("   +cNN      = print NN copies of calendar\n");
    VU ("\n");
#ifndef PAGER
    VU ("Type RETURN for more help\n");
    (void) getline (0, junk);
#endif
    VU ("The special day notations are written on the four lines\n");
    VU ("in the center of the block for the specified day.\n");
    VU ("The format of the special day entries is as follows:\n");
    VU ("\n");
    VU ("   n:mm/dd descriptive text\n");
    VU ("\n");
    VU ("The 'n:' at the beginning of the line specifies which\n");
    VU ("of the four lines in the block to print the descriptive\n");
    VU ("text on.  If the 'n:' is omitted, the default is to print\n");
    VU ("the text on the third line.  The format of the holiday\n");
    VU ("entries is the same except that the 'n:' is always omitted.\n");
    VU ("The text of a holiday entry is printed on the bottom line\n");
    VU ("of the block, next to the julian date.\n\n");
    VU ("The descriptive text can be up to 16 characters for a\n");
    VU ("special day entry and up to 13 characters for a holiday.\n");

    if (vu != stdout) {
	(void) pclose (vu);
    }

}

gen_hdays ()
{
    HDATA * ptr;

    if ((this_year == year2) && (month2 == 1)) {
	return;
    }

    zap2 (holidays, 12, 32);

    if (std_flag) {
	std_hdays ();
    }

    if (!hday_flag) {
	return;
    }
    if (hday_head == NULL) {
	return;
    }

    ptr = hday_head;
    while (ptr) {
	if (*holidays[ptr -> h_month][ptr -> h_day] != '\0') {
	    printf ("User specified holiday conflicts with standard holiday\n");
	    printf ("\tstandard holiday = '%s', user holiday = '%s'\n",
		    holidays[ptr -> h_month][ptr -> h_day], ptr -> h_name);
	    (void) fflush (stdout);
	}
	(void) strcpy (holidays[ptr -> h_month][ptr -> h_day], ptr -> h_name);
	ptr = ptr -> h_link;
    }

}

std_hdays ()
{
    int     temp;
    int     month;

    /* fixed holidays */
    (void) strcpy (holidays[0][1], "New Years Day");
    (void) strcpy (special[3][0][15], "   Martin Luther");
    (void) strcpy (holidays[0][15], " King Jr. Day");
    (void) strcpy (holidays[1][2], "Groundhog Day");
    (void) strcpy (holidays[1][14], "Valentine Day");
    (void) strcpy (special[3][1][12], "       Lincoln's");
    (void) strcpy (holidays[1][12], "    Birthday ");
    (void) strcpy (special[3][1][22], "    Washington's");
    (void) strcpy (holidays[1][22], "   Birthday  ");
    (void) strcpy (special[3][2][17], " Saint Patrick's");
    (void) strcpy (holidays[2][17], "    Day      ");
    (void) strcpy (holidays[5][14], "Flag Day     ");
    (void) strcpy (special[3][6][4], "     Independence");
    (void) strcpy (holidays[6][4], "      Day    ");
    (void) strcpy (special[3][9][24], "  United Nations");
    (void) strcpy (holidays[9][24], "    Day      ");
    (void) strcpy (holidays[9][31], "Halloween    ");
    (void) strcpy (holidays[10][11], "Veterans Day ");
    (void) strcpy (holidays[11][25], "Christmas    ");

    /* Armed Forces Day */
    temp = 21 - zeller (5, this_year);
    (void) strcpy (special[3][4][temp], "    Armed Forces");
    (void) strcpy (holidays[4][temp], "     Day     ");

    /* Labor Day */
    temp = 9 - zeller (9, this_year);
    if (temp > 7) {
	temp -= 7;
    }
    (void) strcpy (holidays[8][temp], "Labor Day");

    /* Thanksgiving */
    temp = 26 - zeller (11, this_year);
    if (temp < 22) {
	temp += 7;
    }
    (void) strcpy (holidays[10][temp], "Thanksgiving");

    /* Memorial Day */
    temp = 30 - zeller (5, this_year);
    if (temp < 25) {
	temp += 7;
    }
    (void) strcpy (holidays[4][temp], "Memorial Day");

    /* Mothers Day */
    temp = 15 - zeller (5, this_year);
    if (temp > 14) {
	temp -= 7;
    }
    (void) strcpy (holidays[4][temp], "Mother's Day");

    /* Fathers Day */
    temp = 22 - zeller (6, this_year);
    if (temp > 21) {
	temp -= 7;
    }
    (void) strcpy (holidays[5][temp], "Father's Day");

    /* Columbus Day */
    temp = 16 - zeller (10, this_year);
    if (temp > 12) {
	temp -= 7;
    }
    (void) strcpy (holidays[9][temp], "Columbus Day");

    /* generate a couple of Canadian holidays */
    temp = 23 - zeller (5, this_year);
    (void) strcpy (special[3][4][temp], "     Canadian   ");
    (void) strcpy (holidays[4][temp], "Victoria Day ");

    temp = zeller (7, this_year);
    switch (temp) {
    case 0: 
	temp = 2;
	break;
    case 6: 
	temp = 3;
	break;
    default: 
	temp = 1;
	break;
    }
    (void) strcpy (special[3][6][temp], "     Canadian   ");
    (void) strcpy (holidays[6][temp], "Dominion Day ");

    /* if year is withing range, generate easter etc. */
    temp = this_year - 1950;
    if ((temp < 1) || (temp > 50)) {
	printf ("Warning: requested year is not within the bounds for\n");
	printf ("generating Easter and related holidays.\n");
	return;
    }

    temp--;
    temp *= 2;
    month = easter[temp];
    temp = easter[++temp];
    (void) strcpy (holidays[--month][temp], "Easter");

    temp -= 2;
    if (temp < 1) {
	month--;
	temp += daycnt[month];
    }
    (void) strcpy (holidays[month][temp], "Good Friday");

    temp -= 5;
    if (temp < 1) {
	month--;
	temp += daycnt[month];
    }
    (void) strcpy (holidays[month][temp], "Palm Sunday");

    temp -= 39;
    while (temp < 1) {
	month--;
	temp += daycnt[month];
    }
    (void) strcpy (holidays[month][temp], "Ash Wednesday");

}

read_holidays (sptr)
char   *sptr;
{
    char    file[1024];
    char    input_line[80];
    char   *temp;
    int     fd;
    int     i,
            j,
            n;
    HDATA  *ptr;
    HDATA  *next;

    if (*sptr == ' ') {
	return;
    }

    if (hday_head != NULL) {
	ptr = hday_head;
	while (ptr) {
	    next = ptr -> h_link;
	    free ((char *) ptr);
	    ptr = next;
	}
    }

    hday_head = NULL;
    (void) strcpy (hday_file, "");
    (void) strcpy (file, sptr);
    if ((temp = index (file, ' ')) != NULL) {
	*temp = '\0';
    }

    if ((fd = open (file, 0)) == -1) {
	printf ("** warning ** - unable to open file %s\n", file);
	printf ("                +h option ignored\n");
	hday_flag = FALSE;
	return;
    }
    (void) strcpy (hday_file, file);

    while (getline (fd, input_line) != -1) {
	temp = input_line;
	while (*temp && (*temp == ' ')) {
	    temp++;
	}
	if (!*temp) {
	    continue;
	}
	n = sscanf (temp, "%d%*c%d", &i, &j);
	if (n != 2) {
	    continue;
	}
	next = (HDATA *) malloc (sizeof (HDATA));
	if (next == NULL) {
	    perror ("calndr");
	    printf ("Unable to alocate space for holiday_data\n");
	    return;
	}
	if (hday_head == NULL) {
	    hday_head = next;
	}
	else {
	    ptr -> h_link = next;
	}
	ptr = next;
	ptr -> h_link = NULL;
	i--;
	temp = index (temp, ' ');
	while (*temp && (*temp == ' ')) {
	    temp++;
	}
	ptr -> h_month = i;
	ptr -> h_day = j;
	(void) strncpy (ptr -> h_name, temp, 13);
    }

    (void) close (fd);

}
