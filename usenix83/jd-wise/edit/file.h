/*
 * per file data
 */
struct filedata{
	/*
	 * temporary file information
	 */
	short	gottemp;	/* temp files have been created */
	short	tfile;		/* text temp file descriptor */
	short	sfile;		/* index temp file descriptor */
	char	tfname[16];	/* text temp file name */
	char	sfname[16];	/* index temp file name */
	short	iblock;		/* input block number */
	short	oblock;		/* output block number */
	char	*ibuff;		/* input buffer */
	char	*obuff;		/* output buffer */
	short	ichanged;	/* input buffer changed */
	short	nleft;
	short	tline;
	short	modflag;	/* file changed since last write */
	char	savedfile[FNSIZE];
	};

/* the files */
#define maxfiles 6
struct filedata filedata[maxfiles];
