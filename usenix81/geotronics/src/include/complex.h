/*
	complex.h -- header for complex arithmetic library routines


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

09-May-1981	D A Gwyn	Created.


Usage:
	See source listing for "complx.c"
*/

typedef struct
	{
	double	r ;
	double	i ;
	}	complex ;		// complex number

extern	complex	*CAdd();
extern	complex	*CConjug();
extern	complex	*CConst();
extern	complex	*CCopy();
extern	complex	*CDiv();
extern	complex	*CMul();
extern	complex	*CPhasor();
extern	complex	*CScale();
extern	complex	*CSub();
extern	double	Modulus();
extern	double	Phase();
