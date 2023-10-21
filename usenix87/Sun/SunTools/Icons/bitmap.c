#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pixrect/pixrect_hs.h>
#include "bitmap.h"

#define HEADER_FORMAT	"/* Format_version=%d, Width=%d, Height=%d, Depth=%d, \
				Valid_bits_per_item=%d"

/*
 * Load bitmap header.
 */
extern FILE *
bm_open(filename, bmh_ptr)
char		*filename;
Bitmap_hdr	*bmh_ptr;
{
	register int	c;
	FILE		*fp;
	char		buf[BUFSIZ];
	struct stat	statbuf;

	/* stat(2) the file to make sure it is a ``regular'' file */
	if (stat(filename, &statbuf) == -1) {
		perror("stat");
		return(NULL);
	}
	if (statbuf.st_mode & S_IFMT != S_IFREG) {
		return(NULL);
	}
	/* open the file */
	if ((fp = fopen(filename, "r")) == NULL) {
		return(NULL);
	}
	/* read header information */
	for (;;) {
		/* get a line */
		if (fgets(buf, BUFSIZ, fp) == NULL) {
			(void) fclose(fp);
			return(NULL);
		}
		/* check to see if a '=' character appears in the line */
		if (index(buf, '=') == 0) continue;
		/* since the '=' was present, assume this line is the format */
		if (sscanf(buf, HEADER_FORMAT, &bmh_ptr->format_version, 
		    &bmh_ptr->width, &bmh_ptr->height, &bmh_ptr->depth, 
	    	    &bmh_ptr->valid_bits_per_item) != 5) {
			(void) fclose(fp);
			return(NULL);
		}
		break;
	} /* end for */
	/* read until we get past all the comments */
	while ((c = getc(fp)) != EOF && c != '\t');
	/* if <c> equals EOF the file is improperly formatted */
	if (c == EOF) {
		(void) fclose(fp);
		return(NULL);
	}
	/* return the file pointer */
	return(fp);
} /* end bm_open() */

/*
 * Load specific bitmap.
 */
extern Bitmap *
bm_load(filename)
char	*filename;
{
	register int		i, nitem;
	register u_int		*data, *data_ptr;
	Bitmap			*bm_ptr;
	Bitmap_hdr		bmh_buf;
	FILE			*fp;

	/* open icon file and read header information */
	if ((fp = bm_open(filename, &bmh_buf)) == NULL) {
		return(NULL_BM);
	}
	/* check to make sure we still are using version 1 */
	if (bmh_buf.format_version != 1) {
		(void) fclose(fp);
		return(NULL_BM);
	}
	/* compute the number of items */
	nitem = ((bmh_buf.width + WORDSIZE - 1) / WORDSIZE) * bmh_buf.height;
	/* create data space for bitmap */
	data_ptr = data = (u_int *) malloc(sizeof(u_int) * nitem);
	/* read data from file */
	for (i = 0; i < nitem; i++) {
		if (fscanf(fp, " 0x%X,", data_ptr++) != 1) {
			free(data);
			(void) fclose(fp);
			return(NULL_BM);
		}
	}
	/* create bitmap */
	bm_ptr = (Bitmap *) malloc(sizeof(Bitmap));
	/* initialize values */
	bm_ptr->width = bmh_buf.width;
	bm_ptr->height = bmh_buf.height;
	bm_ptr->depth = bmh_buf.depth;
	/* create bitmap pixrect */
	if ((bm_ptr->bitmap_pr = mem_create(bm_ptr->width, 
	    bm_ptr->height, bm_ptr->depth)) == NULL) {
		free(data);
		free(bm_ptr);
		(void) fclose(fp);
		return(NULL_BM);
	}
	/* put data into bitmap */
	data_ptr = (u_int *) mpr_d(bm_ptr->bitmap_pr)->md_image;
	for (i = ((nitem % 2 == 0) ? nitem : nitem + 1); i-- > 0; i--) {
		data_ptr[i / 2] = data[i];
		data_ptr[i / 2] |= (data[i - 1] << WORDSIZE);
	}
	free(data);
	(void) fclose(fp);
	return(bm_ptr);
} /* end bm_load() */
