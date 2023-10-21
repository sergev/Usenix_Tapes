#ifndef BITMAP_HDR
#define BITMAP_HDR

#define	WORDSIZE	16

typedef struct bitmapStruct {
	int		height;
	int		width;
	int		depth;
	struct pixrect	*bitmap_pr;
} Bitmap;

typedef struct bitmap_hdrStruct {
	int		depth;
	int		height;
	int		format_version;
	int		valid_bits_per_item;
	int		width;
} Bitmap_hdr;

#define	NULL_BM		((Bitmap *) NULL)

extern FILE	*bm_open();
extern Bitmap	*bm_load();

#endif
