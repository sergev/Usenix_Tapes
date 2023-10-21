/*
#
# $Source: /local/projects/X/fed/RCS/bitmap.c,v $
# $Header: bitmap.c,v 1.6 87/04/08 08:48:51 jim Exp $
#
#                     Copyright (c) 1987 Cognition Inc.
#
# Permission to use, copy, modify and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies, and that both
# that copyright notice and this permission notice appear in supporting
# documentation, and that the name of Cognition Inc. not be used in
# advertising or publicity pertaining to distribution of the software
# without specific, written prior permission.  Cognition Inc. makes no
# representations about the suitability of this software for any purpose.
# It is provided "as-is" without express or implied warranty.
#
#							  Jim Fulton
#							  Cognition Inc.
#                                                         900 Tech Park Drive
# uucp:  ...!{mit-eddie,talcott,necntc}!ci-dandelion!jim  Billerica, MA
# arpa:  jim@athena.mit.edu, fulton@eddie.mit.edu         (617) 667-4800
#
*/


/* $Header: bitmap.c,v 1.6 87/04/08 08:48:51 jim Exp $ */

/*
 * derived from XReadBitmapFile from X.10-2.
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#define boolean int

extern int errno;

/*
 * Read in a stream looking for a bitmap file and return a pointer to the
 * data and the length in rastersize.
 */

short *ReadBitmapFile (file, width, height, x_hot, y_hot, rastersize)
    FILE *file;
    register int *width, *height;
    int *x_hot, *y_hot;
    int *rastersize;
    {
    char variable[81];
    int status, value, i;
    short *raster;

    if (!file) {
	fprintf (stderr, 
	    "ReadBitmapFile: Null FILE pointer passed in.\n");
	return (NULL);
    }
    *width = *height = *x_hot = *y_hot = -1;
    while ((status = fscanf (file, "#define %s %d\n", variable, &value))==2)
    	{
	if (StringEndsWith (variable, "width"))
	    *width = value;
	else if (StringEndsWith (variable, "height"))
    	    *height = value;
	else if (StringEndsWith (variable, "x_hot"))
    	    *x_hot = value;
	else if (StringEndsWith (variable, "y_hot"))
    	    *y_hot = value;
    	 }

    if (*width < 0 || *height < 0) {
	errno = EINVAL;
    	return (NULL);
    	}
    
    *rastersize = ((*width+15)/16)*(*height);
    raster = (short *) malloc ((*rastersize)*sizeof(short));
    if (raster == NULL)
    	return (NULL);
    if (*rastersize == 0)
	return (raster);
    
    status = fscanf (file, "static short %80s = { 0x%4hx", variable,
	&raster[0]);
    if ((status != 2) || !StringEndsWith (variable, "bits[]")) {
    	errno = EINVAL;
	return (NULL);
	}

    for (i=1;i<(*rastersize);i++) {
	status = fscanf (file, ", 0x%4hx", &raster[i]);
	if (status != 1) {
	    free (raster);
	    errno = EINVAL;
	    return ((short *) NULL);
	    }
	}

    return (raster);
}

/* StringEndsWith returns TRUE if "s" ends with "suffix", else returns FALSE */
static boolean StringEndsWith (s, suffix)
  char *s, *suffix;
  {
  int s_len = strlen (s);
  int suffix_len = strlen (suffix);
  return (strcmp (s + s_len - suffix_len, suffix) == 0);
  }


short *ReadAsciiBitmapFile (file, width, height, x_hot, y_hot, rastersize)
    FILE *file;
    register int *width, *height;
    int *x_hot, *y_hot;
    int *rastersize;
    {
    char variable[81];
    int status, value, i, j, w, h, wpsl;
    short *raster;
    register unsigned short *data;
    char line[1024];
    char *cp, *savecp;
    int skipfirstdataread;	/* bool, t:skip first data read */

    if (!file) {
	fprintf (stderr, 
	    "ReadAsciiBitmapFile: Null FILE pointer passed in.\n");
	return (NULL);
    }
    *width = *height = *x_hot = *y_hot = -1;

#define skipspace(cp) while (isspace (*(cp))) (cp)++

    skipfirstdataread = 0;
    while (fgets (line, sizeof line, file) != NULL) {
	cp = line;
	skipspace (cp);
	if (cp[0] != '#') {	/* not beginning of keyword */
	    if (cp[0] == '-') 	/* but is beginning of data */
		skipfirstdataread = 1;
	    break;		/* break in any case */
	} else 			/* else does begin with # */
	if (cp[1] == '#' || cp[1] == '-') {	/* but is really data */
	    skipfirstdataread = 1;
	    break;
	}

	status = sscanf (cp, "#define %s %d\n", variable, &value);
	if (status != 2) break;

	if (StringEndsWith (variable, "width"))
	    *width = value;
	else if (StringEndsWith (variable, "height"))
    	    *height = value;
	else if (StringEndsWith (variable, "x_hot"))
    	    *x_hot = value;
	else if (StringEndsWith (variable, "y_hot"))
    	    *y_hot = value;
    }

    if (*width < 0 || *height < 0) {
	errno = EINVAL;
    	return (NULL);
    	}
    
    wpsl = (*width + 15) / 16;	/* words per scan line */
    *rastersize = wpsl * (*height);
    raster = (short *) malloc ((*rastersize)*sizeof(short));
    if (raster == NULL)
    	return (NULL);
    if (*rastersize == 0)
	return (raster);
    
				/* the interesting part, read in each */
				/* line, strip off blanks, and make sure */
				/* is the right width.  also make sure */
				/* that we get the right number of lines. */
    w = *width;
    h = *height;

    for (i = 0; i < h; i++) {
	if (i != 0 || !skipfirstdataread)
	  if (fgets (line, sizeof (line), file) == NULL) {
	      errno = EINVAL;
	      free (raster);
	      return (NULL);
	  } else
	    skipfirstdataread = 0;
	cp = line;
	skipspace (cp);		/* skip space at beginning of line */
	savecp = cp;		/* remember it for later */
				/* check line */
	for (j = 0; j < w; j++, cp++) {
	    if (*cp != '-' && *cp != '#') { /* valid char? */
		errno = EINVAL;
		free (raster);
		return (NULL);
	    }
	}
#ifdef EXACT_MATCH
				/* if there is any trailing stuff, punt */
	if (!isspace (*cp) && *cp && *cp != '\n') { /* trailing stuff? */
	    errno = EINVAL;
	    free (raster);
	    return (NULL);
	}
#endif
	*cp = '\0';		/* makes debugging easier */

				/* line is okay */
				/* now, convert it into binary data. */
				/* because each scan line is word aligned */
				/* we don't have to maintain state across */
				/* line compilation (except to where the */
				/* next words should be stored), i.e. no */
				/* masking hassles. we do it separately */
				/* since bits in word have to be swapped. */

	cp = savecp;
	data = (unsigned short *) &raster [wpsl * i];
	for (j = 0; j <= (w - 16); j += 16) {	/* iter by words */
	    register int n;
	    register unsigned short mask;
	    
	    cp = &savecp [j];	/* get the image for this word */
	    for (mask = 1, *data = 0, n = 0; n < 16; 
		 n++, mask = (mask << 1)) {
		if (cp[n] == '#') *data |= mask;
	    }
	    data++;		/* since we are doing 1 word at a time */
	}
				/* and do the remaining bits */

	if (j > w) {
	    fprintf (stderr,
	     "ReadAsciiBitmapFile: internal error, w = %d, j = %d, i = %d\n",
		     w, j, i);
	} else
	if (j < w) {	/* then there was stuff left over */
	    register int n;
	    register unsigned short mask;
	    
	    cp = &savecp[j];
	    for (mask = 1, *data = 0, n = 0; n < (w - j);
		 n++, mask = (mask << 1)) {
		if (cp[n] == '#') *data |= mask;
	    }
				/* and cp is now invalid */
	    data++;		/* since we are word aligned */
	}
     } /*rof - iter over lines*/

#ifdef EXACT_MATCH
				/* check for any trailing lines */
    while (fgets (line, sizeof (line), file) != NULL) {
	cp = line;
	skipspace (cp);
	if (*cp == '-' || *cp == '#') {
	    errno = EINVAL;
	    free (raster);
	    return (NULL);
	}
    }
#endif

    return (raster);
}
