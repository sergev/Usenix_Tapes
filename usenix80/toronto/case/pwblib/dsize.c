char dsize_____[] "~|^`dsize.c:	2.1";
/*
	Transforms byte+integer file size as returned by stat(II) and
	fstat(II) to double precision number.
*/

double dsize(size0,size1)
char size0;
int size1;
{
	return(32.*1024.*(((size0&0377)<<1)+((size1>>15)&1))+(size1&077777));
}

