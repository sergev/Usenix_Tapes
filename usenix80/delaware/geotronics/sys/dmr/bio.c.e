88c
	if ( rablkno && !incore(dev, rablkno)
		     && (bfreelist.av_forw != &bfreelist) )  {
.
2a
 * bio.c - block i/o routines
 *
 *	modified 03-Jun-1980 by D A Gwyn:
 *	1) fixed breada bug - read-ahead buffer deadlock.
.
w
q
