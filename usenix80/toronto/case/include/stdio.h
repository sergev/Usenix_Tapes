#define	BUFSIZ	512
#define	_NFILE	20
# ifndef FILE
extern	struct	_iobuf {
	char	*_ptr;
	int	_cnt;
	char	*_base;
	char	_flag;
	char	_file;
} _iob[_NFILE];
# endif

#define	_IOREAD	01
#define	_IOWRT	02
#define	_IONBF	04
#define	_IOMYBUF	010
#define	_IOEOF	020
#define	_IOERR	040
#define	_IOSTRG	0100
#define	NULL	0
#define	FILE	struct _iobuf
#define	EOF	(-1)

#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])
#define	getc(p)		(--(p)->_cnt>=0? *(p)->_ptr++&0377:_filbuf(p))
#define	getchar()	getc(stdin)
#define	putc(x,p)	(--(p)->_cnt>=0? (*(p)->_ptr++=(x)):_flsbuf((x),p))
#define	putchar(x)	putc(x,stdout)
#define	feof(p)		(((p)->_flag&_IOEOF)!=0)
#define	ferror(p)	(((p)->_flag&_IOERR)!=0)
#define	fileno(p)	p->_file

FILE	*fopen();
FILE	*freopen();

#define	_U	01
#define	_L	02
#define	_A	03
#define	_N	04
#define	_S	010

extern	char	_ctype[];

#define	isalpha(c)	(_ctype[c]&_A)
#define	isupper(c)	(_ctype[c]&_U)
#define	islower(c)	(_ctype[c]&_L)
#define	isdigit(c)	(_ctype[c]&_N)
#define	isspace(c)	(_ctype[c]&_S)
#define	toupper(c)	((c)-'a'+'A')
#define	tolower(c)	((c)-'A'+'a')
