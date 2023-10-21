#include	"p.h"

/*
 *  Get the requested local time into the local time part of the
 *  now structure.
 */
gettime()
	{
	char	buf[40];
	int	nitems;

	do {
		fprintf(stderr, "Time (hh:mm:ss):  ");
		gets(buf);
		nitems = sscanf(buf, "%2d:%2d:%2d", &now.lt.tm_hour,
		    &now.lt.tm_min, &now.lt.tm_sec);
	} while (nitems != 3);
	do {
		fprintf(stderr, "Date (mm/dd/yyyy):  ");
		gets(buf);
		nitems = sscanf(buf, "%2d/%2d/%5d", &now.lt.tm_mon,
		    &now.lt.tm_mday, &now.lt.tm_year);
	} while (nitems != 3);
	/*
	 *  Add day of year.
	 */
	dayofyr(&now.lt);
	/*
	 *  Maybe adjust for DST.
	 */
	dstime(&now.lt);
}

struct	equat *
getequat()
	{
	static	struct	equat	eq;
	struct	deg	deg;
	char	buf[40];
	int	nitems;

	do {
		printf("Hour angle (hh:mm:ss):  ");
		gets(buf);
		nitems = sscanf(buf, "%2d:%2d:%2d", &deg.deg, &deg.min,
		    &deg.sec);
	} while (nitems != 3);
	eq.ra = dmstodeg(&deg);
	do {
		printf("Decl (deg:min:ss):  ");
		gets(buf);
		nitems = sscanf(buf, "%2d:%2d:%2d", &deg.deg, &deg.min,
		    &deg.sec);
	} while (nitems != 3);
	eq.dec = dmstodeg(&deg);
	return(&eq);
}

/*
 *  Compute and display angular separation data.
 */
prtsep()
	{
	register p1, p2, p3;
	REAL	seps[8][8];
	REAL	smsep;

	/*
	 *  Compute the angular separations.
	 */
	for (p1 = MERCURY; p1 <= PLUTO; p1++) {
	    for (p2 = p1 + 1; p2 <= PLUTO; p2++) {
		    seps[p1][p2] = angsep(&wandeq[p1], &wandeq[p2]);
	    }
	}
	smsep = angsep(&sun.equat, &moon.equat);

	if ( ! print) {
	    /*
    	     *  Display the angular separations.
	     */
	    for (p1 = MERCURY; p1 <= PLUTO; p1++) {
		for (p2 = p1 + 1; p2 <= PLUTO; p2++) {
			move(pt[SEPMAT].row+p1+1, p2*8);
			if (seps[p1][p2] <= 5.0) {
				standout();
				printw("%7.3f", seps[p1][p2]);
				standend();
			} else
				printw("%7.3f", seps[p1][p2]);
		}
	    }
	    move(pt[SEPMAT].row+6, 8);
	    printw("%7.3f", smsep);
	} else {		/* hard copy */
	    printf("\n      ");
	    /*
	     *  Print the labels.
	     */
	    for (p1 = VENUS; p1 <= PLUTO; p1++)
		printf("%7.7s ", name[p1]);
	    putchar('\n');

	    for (p1 = MERCURY; p1 < PLUTO; p1++) {
	        printf("      ");
		for (p3 = 0; p3 < p1; p3++)
		    printf("        ");
		for (p2 = p1 + 1; p2 <= PLUTO; p2++) {
		    printf("%7.3f ", seps[p1][p2]);
		}
		printf("%s\n", name[p1]);
	    }
	    printf("\tSun-Moon: %7.3f\n", smsep);
	    dashes();
	}
}

prtshdr()
	{
	register p1;

	if (print)
		return;
	/*
	 *  Toss the label.
	 */
	prtname(SEPMAT);
	/*
	 *  Print the labels.  (Note the limits.)
	 */
	for (p1 = VENUS; p1 <= PLUTO; p1++) {
		move(pt[SEPMAT].row, (p1-1)*8+8);
		printw("%7.7s", name[p1]);
	}
	for (p1 = MERCURY; p1 < PLUTO; p1++) {
		/*
		move(pt[SEPMAT].row+p1+1, 1); printw(".");
		 */

		move(pt[SEPMAT].row+p1+1, 64);
		printw("%s", name[p1]);
	}

	move(pt[SEPMAT].row+5, 8);
	printw("    Sun");
	move(pt[SEPMAT].row+6, 16);
	printw("Moon");
}

static	char	*hdrhdr =
"        rh rm dec  azd eld   rise  set       disAU h mn  dias  phse  magnit";

prthdr()
	{
	if (print) {
		dashes();
		printf(hdrhdr);
		putchar('\n');
		putchar('\n');
	} else {
		move(0, 0);
		printw(hdrhdr);
	}

}

dashes()
	{
	register i;

	for (i = 0; i < 76; i++)
		putchar('-');

	putchar('\n');
}

prtname(plano)
	{
	if (print) {
		if (plano == MERCURY)
			prthdr();
		if (plano >= SUN)
			putchar('\n');
		printf(name[plano]);
		if (plano != TIME)
			putchar('\t');
	} else {
		move(pt[plano].row, 0);
		printw(name[plano]);
	}
}

prtmp(tmp)
register struct	tm	*tmp;
	{
	switch (tmp->tm_isdst) {
	case DST:
	case LT:
	case LST:
	case GMT:
	case GST:
		printf("%2dh %2dm %2ds", tmp->tm_hour,
		    tmp->tm_min, tmp->tm_sec);
		switch (tmp->tm_isdst) {
		case DST:
			printf( " DST"); break;
		case LT:
			printf( " LT "); break;
		case LST:
			printf( " LST"); break;
		case GMT:
			printf( " GMT"); break;
		case GST:
			printf( " GST"); break;
		}
		putchar('\n');
		break;

	default:
		printf("%2dh %2dm %2ds isdst: %d\n", tmp->tm_hour,
		    tmp->tm_min, tmp->tm_sec, tmp->tm_isdst);
	}
}

prtdate(tmp)
register struct	tm	*tmp;
	{
	register int	yr;
	register char	*day;

	day = days[tmp->tm_wday];
	yr = tmp->tm_year;

	printf("%3s %2d/%2d/%4d", day, tmp->tm_mon, tmp->tm_mday, yr);
	printf("   wday: %d yday: %3d\n", tmp->tm_wday, tmp->tm_yday);
}

prtae(aep)
register struct	ae	*aep;
	{
	struct	deg	az, el;

	degtodms(&az, aep->az);
	degtodms(&el, aep->el);

	printf("Az: %3dd %2dm %2ds  ", az.deg, az.min, az.sec);
	printf("El: %c%2dd %2dm %2ds     \n", el.sg, el.deg, el.min, el.sec);
}

prteq(eqp)
register struct	equat	*eqp;
	{
	struct	deg	ra, dec;

	degtodms(&ra, eqp->ra);
	degtodms(&dec, eqp->dec);

	printf("Ra: %2dh %2dm %2ds  ", ra.deg, ra.min, ra.sec);
	printf("Dec: %c%2dd %2dm %2ds\n", dec.sg, dec.deg, dec.min, dec.sec);
}

static	char	*planfmt =
"%2d %2d %3d  %3d %3d  %02d:%02d %02d:%02d %3s  %5.2f %d %2d  %4.1f  %4.2f  %6.2f";

prtplan(planp, plano)
register struct	planet	*planp;
	{
	struct	tm	tm, *tmp;
	struct	deg	ra, dec, az, el;
	struct	deg	rt, st;
	char	*timek();

	degtodms(&ra, planp->equat.ra);
	degtodms(&dec, planp->equat.dec);
	if (dec.sg == '-')
		dec.deg = -dec.deg;
	degtodms(&az, planp->hzn.az);
	degtodms(&el, planp->hzn.el);
	if (el.sg == '-')
		el.deg = -el.deg;

	hrtohms(&tm, planp->rs.risetime);
	tmp = lsttolt(&tm);
	rt.deg = tmp->tm_hour;
	rt.min = tmp->tm_min;

	hrtohms(&tm, planp->rs.settime);
	tmp = lsttolt(&tm);
	st.deg = tmp->tm_hour;
	st.min = tmp->tm_min;

	if (print) {
		printf(planfmt, ra.deg, ra.min, dec.deg, az.deg, el.deg,
		    rt.deg, rt.min, st.deg, st.min, timek(tmp),
		    planp->rho, planp->ltt.deg, planp->ltt.min,
		    planp->th, planp->F, planp->m);
		putchar('\n');
	} else {
		move(pt[plano].row, pt[plano].col);
		printw(planfmt, ra.deg, ra.min, dec.deg, az.deg, el.deg,
		    rt.deg, rt.min, st.deg, st.min, timek(tmp),
		    planp->rho, planp->ltt.deg, planp->ltt.min,
		    planp->th, planp->F, planp->m);
	}

}

char *
timek(tmp)
register struct	tm	*tmp;
	{
	char	*p;

	switch (tmp->tm_isdst) {
	case LT:
		p = "LT "; break;

	case DST:
		p = "DST"; break;

	case GMT:
		p = "GMT"; break;

	case GST:
		p = "GST"; break;

	case LST:
		p = "LST"; break;

	default:
		p = "???"; break;
	}
	return(p);
}

static	char	*sunfmt =
"%2d %2d %3d  %3d %3d  %02d:%02d %02d:%02d %3s %5.4fAU  %2dm %2ds";

prtsun()
	{
	struct	tm	tm, *tmp;
	struct	deg	ra, dec, az, el;
	struct	deg	rt, st;
	struct	deg	adia;
	char	*timek();

	degtodms(&ra, sun.equat.ra);
	degtodms(&dec, sun.equat.dec);
	if (dec.sg == '-')
		dec.deg = -dec.deg;
	degtodms(&az, sun.hzn.az);
	degtodms(&el, sun.hzn.el);
	if (el.sg == '-')
		el.deg = -el.deg;

	hrtohms(&tm, sun.rs.risetime);
	tmp = lsttolt(&tm);
	rt.deg = tmp->tm_hour;
	rt.min = tmp->tm_min;

	hrtohms(&tm, sun.rs.settime);
	tmp = lsttolt(&tm);
	st.deg = tmp->tm_hour;
	st.min = tmp->tm_min;

	degtodms(&adia, sun.th);

	if (print) {
		printf(sunfmt, ra.deg, ra.min, dec.deg, az.deg, el.deg,
		    rt.deg, rt.min, st.deg, st.min, timek(tmp),
		    sun.dist, adia.min, adia.sec);
		putchar('\n');
	} else {
		move(pt[SUN].row, pt[SUN].col);
		printw(sunfmt, ra.deg, ra.min, dec.deg, az.deg, el.deg,
		    rt.deg, rt.min, st.deg, st.min, timek(tmp),
		    sun.dist, adia.min, adia.sec);
	}
}

static	char	*moonfmt = "%2d %2d %3d  %3d %3d  %02d:%02d %02d:%02d \
%3s %6.0fkm  %2dm %2ds  %4.2f  %4.1fda";

prtmoon()
	{
	struct	tm	tm, *tmp;
	struct	deg	ra, dec, az, el;
	struct	deg	rt, st;
	struct	deg	adia;
	char	*timek();

	degtodms(&ra, moon.equat.ra);
	degtodms(&dec, moon.equat.dec);
	if (dec.sg == '-')
		dec.deg = -dec.deg;
	degtodms(&az, moon.hzn.az);
	degtodms(&el, moon.hzn.el);
	if (el.sg == '-')
		el.deg = -el.deg;

	hrtohms(&tm, moon.rs.risetime);
	tmp = lsttolt(&tm);
	rt.deg = tmp->tm_hour;
	rt.min = tmp->tm_min;

	hrtohms(&tm, moon.rs.settime);
	tmp = lsttolt(&tm);
	st.deg = tmp->tm_hour;
	st.min = tmp->tm_min;

	degtodms(&adia, moon.th);

	if (print) {
		printf(moonfmt, ra.deg, ra.min, dec.deg, az.deg, el.deg,
		    rt.deg, rt.min, st.deg, st.min, timek(tmp),
		    moon.dist, adia.min, adia.sec, moon.F, moon.age);
		putchar('\n');
	} else {
		move(pt[MOON].row, pt[MOON].col);
		printw(moonfmt, ra.deg, ra.min, dec.deg, az.deg, el.deg,
		    rt.deg, rt.min, st.deg, st.min, timek(tmp),
		    moon.dist, adia.min, adia.sec, moon.F, moon.age);
	}
}

prtimes()
	{
	register int	yr;
	register char	*day, *mon, *ab;
	char	*dfmt = "%3.3s, %3.3s %2d, %4d %2.2s";

	day = days[now.lt.tm_wday];
	yr = now.lt.tm_year;
	mon = month[now.lt.tm_mon-1];

	if (yr > 0)
		ab = "AD";
	else {
		ab = "BC";
		yr = -yr;
	}
	if (print)
		printf(dfmt, day, mon, now.lt.tm_mday, yr, ab);
	else {
		move(pt[TIME].row, pt[TIME].col);
		printw(dfmt, day, mon, now.lt.tm_mday, yr, ab);
	}
	if ( ! print)
		move(pt[TIME].row, 22);
	else
		putchar(' ');
	prtim(&now.lt);

	if ( ! print)
		move(pt[TIME].row, 36);
	else
		printf("   ");
	prtim(&now.gmt);

	if ( ! print)
		move(pt[TIME].row, 50);
	else
		printf("  ");
	prtim(&now.gst);

	if ( ! print)
		move(pt[TIME].row, 64);
	else
		printf("  ");
	prtim(&now.lst);
	/*
	 * For pr page alignment.
	 */
	if (print)
		putchar('\n');
}

prtim(tmp)
register struct	tm	*tmp;
	{
	char	*tfmt = "%02d:%02d:%02d %s";

	if (print)
		printf(tfmt, tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
		    timek(tmp));
	else
		printw(tfmt, tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
		    timek(tmp));
}

static	char	inb[30];
static	int	bufsub;
static	char	buf;

/*
 *  Scan stdin for a character.
 */
scan()
	{
	if (print)
		return;

	while (read(0, &buf, 1) == 1) {
		move(23, bufsub + 4);
		if (buf != ' ' && buf != '\n') {
			inb[bufsub++] = buf;
			addch(buf);
		} else {
			inb[bufsub] = '\0';
			printw("    ");
			donin++;
			bufsub = 0;
		}
		refresh();
	}
}

/*
 *  Process the user's request, if it's there.
 */
dorequest()
	{
	REAL	dt;

	if ( ! donin)
		return(-1);
	else
		donin = 0;

	switch (inb[0]) {
	case 'e':			/* enable elapsed time mode */
		if ((deltat * 1440.0 * 60.0) < 999.0)
			eltime++;
		break;

	case 'n':
		if (inb[1]) switch (inb[1]) {
		case 'e':		/* disable elapsed time mode */
			eltime = 0;
			move(pt[SEPMAT].row+7, SECCOL);
			printw("          ");
			break;

		case 'l':		/* disable looping */
			loop = 0;
			break;
		} else {
			settime();
			return(0);	/* don't ticktock */
		}
		break;

	case 'q':
		die();

	case 'r':			/* reverse the flow of time */
		if ( ! eltime)
			deltat = -deltat;
		break;

	default:			/* look for a new delta t */
		dt = parsint(inb);
		if (eltime && (dt * 1440.0 * 60.0 > 999.0))
			dt = 0.0;
		if (dt)
			/*
			 *  Running in reverse?
			 */
			if (deltat < 0.0)
				deltat = -dt;
			else
				deltat = dt;
		break;
	}
	return(1);
}
