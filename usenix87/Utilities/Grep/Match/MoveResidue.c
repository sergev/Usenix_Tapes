#include "match.h"
/* Moves the text which has not been completely searched at the end of
* the buffer to the beginning of the buffer
* and returns number of bytes moved */
int MoveResidue(DescVec, NPats,Buffer, BuffEnd)
register struct PattDesc **DescVec;
int NPats;
char *Buffer, *BuffEnd;
{
	char *FirstStart;
	register char *Residue;
	/* use this declaration if you don't use "bcopy" */
	register char *f, *t;
	register int i;
	int ResSize;

	FirstStart = BuffEnd;
	/* find the earliest point which has not been
	* completely searched */
	for (i=0; i < NPats; i++) {
		FirstStart = 
			min(FirstStart, DescVec[i]-> Start );
	} /* for */
	/* now scan to the beginning of the line containing the
	* unsearched text */
	for (Residue = FirstStart; *Residue != '\n' &&
	Residue >= Buffer; Residue--);
	if (Residue < Buffer)
		Residue = FirstStart;
	else ++Residue;
	/* now move the residue to the beginning of
	* the file */
	ResSize = BuffEnd - Residue + 1;
	/* use this if you don't have "bcopy" */
	t = Buffer; f = Residue;
#ifdef BCOPY
	bcopy(Residue, Buffer, ResSize);
#else
	for(i=ResSize;i;--i)
		*t++ = *f++;
#endif
	/* now fix the status vectors */
	for (i=0; i < NPats; i++) {
		DescVec[i]->Start -= (Residue - Buffer);
	} /* for */
	return(ResSize);
} /* MoveResidue */
