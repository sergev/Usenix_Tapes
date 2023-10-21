#include "sys/types.h"
#include "sys/times.h"
#include "stdio.h"

#define NOTEST 23
#define NULL 0

FILE * stream;

/* add short test */
	
adds(n)
	int n;
{
	register short sa,sb,*psa,*psb;
	short sx,sy,asx[32],asy[32],*psx,*psy;
	static short s1,s2,as1[32],as2[32],*ps1,*ps2;
	register int j;
	register int s = 0;

	j = s + 1;
	sa = s + 5;
	sb = s + 10;
	sy = s + 4;
	sx = s + 5;
	asx[1] = s + 7;
	asy[1] = s + 3;
	s1 = s;
	s2 = s + 9;
	as1[1] = s + 8;
	as2[1] = s + 7;
	
	psx = &sx;
	psb = &sy;
	
	while(n--)
	{
		s1 = s1 + sb + sx + sy + sa + sb + sx + sy + sa + sb + sx + sy
			+ sa + sb + sx + sy + sa + sb + sx + sy + sa + sb + s2
			+ sa + sa + sb + s2 + (*psb) + asx[j] + asy[j] + as1[j]
			+ as2[j] + (*psx);
	}
	return(s1);
}
	
/* substract short test */

subs(n)
	int n;
{
	register short sa,sb,*psa,*psb;
	short sx,sy,asx[32],asy[32],*psx,*psy;
	static short s1,s2,as1[32],as2[32],*ps1,*ps2;
	register int j;
	register s = 0;

	j = s + 1;
	sa = s + 5;
	sb = s + 3;
	sy = s + 5;
	sx = s + 6;
	asx[1] = s + 7;
	asy[1] = s + 4;
	s1 = s + 32000;
	s2 = s + 2;
	as1[1] = s + 3;
	as2[1] = s + 7;
	
	psx = &sx;
	psb = &sy;
	
	while(n--)
	{
		s1 = s1 - sb - sx - sy - sa - sb - sx - sy - sa - sb - sx - sy
			- sa - sb - sx - sy - sa - sb - sx - sy - sa - sb - s2
			- sa - sa - sb - s2 - (*psb) - asx[j] - asy[j] - as1[j]
			- as2[j] - (*psx);
	}
	return(s1);
}

/* multiply short */

muls(n)
	int n;
{
	register short sa,sb,*psa,*psb;
	short sx,sy,asx[32],asy[32],*psx,*psy;
	static short s1,s2,as1[32],as2[32],*ps1,*ps2;
	register int j;
	register int s = 0;

	j = s + 1;
	sa = 2;
	sb = s + 3;
	sx = s + 2;
	sy = s + 5;
	s2 = s + 4;
	asx[1] = s + 1;
	as1[1] = s + 7;
	asy[1] = s + 6;
	as2[1] = s + 8;
	s1 = s + 1;
	
	psx = &sx;
	psb = &sy;
	
	while(n--)
	{
		s1 = sa * sb * sx * sy * (1+(s2 >> 12)) * (*psb) * asx[j] * as1[j] * (1+(s1 >> 12));
		s2 = sa * as2[j] * asy[j] * (*psx) * sy * sx * (1+(s1 >> 12)) * ++s2 * sb;
		s1 = sa * sb * sx * sy * (1+(s2 >> 12)) * (*psb) * asx[j] * as1[j] * (1+(s1 >> 12));
		s2 = sa * as2[j] * asy[j] * (*psx) * sy * sx * (1+(s1 >> 12)) * ++s2 * sb;
	}
	return(s2);
}

/* divide short test */

divs(n)
	int n;
{
	register short sa,sb,*psa,*psb;
	short sx,sy,asx[32],asy[32],*psx,*psy;
	static short s1,s2,as1[32],as2[32],*ps1,*ps2;
	register int j;
	register int s = 0;

	j = s + 1;
	sa = 32000;
	sb = s + 3;
	sx = s + 2;
	sy = s + 5;
	s2 = s + 4;
	asx[1] = s + 1;
	as1[1] = s + 7;
	asy[1] = s + 6;
	as2[1] = s + 8;
	s1 = s + 1;
	
	psx = &sx;
	psb = &sy;
	
	while(n--)
	{
		s1 = sa / sb / sx / sy / ++s2 / (*psb) / asx[j] / as1[j] / ++s1;
		s2 = sa / as2[j] / asy[j] / (*psx) / sy / sx / ++s1 / ++s2 / sb;
		s1 = sa / sb / sx / sy / ++s2 / (*psb) / asx[j] / as1[j] / ++s1;
		s2 = sa / as2[j] / asy[j] / (*psx) / sy / sx / ++s1 / ++s2 / sb;
	}
	return(s2);
}

/* assign short test */

asgns(n)
	int n;
{
	register short sa,sb,*psa,*psb;
	short sx,sy,asx[32],asy[32],*psx,*psy;
	static short s1,s2,as1[32],as2[32],*ps1,*ps2;
	register int j;
	register int s = 0;

	j = s + 1;
	sa = s + 5;
	sb = s + 10;
	sy = s + 15;
	sx = s + 19;
	asx[1] = s + 21;
	asy[1] = s + 27;
	s1 = s + 9;
	s2 = s + 17;
	as1[1] = s + 13;
	as2[1] = s + 217;
	
	psx = &sx;
	psb = &sy;
	
	while(n--)
	{
		sa = s2; sx = sa; sy = sx; s2 = sx; s1 = s2;
		sb = s1; sa = sb; sx = sa; sy = sx; sb = sy;
		*psb = sb; sa = *psb; asx[j] = sa; sx = asx[j];
		asy[j] = sx; sb = asy[j]; *psx = sb; s2 = *psx;
		s1 = s2; 
		sb = s1; sa = sb; sx = sa; sy = sx; sb = sy;
		*psb = sb; sa = *psb; asx[j] = sa; sx = asx[j];
		asy[j] = sx; sb = asy[j]; *psx = sb; s2 = *psx;
	}
	return(s2);
}

/* compare short test */

cmps(n)
	int n;
{
	register short sa,sb,*psa,*psb;
	short sx,sy,asx[32],asy[32],*psx,*psy;
	static short s1,s2,as1[32],as2[32],*ps1,*ps2;
	register int j;
	register int s = 0;

	sb = sy =s2 = asy[1] = as2[1] = n - 1;
	j = s + 1;
	
	psx = &sx;
	psa = &sy;
	
	while(n--)
	{
		sa = sx = s1 = asx[1] = as1[1] = n;
		if( sa == sb ) s++;
		if( sa == sb ) s++;
		if( sa == sb ) s++;
		if( sa == sb ) s++;
		if( sa == sb ) s++;
		if( sa == sb ) s++;
		if( sa == sb ) s++;
		if( sx == sy ) s++;
		if( sx == sy ) s++;
		if( sx == sy ) s++;
		if( sx == sy ) s++;
		if( s1 == s2 ) s++;
		if( *psa == *psx ) s++;
		if( asx[j] == as1[j] ) s++;
		sb = sy =s2 = asy[1] = as2[1] = n - 1;
		if( sa != sb ) s++;
		if( sa != sb ) s++;
		if( sx != sy ) s++;
		if( sx != sy ) s++;
		if( asx[j] != asy[j] ) s++;
		if( s1 != s2 ) s++;
		if( sb < sa ) s++;
		if( sb < sa ) s++;
		if( sy < sx ) s++;
		if( sy < sx ) s++;
		if( sa > sb ) s++;
		if( sx > sy ) s++;
		if( sb <= sx ) s++;
		if( sb <= sx ) s++;
		if( sa >= sb ) s++;
		if( sa >= sb ) s++;
		if( sx >= sy ) s++;
		if( sx >= sy ) s++;
	}
	return(s);
}


/* add long test */

addl(n)
	int n;
{
	register int i = 0;
	register long ia,ib,*pia,*pib;
	long ix,iy,aix[32],aiy[32],*pix,*piy;
	static long i1,i2,ai1[32],ai2[32],*pi1,*pi2;
	register int j;

	j = i + 1;
	ia = i + 5;
	ib = i + 10;
	iy = i + 15;
	ix = i + 19;
	aix[1] = i + 21;
	aiy[1] = i + 27;
	i1 = i;
	i2 = i + 17;
	ai1[1] = i + 13;
	ai2[1] = i + 7;
	
	pix = &ix;
	pib = &iy;
	
	while(n--)
	{
		i1 = i1 + ib + ix + iy + ia + ib + ix + iy + ia + ib + ix + iy
			+ ia + ib + ix + iy + ia + ib + ix + iy + ia + ib + i2
			+ ia + ia + ib + i2 + (*pib) + aix[j] + aiy[j] + ai1[j]
			+ ai2[j] + (*pix);
	}
	return(i1);
}

/* substract long test */

subl(n)
	int n;
{
	register int i = 0;
	register long ia,ib,*pia,*pib;
	long ix,iy,aix[32],aiy[32],*pix,*piy;
	static long i1,i2,ai1[32],ai2[32],*pi1,*pi2;
	register int j;

	j = i + 1;
	ia = i + 5;
	ib = i + 10;
	iy = i + 15;
	ix = i + 19;
	aix[1] = i + 21;
	aiy[1] = i + 27;
	i1 = i + 917672934;
	i2 = i + 17;
	ai1[1] = i + 13;
	ai2[1] = i + 7;
	
	pix = &ix;
	pib = &iy;
	
	while(n--)
	{
		i1 = i1 - ib - ix - iy - ia - ib - ix - iy - ia - ib - ix - iy
			- ia - ib - ix - iy - ia - ib - ix - iy - ia - ib - i2
			- ia - ia - ib - i2 - (*pib) - aix[j] - aiy[j] - ai1[j]
			- ai2[j] - (*pix);
	}
	return(i);
}

/* multiply long test */

mull(n)
	int n;
{
	register int i = 0;
	register long ia,ib,*pia,*pib;
	long ix,iy,aix[32],aiy[32],*pix,*piy;
	static long i1,i2,ai1[32],ai2[32],*pi1,*pi2;
	register int j;

	j = i + 1;
	ia = 2;
	ib = i + 3;
	ix = i + 2;
	iy = i + 5;
	i2 = i + 4;
	aix[1] = i + 1;
	ai1[1] = i + 7;
	aiy[1] = i + 6;
	ai2[1] = i + 8;
	i1 = i + 1;
	
	pix = &ix;
	pib = &iy;
	
	while(n--)
	{
		i1 = ia * ib * ix * iy * (1+(i2 >> 12)) * (*pib) * aix[j] * ai1[j] * (1+(i1 >> 12 ));
		i2 = ia * ai2[j] * aiy[j] * (*pix) * iy * ix * (1+(i1 >> 12)) * (1+(i2 >> 12)) * ib;
		i1 = ia * ib * ix * iy * (1+(i2 >> 12)) * (*pib) * aix[j] * ai1[j] * (1+(i1 >> 12 ));
		i2 = ia * ai2[j] * aiy[j] * (*pix) * iy * ix * (1+(i1 >> 12)) * (1+(i2 >> 12)) * ib;
	}
	return(i2);
}

/* divide long test */

divl(n)
	int n;
{
	register int i = 0;
	register long ia,ib,*pia,*pib;
	long ix,iy,aix[32],aiy[32],*pix,*piy;
	static long i1,i2,ai1[32],ai2[32],*pi1,*pi2;
	register int j;

	j = i + 1;
	ia = 2072397218;
	ib = i + 3;
	ix = i + 2;
	iy = i + 5;
	i2 = i + 4;
	aix[1] = i + 1;
	ai1[1] = i + 7;
	aiy[1] = i + 6;
	ai2[1] = i + 8;
	i1 = i + 1;
	
	pix = &ix;
	pib = &iy;
	
	while(n--)
	{
		i1 = ia / ib / ix / iy / ++i2 / (*pib) / aix[j] / ai1[j] / ++i1;
		i2 = ia / ai2[j] / aiy[j] / (*pix) / iy / ix / ++i1 / ++i2 / ib;
		i1 = ia / ib / ix / iy / ++i2 / (*pib) / aix[j] / ai1[j] / ++i1;
		i2 = ia / ai2[j] / aiy[j] / (*pix) / iy / ix / ++i1 / ++i2 / ib;
	}
	return(i2);
}

/* assign long test */

asgnl(n)
	int n;
{
	register int i = 0;
	register long ia,ib,*pia,*pib;
	long ix,iy,aix[32],aiy[32],*pix,*piy;
	static long i1,i2,ai1[32],ai2[32],*pi1,*pi2;
	register int j;

	j = i + 1;
	ia = i + 5;
	ib = i + 10;
	iy = i + 15;
	ix = i + 19;
	aix[1] = i + 21;
	aiy[1] = i + 27;
	i1 = i + 9;
	i2 = i + 17;
	ai1[1] = i + 13;
	ai2[1] = i + 217;
	
	pix = &ix;
	pib = &iy;
	
	while(n--)
	{
		ia = i2; ix = ia; iy = ix; i2 = ix; i1 = i2;
		ib = i1; ia = ib; ix = ia; iy = ix; ib = iy;
		*pib = ib; ia = *pib; aix[j] = ia; ix = aix[j];
		aiy[j] = ix; ib = aiy[j]; *pix = ib; i2 = *pix;
		i1 = i2; 
		ib = i1; ia = ib; ix = ia; iy = ix; ib = iy;
		*pib = ib; ia = *pib; aix[j] = ia; ix = aix[j];
		aiy[j] = ix; ib = aiy[j]; *pix = ib; i2 = *pix;
	}
	return(i2);
}

/* compare long test */

cmpl(n)
	int n;
{
	register int i = 0;
	register long ia,ib,*pia,*pib;
	long ix,iy,aix[32],aiy[32],*pix,*piy;
	static long i1,i2,ai1[32],ai2[32],*pi1,*pi2;
	register int j;

	ib = iy =i2 = aiy[1] = ai2[1] = n - 1;
	j = i + 1;
	
	pix = &ix;
	pia = &iy;
	
	while(n--)
	{
		ia = ix = i1 = aix[1] = ai1[1] = n;
		if( ia == ib ) i++;
		if( ia == ib ) i++;
		if( ia == ib ) i++;
		if( ia == ib ) i++;
		if( ia == ib ) i++;
		if( ia == ib ) i++;
		if( ia == ib ) i++;
		if( ix == iy ) i++;
		if( ix == iy ) i++;
		if( ix == iy ) i++;
		if( ix == iy ) i++;
		if( i1 == i2 ) i++;
		if( *pia == *pix ) i++;
		if( aix[j] == ai1[j] ) i++;
		ib = iy =i2 = aiy[1] = ai2[1] = n - 1;
		if( ia != ib ) i++;
		if( ia != ib ) i++;
		if( ix != iy ) i++;
		if( ix != iy ) i++;
		if( aix[j] != aiy[j] ) i++;
		if( i1 != i2 ) i++;
		if( ib < ia ) i++;
		if( ib < ia ) i++;
		if( iy < ix ) i++;
		if( iy < ix ) i++;
		if( ia > ib ) i++;
		if( ix > iy ) i++;
		if( ib <= ix ) i++;
		if( ib <= ix ) i++;
		if( ia >= ib ) i++;
		if( ia >= ib ) i++;
		if( ix >= iy ) i++;
		if( ix >= iy ) i++;
	}
	return(i);
}

/* bit long test */

bitl(n)
	int n;
{
	register int i;
	register long ia,ib,*pia,*pib;
	long ix,iy,aix[32],aiy[32],*pix,*piy;
	static long i1,i2,ai1[32],ai2[32],*pi1,*pi2;
	register int j = 1;

	i = j + 7;
	ia = i + 5;
	ib = i + 10;
	iy = i + 15;
	ix = i + 19;
	aix[1] = i + 21;
	aiy[1] = i + 27;
	i1 = i + 9;
	i2 = i + 17;
	ai1[1] = i + 13;
	ai2[1] = i + 217;
	
	pix = pia = pi1 = &ix;
	piy = pib = pi2 = &iy;
	
	while(n--)
	{
		i = 2;
		while(i--)
			i1 = ia & ib | (ix >> 1) & (~iy) | (i1 << 2) & i2 |
				(*pia) & (*pib) | aix[j] | aiy[j] |
				*pix | *piy | (~(*pix));
	}
	return(i1);
}

/* logical and or not test */

logical(n)
	int n;
{
	register int i = 0;
	register long ia,ib;
	long ix,iy;
	static long i1,i2;
	
	i1 = ib =ix =iy = i + 1;
	i2 = ia = ++i;
	while(n--)
	{
		i = 2;
		while(i--)
		i1 = ia && ib || ix && iy || ia && ib ||
			(!i1) && (!i2) || ix && iy || ia && ix
			&& iy && (!i1);
	}
	return(i1);
}

/* character assign test */

asgnc(n)
	int n;
{
	register int i = 0;
	register char ca,cb,*pca,*pcb;
	char cx,cy,acx[32],acy[32],*pcx,*pcy;
	static char ac1[32],ac2[32],*pc1,*pc2;
	static char c1 = 'a';
	static char c2 = 'b';
	register int j;

	pca = pcx = pc1 = &c1;
	pcb = pcy = pc2 = &c2;
	ca = acx[1] = ac1[1] = c2;
	cb = acy[1] = ac2[1] = c1;
	j = i + 1;
	
	while(n--)
	{
		*pca = c1; *pcb = *pca; *pcx = *pcb; *pcy = *pcx;
		*pc1 = *pcy; *pc2 = *pc1; acx[j] = *pc2; acy[j] = acx[j];
		ac1[j] = acy[j]; ac2[j] = ac1[j]; ca = ac2[j]; cb = ca;
		cx = cb; cy = cx; *pca = cy; *pcb = *pca; *pcx = *pcb;
		*pcy = *pcx; *pc1 = *pcy; acx[j] = *pc1; acy[j] = acx[j];
		ac1[j] = acy[j]; ac2[j] = ac1[j];
		*pcx = ac2[j]; *pcb = *pcx;
		acx[j] = *pcb; acy[j] = acx[j]; *pca = acy[j]; *pcb = *pca;
		ca = *pcb; *pca = ca; c1 = *pca;
	}
	return((int)c1);
}

/* character compare test */

cmpc(n)
	int n;
{
	register int i = 0;
	register char ca,cb,*pca,*pcb;
	char cx,cy,acx[32],acy[32],*pcx,*pcy;
	static char ac1[32],ac2[32],*pc1,*pc2;
	static char c1 = 'a';
	static char c2 = 'a';
	register int j;

	j = i + 1;
	pca = pcx = pc1 = &c1;
	pcb = pcy = pc2 = &c2;
	cb = acy[1] = ac2[1] = c1;
	
	while(n--)
	{
		ca = acx[1] = ac1[1] = c1 = c2;
		if( ca == cb ) i++;
		if( cx == cy ) i++;
		if( acx[j] == acy[j] ) i++;
		if( acx[j] == acy[j] ) i++;
		if( acx[j] == acy[j] ) i++;
		if( ac1[j] == ac2[j] ) i++;
		if( ac1[j] == ac2[j] ) i++;
		if( *pca == *pcb ) i++;
		if( *pca == *pcb ) i++;
		if( *pca == *pcb ) i++;
		if( *pca == *pcb ) i++;
		if( *pcx == *pcy ) i++;
		if( *pcx == *pcy ) i++;
		ca = acx[1] = ac1[1] = c1 = 'b';
		if( ca != cb ) i++;
		if( acx[j] != acy[j] ) i++;
		if( ac1[j] != ac2[j] ) i++;
		if( *pca != *pcb ) i++;
		if( *pca != *pcb ) i++;
		if( *pcx != *pcy ) i++;
		if( cb < ca ) i++;
		if( ac2[j] < ac1[j] ) i++;
		if( *pcb < *pca ) i++;
		if( *pcy < *pcx ) i++;
		if( acx[j] > ac2[j] ) i++;
		if( *pca > *pcb ) i++;
		if( *pcb <= *pcx ) i++;
		if( acx[j] >= acy[j] ) i++;
		if( *pca >= *pcb ) i++;
		if( *pca > *pc2 ) i++;
		if( *pcx >= *pcb ) i++;
	}
	return(i);
}

/* character bit test */

bitc(n)
	int n;
{
	register int i;
	register char ca,cb,*pca,*pcb;
	char cx,cy,acx[32],acy[32],*pcx,*pcy;
	static char ac1[32],ac2[32],*pc1,*pc2;
	static char c1 = 'A';
	static char c2 = 'B';
	register int j = 1;

	pca = pcx = pc1 = &c1;
	pcb = pcy = pc2 = &c2;
	ca = cx = acx[1] = ac1[1] = c1;
	cb = cy = acy[1] = ac2[1] = c2;
	c2 = ca;
	
	while(n--)
	{
		c1 = ca & cb & cx
			& acx[j] & acy[j] & cy
			& *pca & *pcb & *pcx
			& *pcy & *pc1 & *pc2
			| ca | cb
			| cx | cy | acx[j]
			| acy[j] | ac1[j] | ac2[j]
			| *pca | *pcb | *pcx
			| *pcy | *pc1 | *pc2
			| (ca << 1) | (cb >> 2) | ~c2
			| c1 ;
	}
	return((int)c1);
}

/* function call no parameters */

funct0(n)
	int n;
{
	register int  i,j;

	j = 0;
	while (n--)
	{
		i = 2;
		while(i--)
		{
		      j += func0( ); j += func0( ); j += func0( ); j += func0( );
		      j += func0( ); j += func0( ); j += func0( ); j += func0( );
		      j += func0( ); j += func0( ); j += func0( ); j += func0( );
		      j += func0( ); j += func0( ); j += func0( ); j += func0( );
		}
	}
	return(j);
}
func0()
{
	static int j = 0;
	return(++j);
}

/* function call one parameter test */

funct1(n)
	int n;
{
	register int  i,j;

	j = 0;
	while (n--)
	{
		i = 2;
		while(i--)
		{
		      j += func1(n); j += func1(n); j += func1(n); j += func1(n);
		      j += func1(n); j += func1(n); j += func1(n); j += func1(n);
		      j += func1(n); j += func1(n); j += func1(n); j += func1(n);
		      j += func1(n); j += func1(n); j += func1(n); j += func1(n);
		}
	}
	return(j);
}
func1(n)
	int n;
{
	return(++n);
}

/* function call two prameters test */

funct2(n)
	int n;
{
	register int  i,j;

	j = 0;
	while (n--)
	{
		i = 2;
		while(i--)
		{
		      j += func2(i,n); j += func2(i,n); j += func2(i,n); j += func2(i,n);
		      j += func2(i,n); j += func2(i,n); j += func2(i,n); j += func2(i,n);
		      j += func2(i,n); j += func2(i,n); j += func2(i,n); j += func2(i,n);
		      j += func2(i,n); j += func2(i,n); j += func2(i,n); j += func2(i,n);
		}
	}
	return(j);
}
func2(i,j)
	int i,j;
{
	return(i+j);
}


/* fork test */

sys1(n)
	int n;
{
	register int c;
	int status;
	
	while(n--)
	{
		c = fork( );
		if( c == 0 )
			_exit(0);
		if( c < 0 )
		{
			printf("unable to perform fork\n");
			return;
		}
		if( c != wait(&status) )
		{
			printf("wait does not return the correct child process id\n");
			return;
		}
	}
}

/* getpid test */

sys3(n)
	int n;
{
	register int i;
	while(n--)
		i = getpid( );
	return(i);
}

/* sbrk test */
	
sys4(n)
	int n;
{
	register int i;
	while(n--)
		i = sbrk(0);
	return(i);
}
sys5(n)
	int n;
{
	register int i;
	while(n--)
		i = umask(0);
	return(i);
}
static long start,end;
static double stime,ltime,factor;
static double ddelta,dstep,ddcnt;
static double tdelta,tstep,ttcnt;
static int nd,nt,nl,dd,td,i,ddcount,ttcount,lflag;
struct teststru
{
	int nops;
	int (*funcall)( );
};
static struct teststru test[NOTEST] =
{
	117,adds,
	93,subs,
	11,muls,
	5,divs,
	189,asgns,
	141,cmps,
	949,addl,
	751,subl,
	91,mull,
	43,divl,
	1521,asgnl,
	1135,cmpl,
	689,bitl,
	465,logical,
	878,asgnc,
	655,cmpc,
	354,bitc,
	593,funct0,
	976,funct1,
	344,funct2,
	1,sys3,
	1,sys4,
	1,sys5
};
main( )
{
	static char machine[100],s[100];
	static int noproc;
	static int nomp,novol;
	{
	register int j,n,i;

	if( (stream = fopen("config","r")) == NULL )
	{
		printf("unable to open file config\n");
		exit(0);
	}
	i = 1;
	while( i <=3 )
	{
		if( fgets(s,100,stream) == NULL )
		{
			printf("comfig file is empty\n");
			exit(1);
		}
		if( s[0] == '#' ) continue;
		switch(i)
		{
		case 1:
			sscanf(s,"%d",&nomp);
			break;
		case 2:
			sscanf(s,"%d",&noproc);
			break;
		case 3:
			sscanf(s,"%d",&novol);
			break;
		}
		i++;
	}
	fclose(stream);
	readmac:
	printf("ENTER THE NAME OR DESCRIPTION OF THIS MACHINE (5 TO 40 CHARTERS)\n$");
	gets(machine);
	if( (strlen(machine) < 5)  || (strlen(machine) > 40) )
	{
		printf("THE MACHINE NAME OR DESCRIPTION HAS AN INCORRECT LENGTH\n");
		goto readmac;
	}
	if( (stream = fopen("answers","w")) == NULL )
	{
		printf("unable to open file answers\n");
		exit(1);
	}
	fprintf(stream,"%s\n",machine);
	fprintf(stream,"%d\n",noproc);
	fclose(stream);
	ptime(1);
	start = ptime(1);
	for(j=0; j<NOTEST; j++)
	{
		n = test[j].nops;
		(*test[j].funcall)(n);
	}
	end = ptime(1);
	stime = (double)(end - start) / 60.0;
	if( (stream = fopen("crunout","w")) == NULL )
	{
		printf("unable to open file crunout\n");
		exit(1);
	}
	fprintf(stream,"%s\n",machine);
	fprintf(stream,"TESTTIME = %9.5f\n",stime);
	}
	{
	register int dc,tc,i;
	nl = 1000;
	nt = 30;
	nd = 32;
	lflag = 1;
	loop:
	dc = 1;
	tc = 1;
	ddcount = 0;
	ttcount = 0;
	dstep = 0.0;
	ddelta = (double)nd / (double)nl;
	tstep = 0.0;
	tdelta = (double)nt / (double)nl;
	factor = 2.100;
	nl++;
	start = ptime(1);
	for(i=1; i<=nl; i++)
	{
		dstep += ddelta;
		ddcnt = (double)dc;
		if( dstep >= ddcnt )
		{
			dc++;
		}
		tstep += tdelta;
		ttcnt = (double)tc;
		if( tstep >= ttcnt )
		{
			tc++;
		}
	}
	end = ptime(1);
	ltime = (double)(end - start) / 60.0;
	fprintf(stream,"nl = %d   ",nl);
	fprintf(stream,"ltime = %9.5f  dcount = %d   tcount = %d\n",ltime,dc,tc);
	if( lflag )
	{
		nl = (int)( (double)nl * (stime / (ltime * factor) ) );
		lflag = 0;
		goto loop;
	}
	fclose(stream);
	if( (stream = fopen("loopcount","w")) == NULL )
	{
		printf("unable to open file loopcount\n");
		exit(1);
	}
	fprintf(stream,"%d\n",nl);
	fclose(stream);
	if( (stream = fopen("subele","a")) == NULL )
	{
		printf("unable to open file subele\n");
		exit(1);
	}
	fprintf(stream,"\f\n\n");
	fprintf(stream," S Y S T E M   U N D E R   T E S T   %s\n\n\n",machine);
	fprintf(stream,"\n  C P U   S U B S Y S T E M \n");
	fprintf(stream," Time to Execute CRUN %9.3f\n",stime);
	fprintf(stream," Realative speed of a Single CPU  %9.3f\n", factor / stime );
	fprintf(stream," Number of Main CPU Processors %d\n",nomp);
	fclose(stream);
	}
	if( nomp > 1 )
	{
		if( execl("cpurun","cpurun",NULL) < 0 )
		{
			printf(" unable to execute cpurun \n");
		}
	}
	exit(0);
}

long
ptime(tflag)
	int tflag;
{
	static long tm[4];
	static int first = 1;
	static long tstart = 0;
	static long xtime = 0;
	long times( );

	if( tflag && first )
	{
		tstart = times(tm);
		first = 0;
		return(0);
	}
	xtime = times(tm);
	if( tflag )
	{
		xtime = xtime - tstart;
	}
	else
	{
		xtime = tm[0];
	}
	return(xtime);
}

