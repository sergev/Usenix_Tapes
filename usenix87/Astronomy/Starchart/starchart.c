/*
 * starchart.c - draw yale star catalog on binary output device
 *
 * (c) copyright 1987 by Alan Paeth (awpaeth@watcgl)
 */

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include "starchart.h"

#define STARFILE "/u/awpaeth/hobby/astro/yale.star"
#define PLANETFILE "/u/awpaeth/hobby/astro/planet.star"

#define DCOS(x) (cos((x)*3.14159265354/180.0))
#define MAX(a,b) ((a)>(b)?(a):(b))

double max(), modup(), moddown();

main(argc, argv)
    int argc;
    char *argv[];
    {
    double thumbscale;
    vecopen();
    chartparms(&master, atof(argv[1]), atof(argv[2]), atof(argv[3]));
    thumbscale = MAX( atof(argv[3]) * 1.2, 10.0);
    chartparms(&thumbnail, atof(argv[1]), atof(argv[2]), thumbscale);
    chartall(&master);
    chartall(&thumbnail);
    chartlegend(&master, (argc >= 5) ? argv[4] : "LEGEND");
    vecclose();
    exit(0);
    }

chartparms(chart, ra, decl, scl)
    double ra, decl, scl;
    map chart;
    {
    double adj, xscale;
    chart->racen = ra * 360.0 / 24.0;
    chart->dlcen = decl;
    chart->scale = scl;
    if (chart->scale == 0.0) chart->scale = 15.0;

    chart->north = (chart->dlcen + chart->scale / 2.0);
    chart->south = (chart->dlcen - chart->scale / 2.0);
/*
 * xscale is other than chart->scale in order to widen the horizontal viewing
 * area, which otherwise shrinks near the poles under Sanson's projection
 * this happens in polar maps which do not span the celestial equator
 */
    adj = 1.0;
    if (chart->north * chart->south > 0.0)
	adj = max(DCOS(chart->north), DCOS(chart->south));
    xscale = chart->scale/adj;

    chart->east  = (chart->racen + xscale*chart->ww/chart->wh / 2.0);
    chart->west  = (chart->racen - xscale*chart->ww/chart->wh / 2.0);
    chart->yscale = chart->wh / chart->scale;
    }

chartlegend(chart, title)
    map chart;
    char *title;
    {
    char tstr[20], tstr2[20];
    vecsize(14);
    vecsyms(100, 220, title);
    vecsize(10);
    vecsyms( 20, 200, "= = = = = = = = = = = =");
    vecsyms( 20, 180, "Right Ascension:");
    rastr(tstr, chart->racen);
    sprintf(tstr2, " %s", tstr);
    vecsyms(260, 180, tstr2);
    vecsyms( 20, 160, "Declination:");
    declstr(tstr, chart->dlcen);
    vecsyms(260, 160, tstr);
    vecsyms( 20, 140, "Chart Scale:");
    sprintf(tstr, "%.3f", chart->scale);
    sprintf(tstr2, " %s", tstr);
    vecsyms(260, 140, tstr2);
    vecsyms( 20,  20, "= = = = = = = = = = = =");

    vecsyms(115, 110, "<1.0");
    vecsyms(115,  80, "<2.0");
    vecsyms(115,  50, "<3.0");
    vecsyms(260, 110, "<4.0");
    vecsyms(260,  80, "<5.0");
    vecsyms(260,  50, ">5.0");

    drawstar( 65, 110, 0);
    drawstar( 65,  80, 1);
    drawstar( 65,  50, 2);
    drawstar(240, 110, 3);
    drawstar(240,  80, 4);
    drawstar(240,  50, 5);
    }

readstar(file, lat, lon, mag, code, subcode, label)
    FILE *file;
    double *lat, *lon, *mag;
    char *code, *subcode, *label;
    {
#define LINELEN 40
    char sbuf[LINELEN];
    double rah, ram, ras, dld, dlm, dl, inten;
    fgets(sbuf, LINELEN, file);
    if (feof(file)) return(1);
/*
 * sscanf of floats is TOOO slow:
 *     sscanf(sbuf, "%2f%2f%2f%c%2f%2f%c%3f%c%c%s", ... );
 * use alternate:
 */
#define F2(i) ((double)((sbuf[i]-'0')*10+sbuf[i+1]-'0'))
#define F3(i) ((double)((sbuf[i]-'0')*100+(sbuf[i+1]-'0')*10+sbuf[i+2]-'0'))
    rah = F2(0);
    ram = F2(2);
    ras = F2(4);
    dld = F2(7);
    dlm = F2(9);
    inten = F3(12);
    code[0] = sbuf[15];
    subcode[0] = sbuf[16];
    label[0] = '\0';
    strncat(label, &sbuf[17], strlen(&sbuf[17])-1);
#define DLDEGSEC 3600.0
#define DLMINSEC 60.0
#define RAHRSSEC 54000.0
#define RAMINSEC 900.0
#define RASECSEC 15.0
    *lon = (RAHRSSEC*rah + RAMINSEC*ram + RASECSEC*ras)/DLDEGSEC;
    dl = (DLDEGSEC*dld + DLMINSEC*dlm)/DLDEGSEC;
    *lat = (sbuf[6]  == '-') ? -dl : dl;
    *mag = ((sbuf[11] == '-') ? -inten : inten)/100.0;
    return(0);
    }

xform(chart, lat, lon, xloc, yloc)
    map chart;
    double lat, lon;
    int *xloc, *yloc;
    {
 /*
  * This is Sanson's Sinusoidal projection. Its properties:
  *   (1) area preserving
  *   (2) preserves linearity along y axis (declination/azimuth)
  */
    *xloc = chart->wx+chart->ww/2 + (chart->racen-lon)*chart->yscale*DCOS(lat);
    *yloc = chart->wy + (int)((lat - chart->south) * chart->yscale);
    }

chartall(chart)
    map chart;
    {
#define READMODE "r"
#define OPENFAIL 0
    FILE *sfile, *pfile;
    if ((sfile = fopen(STARFILE, READMODE)) == OPENFAIL)
	die("open fail on %s", STARFILE);
    chartoutline(chart);
    chartgrid(chart);
    chartobjects(chart, sfile);
    fclose(sfile);
    if ((pfile = fopen(PLANETFILE, READMODE)) != OPENFAIL)
	{
	chartobjects(chart, pfile);
	fclose(pfile);
	}
    }

chartobjects(chart, file)
    map chart;
    FILE *file;
    {
    double lat, lon, mag;
    char code[1], subcode[1], label[100];
    int xloc, yloc;
    for(;;)
	{
	if (readstar(file, &lat, &lon, &mag, code, subcode, label)) break;
	if (mag > chart->maglim) break;
	if ((chart->west < 0.0) && (lon>180.0)) lon -= 360.0;
	if ((chart->east > 360.0) && (lon<180.0)) lon += 360.0;
	if ( (lon >= chart->west) && (lon <= chart->east) &&
	     (lat >= chart->south) && (lat <= chart->north) )
	    {
	    xform(chart, lat, lon, &xloc, &yloc);
	    if (code[0] == 'S') drawstar(xloc, yloc, (int)(mag));
	    if (code[0] == 'P') drawP(xloc, yloc);
	    if (strlen(label))
		{
		vecsize(10);
		vecsyms(xloc+8, yloc, label);
		}
	    }
	}
    }

/*
 * Chart Construction
 */

chartgrid(chart)
    map chart;
    {
    charthgrid(chart, 15.0, 18);
    charthgrid(chart, 5.0, 12);
    charthgrid(chart, 1.0, 6);
    chartvgrid(chart, 10.0, 18);
    chartvgrid(chart, 5.0 , 12);
    chartvgrid(chart, 1.0, 6);
    }

chartoutline(chart)
    map chart;
    {
    double start, inc;
    int xloc, xloc2, yloc, yloc2, div, i;

    xform(chart, chart->south, chart->west, &xloc,  &yloc);
    xform(chart, chart->south, chart->east, &xloc2, &yloc2);
    vecmovedraw(xloc, yloc, xloc2, yloc2);
    xform(chart, chart->north, chart->west, &xloc,  &yloc);
    xform(chart, chart->north, chart->east, &xloc2, &yloc2);
    vecmovedraw(xloc, yloc, xloc2, yloc2);

    inc = (chart->north - chart->south);
    div = (int)(inc);
    if (div < 1) div = 1;
    inc /= div;
    start = chart->south;
    xform(chart, start, chart->west, &xloc, &yloc);
    vecmove(xloc, yloc);
    for (i=0; i < div; i++)
	{
	start += inc;
	xform(chart, start, chart->west, &xloc, &yloc);
	vecdraw(xloc, yloc);
	}
    start = chart->south;
    xform(chart, start, chart->east, &xloc, &yloc);
    vecmove(xloc, yloc);
    for (i=0; i < div; i++)
	{
	start += inc;
	xform(chart, start, chart->east, &xloc, &yloc);
	vecdraw(xloc, yloc);
	}
    }

rastr(str, ra)
    char *str;
    double ra;
    {
    int hrs, min;
    hrs = (int)(ra/15.0);
    min = (int)((ra - hrs * 15.0) * 4.0);
    sprintf(str, "%02dh", hrs);
    if (min) sprintf(str, "%s%02dm", str, min);
    }

declstr(str, dl)
    char *str;
    double dl;
    {
    int deg, min;
    if (dl == 0.0) sprintf(str, "%s", " ");
    else if (dl > 0.0) sprintf(str, "%s", "+");
	else
	{
	sprintf(str, "%s", "-");
	dl = -dl;
	}
    deg = (int)(dl);
    min = (int)((dl - deg) * 60.0);
    sprintf(str, "%s%02dd", str, deg);
    if (min) sprintf(str, "%s%02dm", str, min);
    }

charthgrid(chart, inc, hgt)
    map chart;
    double inc;
    {
#define HTICKLIM 2
#define HTEXTLIM 80
    double start, stop, ra;
    int xloc, xloc2, yloc, xloc3, yloc3;
    start = modup(chart->west, inc);
    stop = moddown(chart->east, inc);
    xform(chart, chart->south, start, &xloc, &yloc);
    xform(chart, chart->south, start+inc, &xloc2, &yloc);
    if (xloc - xloc2 > HTICKLIM)
	for (ra = start; ra <= stop; ra += inc)
	    {
	    xform(chart, chart->south, ra, &xloc3, &yloc3);
	    vecmovedraw(xloc3, yloc3-hgt, xloc3, yloc3);
	    if (xloc - xloc2 > HTEXTLIM)
		{
		char tstr[20];
		rastr(tstr, ra);
		vecsize(10);
		vecsyms(xloc3+2, yloc-17, tstr);
		}
	    }
    }

chartvgrid(chart, inc, wid)
    map chart;
    double inc;
    {
#define VTICKLIM 2
#define VTEXTLIM 20
    double start, stop, dl;
    int xloc, yloc, yloc2, xloc3, yloc3;
    start = modup(chart->south, inc);
    stop = moddown(chart->north, inc);
    xform(chart, start, chart->west, &xloc, &yloc);
    xform(chart, start+inc, chart->west, &xloc, &yloc2);
    if (yloc2 - yloc > VTICKLIM)
	for (dl = start; dl <= stop; dl += inc)
	    {
	    xform(chart, dl, chart->west, &xloc3, &yloc3);
	    vecmovedraw(xloc3, yloc3, xloc3+wid, yloc3);
	    if (yloc2 - yloc > VTEXTLIM)
		{
		char tstr[20];
		declstr(tstr, dl);
		vecsize(10);
		vecsyms(xloc3+24, yloc3, tstr);
		}
	    }
    }

/*
 * General Utilities
 */

double max(a, b)
    double a, b;
    {
    if (a>b) return(a);
    return(b);
    }

double modup(a, b)
    double a, b;
    {
    double new;
    new = ((double)((int)(a/b))*b);
    if (new >= a) return(new);
    return(new += b);
    }

double moddown(a, b)
    double a, b;
    {
    double new;
    new = ((double)((int)(a/b))*b);
    if (new <= a) return(new);
    return (new -= b);
    }

die(a,b)
    char *a, *b;
    {
    fprintf(stderr,"Error: ");
    fprintf(stderr,a,b);
    fprintf(stderr,"\n");
    exit(1);
    }
