/*	qfont.h		27 Nov 1985 */
#define	Q_FORMAT1	1
#define	Q_FORMAT2	2
#define Q_NODIMENSIONS	101
#define Q_WRONGHEIGHT	102
#define Q_NOOFFSET	103
#define Q_NOBITDATA	106
#define Q_NOSEPARATOR	107
#define Q_OUTOFSPACE	108
#define Q_RATIOERR	109
#define	Q_BADORIENTATION	110
#define	TOHEX(n)	((n)>9? 'A'+(n)-10: '0'+(n))

#define	qread(f,h,g)	qread_((f),(h),(g),1)
#define	qread0(f,h,g)	qread_((f),(h),(g),0)

struct q_header {
	unsigned short q_number;
	char q_orientation;
	char q_version;
	char q_name[5];
	short q_fheight;
	short q_baseline;
	short q_format;
};

struct q_glyph {
	unsigned char q_char;
	short q_spacing;
	short q_width;
	short q_height;
	short q_hoffset;
	short q_voffset;
	unsigned char *q_bitmap;
};

int qreadh(), qwriteh(), qwrite();
char *malloc();
#define	MALLOC(n)	((unsigned char *)malloc((long)(n)))
#define QMIN(x,y) ((x)<(y)? (x): (y))
#define QMAX(x,y) ((x)>(y)? (x): (y))
