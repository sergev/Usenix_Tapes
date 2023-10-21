#include <stdio.h>
#include "make.h"
#ifdef VAXVMS
#include <rms.h>
#endif


/*
 * Get a file's creation date.
 */
int getdate(f)
FILENODE *f;
{
	if(f->fdate != NULL || filedate(f) != -1) return;

	if(f->fflag & ROOTP == 0)
	{
		fprintf(stderr, "Can't get date for file '%s'\n", f->fname);
		f->fdate = endoftime;
	} else f->fdate = bigbang;
	return;
}


#ifdef VAXVMS
/*
 * filedate - return file's creation date (VAX/VMS only.)
 * Returns -1 if file cannot be found, 0 if succesful.
 */
filedate(fnd)
FILENODE *fnd;
{
	unsigned *datetime;
	DATE adate();
	struct FAB *fptr;
	struct XABDAT *dptr;

	fptr = malloc(sizeof(struct FAB));	/* allocate FAB and XABDAT */
	dptr = malloc(sizeof(struct XABDAT));
	if(fptr == NULL || dptr == NULL) allerr();
	*fptr = cc$rms_fab;			/* initialize FAB and XABDAT */
	*dptr = cc$rms_xabdat;
	fptr->fab$l_xab = (char *) dptr;	/* FAB -> XABDAT */

	fptr->fab$l_fna = fnd->fname;		/* setup filename */
	fptr->fab$b_fns = strlen(fnd->fname);

	if(sys$open(fptr) != RMS$_NORMAL ||	/* open the file */
	   sys$display(fptr) != RMS$_NORMAL)	/* get XABDAT info */
		return -1;

	datetime = &(dptr->xab$q_cdt);		/* record 64-bit date */
	fnd->fdate = adate(datetime[0], datetime[1]);

	sys$close(fptr);			/* close the file */

	free(dptr);				/* clean up and return */
	free(fptr);
	return 0;
}
#endif


#ifdef MSDOS
/*
 * filedate - return file's creation date (MSDOS only.)
 * Returns -1 if file cannot be found, 0 if successful
 */
filedate(fnd)
FILENODE *fnd;
{
	unsigned date, time;
	DATE adate();

	if(osdate(fnd->fname, &time, &date) == -1) return -1;
	fnd->fdate = adate(time, date);
}
#endif


/*
 * laterdt - compare two dates.
 * Return -1, 0 or 1 if date1 < date2, date1 == date2, or date1 > date2
 */
laterdt(date1, date2)
DATE date1, date2;
{
	if(date1->ds_high > date2->ds_high ||
	   (date1->ds_high >= date2->ds_high &&
	    date1->ds_low > date2->ds_low)) return 1;
	else if(date1->ds_high == date2->ds_high &&
	   date1->ds_low == date2->ds_low) return 0;
	else return -1;
}


/*
 * adate - allocate a date with the given time
 */
DATE adate(time1, time2)
unsigned time1, time2;
{
	DATE d;

	if((d = (DATE)malloc(sizeof(struct date_str))) == NULL) allerr();
	d->ds_low = time1;
	d->ds_high = time2;
	return d;

}
