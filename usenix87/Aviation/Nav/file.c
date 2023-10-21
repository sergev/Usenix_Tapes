/*
 *	Routines for reading and parsing nav data files
 *
 *	All degrees stored as decimal degrees.
 */
#include <stdio.h>
#include "nav.h"

double Radians();
double Degrees();
char *index();
char *malloc();
double atof();

struct vor *LineToVor(s)
char *s;
{
	static struct vor buf;
	register char *p, *q;
	register double min;

	p = s;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	strcpy(buf.id,p);		/* copy in the id */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	strcpy(buf.name,p);		/* copy in the name */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	buf.freq = (float)atof(p);	/* frequency */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	buf.alt = atoi(p);		/* vor altitude in feet */
	p = q+1;                        /* vortacs must be non-zero */
	q = index(p,':');               /* vors must be zero */
	if (q == NULL)
		return NULL;
	*q = 0;
	buf.var = atof(p);		/* variation in degrees */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	min = atof(p) / 60;		/* add variation minutes */
	buf.var += buf.var<0 ? (0. - min) : min;
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	buf.loc.lat.deg = atof(p);	/* north degrees */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	min = atof(p) / 60;		/* add north minutes */
	buf.loc.lat.deg += buf.loc.lat.deg<0 ? (0. - min) : min;
	buf.loc.lat.rad = Radians(buf.loc.lat.deg);
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	buf.loc.lon.deg = atof(p);	/* west degrees */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	min = atof(p) / 60;		/* add west minutes */
	buf.loc.lon.deg += buf.loc.lon.deg<0 ? (0. - min) : min;
	buf.loc.lon.rad = Radians(buf.loc.lon.deg);
	p = q+1;
	if (! *p)
		return NULL;
	strncpy(buf.comments,p,sizeof buf.comments);
	q = index(buf.comments,'\n');
	q = 0;
	return &buf;
}

struct apt *LineToApt(s)
char *s;
{
	static struct apt buf;
	register char *p, *q;
	register double min;

	p = s;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	strcpy(buf.id,p);		/* copy in the id */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	strcpy(buf.city,p);		/* city closest to airport */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	strcpy(buf.name,p);		/* name of airport */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	buf.alt = atoi(p);		/* airport altitude in feet */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	buf.var = atof(p);		/* variation in decimal degrees */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	min = atof(p) / 60;		/* add variation minutes */
	buf.var += buf.var<0 ? (0. - min) : min;
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	buf.loc.lat.deg = atof(p);	/* north degrees */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	min = atof(p) / 60;		/* add north minutes */
	buf.loc.lat.deg += buf.loc.lat.deg<0 ? (0. - min) : min;
	buf.loc.lat.rad = Radians(buf.loc.lat.deg);
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	buf.loc.lon.deg = atof(p);	/* west degrees */
	p = q+1;
	q = index(p,':');
	if (q == NULL)
		return NULL;
	*q = 0;
	min = atof(p) / 60;		/* add west minutes */
	buf.loc.lon.deg += buf.loc.lon.deg<0 ? (0. - min) : min;
	buf.loc.lon.rad = Radians(buf.loc.lon.deg);
	p = q+1;
	if (! *p)
		return NULL;
	strncpy(buf.comments,p,sizeof buf.comments);
	q = index(buf.comments,'\n');
	q = 0;
	return &buf;
}

/*
 *	Fill array of struct apt with data from file.
 *	Return number of elements in array, 0 on error.
 */
ParseApt(array,fp)
struct apt array[];
FILE fp;
{
	char line[BUFSIZ];
	register char *s;
	register struct apt *ptr;
	register int cnt;

	cnt = 0;
	while ((cnt <= MAXAPTS) && (fgets(line, sizeof line, fp) != NULL)){
		if ( line[0] == '#' ){	/* comment line */
			continue;
		}
		ptr = LineToApt(line);
		if (ptr == NULL){
			fprintf(stderr,
			"LineToApt from ParseApt returned NULL on line\n%s\n",
			line);
			return NULL;
		}
		array[cnt] = *ptr;
		cnt++;
	}
	return cnt;
}

/*
 *	Fill array of struct vor with data from file.
 *	Return number of elements in array, 0 on error.
 */
ParseVor(array,fp)
struct vor array[];
FILE fp;
{
	char line[BUFSIZ];
	register char *s;
	register struct vor *ptr;
	register int cnt;

	cnt = 0;
	while ((cnt <= MAXVORS) && (fgets(line, sizeof line, fp) != NULL)){
		if ( line[0] == '#' ){	/* comment line */
			continue;
		}
		ptr = LineToVor(line);
		if (ptr == NULL){
			fprintf(stderr,
			"LineToVor from ParseVor returned NULL on line\n%s\n",
			line);
			return NULL;
		}
		array[cnt] = *ptr;
		cnt++;
	}
	return cnt;
}
