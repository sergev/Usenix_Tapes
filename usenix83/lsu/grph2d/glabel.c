#
#include <stdio.h>
#include <math.h>
#include "nuplot.h"
#define YMGN	YSIZE*NPW-H
#define nchars 65	/*number of chars in char set*/
#define MAXLINE 780
#define nlines  6	/*number of lines in a banner character*/
#define pposs  780	/*number of print positions on a line (must be multiple of 4)*/
			/*followed by end of string character*/
#define pospch 8	/*number of char positions per banner char*/
#define chpln  120	/*number of banner characters per line*/

/*********************************************************************
 *                                                                   *
 *         /USR/KLYCE/VGRAPH/VLABEL.C                                *
 *                                                                   *
 *	This program is part of the local graphics package to plot   *
 *  on the Printronix P300.  A large part of the code herein was     *
 *  lifted verbatim from the UNIX BANNER program.  Used with plotlb  *
 *  this program provides title and axis labels with proper scaling. *
 *                                                                   *
 *  BUGS:  My part of the code is rather clumsy - writing and read-  *
 *  files for type conversions, etc. - which slows it down.  But it  *
 *  seems to work!                                                   *
 *                                                                   *
 *                      S. Klyce     13/Nov/80                       *
 *                                                                   *
 *               R E V I S I O N  H I S T O R Y  		     *
 *								     *
 *	>>> Dec  3, 1980 (klyce).  Fixed bug that hung up axlabel    *
 *  when xmax or ymax == 0.				             *
 *								     *
 *      >>> Feb  9, 1981 (klyce).  Converted program to write data   *
 *  into two files (upper half of page, lower half) to take advan-   *
 *  tage of the faster plotting routines worked out in /usr/klyce/   *
 *  newgraphix.  						     *
 *	At the request of the Computer Esthetics Committee, headed   *
 *  by Fred Welden, the bit pattern for char "0" was modernized.     *
 *								     *
 *	>>> Jan  14, 1982 (klyce).  Converted axlabel.c to vlabel.c  *
 *  to provide labels to 3D plots for the viscous flow model.        *
 *								     *
 *	>>> Jan 29, 1982 (klyce).  Converted vlabel.c to vlabel.c    *
 *  to upgrade 2D plotting package to 3 print windows.		     *
 *								     *
 *********************************************************************/

struct bann{
	  char alpha[nlines][pposs];
};
struct bann buffer,*bp &buffer;
char ctbl[nchars][nlines]{
	036,041,046,051,046,035,	/*@*/
	014,022,041,077,041,041,	/*A*/
	076,041,076,041,041,076,	/*B*/
	036,041,040,040,041,036,	/*C*/
	076,041,041,041,041,076,	/*D*/
	077,040,076,040,040,077,	/*E*/
	077,040,076,040,040,040,	/*F*/
	036,041,040,047,041,036,	/*G*/
	041,041,077,041,041,041,	/*H*/
	004,004,004,004,004,004,	/*I*/
	001,001,001,001,041,036,	/*J*/
	041,042,074,044,042,041,	/*K*/
	040,040,040,040,040,077,	/*L*/
	041,063,055,041,041,041,	/*M*/
	041,061,051,045,043,041,	/*N*/
	036,041,041,041,041,036,	/*O*/
	076,041,041,076,040,040,	/*P*/
	036,041,041,045,042,035,	/*Q*/
	076,041,041,076,042,041,	/*R*/
	036,040,036,001,041,036,	/*S*/
	037,004,004,004,004,004,	/*T*/
	041,041,041,041,041,036,	/*U*/
	041,041,041,041,022,014,	/*V*/
	041,041,041,055,063,041,	/*W*/
	041,022,014,014,022,041,	/*X*/
	021,012,004,004,004,004,	/*Y*/
	077,002,004,010,020,077,	/*Z*/
	016,010,010,010,010,016,	/*[*/
	040,020,010,004,002,001,	/*\*/
	034,004,004,004,004,034,	/*]*/
	004,012,000,000,000,000,	/*^*/
	000,000,000,000,000,077,	/*_*/
	000,000,000,000,000,000,	/* */
	010,010,010,010,000,010,	/*!*/
	022,022,000,000,000,000,	/*"*/
	022,077,022,022,077,022,	/*#*/
	036,054,036,015,055,036,	/*$*/
	001,062,064,013,023,040,	/*%*/
	014,022,014,024,042,035,	/*&*/
	010,010,000,000,000,000,	/*'*/
	004,010,010,010,010,004,	/*(*/
	010,004,004,004,004,010,	/*)*/
	000,022,014,014,022,000,	/***/
	000,010,010,076,010,010,        /*+*/
	000,000,060,060,020,040,	/*,*/
	000,000,000,076,000,000,	/*-*/
	000,000,000,000,060,060,	/*.*/
	001,002,004,010,020,040,	/*<backslash>*/
	014,022,022,022,022,014,        /*0*/
	004,014,004,004,004,037,	/*1*/
	016,021,001,016,020,037,	/*2*/
	016,021,001,016,001,036,	/*3*/
	022,022,022,037,002,002,	/*4*/
	037,020,020,036,001,036,	/*5*/
	017,020,036,021,021,016,	/*6*/
	037,001,002,004,010,020,	/*7*/
	016,021,016,021,021,016,	/*8*/
	016,021,021,017,001,016,	/*9*/
	000,030,030,000,030,030,	/*:*/
	000,030,030,000,010,020,	/*;*/
	000,000,000,000,000,000
};
FILE *fp, *fopen();
FILE *fq, *fopen();
FILE *fr, *fopen();
FILE *fs, *fopen();
FILE *ft, *fopen();
char blank ' ';
char plot 'X';
int msk 040;	/*mask at sixth bit*/
char title[120];
char ylabel[120];
char xlabel[120];
float ymin,ymax,xmin,xmax;
float yexp,xexp;
char yscale[132],xscale[132];
int flag 0;
main()
{
	float setexp();
	char **argp;int argc;
	char yx[20],xx[20];
	char dummy[MAXLINE];
	int i;
	argc=4;
	fr = fopen("/tmp/glabel","r");
	fgets(title,120,fr);
	fgets(ylabel,120,fr);
	fgets(xlabel,120,fr);
	fclose(fr);
	squeeze(title,'\n');
	squeeze(ylabel,'\n');
	squeeze(xlabel,'\n');
	fs = fopen("/tmp/gscale","r");
	fscanf(fs,"%f%f%f%f",&ymin,&ymax,&xmin,&xmax);
	fclose(fs);
	setexp(&yexp,&ymin,&ymax);
	setexp(&xexp,&xmin,&xmax);
	fr = fopen("/tmp/g2scale","w");
	for (i=0;i<11;i++)
	{
		fprintf(fr,"%5.2f\n",ymin+i*(fabs(ymax-ymin)/10));
		fprintf(fr,"%5.2f\n",xmin+i*(fabs(xmax-xmin)/10));
	}
	fclose(fr);
	fr = fopen("/tmp/g2scale","r");
	fq = fopen("/tmp/plt_data_a","a");
	ft = fopen("/tmp/plt_data_b","a");
	fs = fopen("/tmp/plt_data_c","a");
	for (i=0;i<11;i++)
	{
		fgets(yscale,132,fr);
		squeeze(yscale,'\n');
		banner(yscale,bp);
		legend((int)(W),(int)(CHARH/2+i*H/10+YMGN),XYP,HORZ);
		fgets(xscale,132,fr);
		squeeze(xscale,'\n');
		banner(xscale,bp);
		legend((int)(-5*CHARW/2+i*W/10),(int)(YMGN-0.5*CHARH),XYP,HORZ);
	}
	fclose(fr);
	fr = fopen("/tmp/exp","w");
	fprintf(fr,"x%6.0e\nx%6.0e\n",yexp,xexp);
	fclose(fr);
	fr = fopen("/tmp/exp","r");
	fgets(yx,20,fr);
	fgets(xx,20,fr);
	fclose(fr);
	squeeze(yx,'\n');
	squeeze(xx,'\n');
	banner(yx,bp);
	legend((int)(W+5.5*CHARW+1.5*CHARH),(int)(H/2-4*strlen(yx)+YMGN),XYP,VERT);
	banner(xx,bp);
	legend((int)(W/2-4*strlen(xx)),(int)(YMGN-5*CHARH),XYP,HORZ);
	banner(title,bp);
	legend((int)(W/2-4*strlen(title)),(int)(H-3*CHARH+YMGN),XYP,HORZ);
	banner(ylabel,bp);
	legend((int)(W+5.5*CHARW),(int)(H/2-4*strlen(ylabel)+YMGN),XYP,VERT);
	banner(xlabel,bp);
	legend((int)(W/2-4*strlen(xlabel)),(int)(YMGN-3*CHARH),XYP,HORZ);
	fclose(ft);
	fclose(fq);
	fclose(fs);
}

banner(s,bufp)
char *s;struct bann *bufp;
{
	char c,*p,*q,*r;
	p=s;
	r=bufp;
	banset(blank,bufp);

	while((c= *s++)!=0){
	  if((s-p)>chpln)return(s-p);
	  if(c>='`')c =- ' ';	/*map lower to upper case*/
	  if(c<' ')c='#';
	  if(c>'?')c=- 0100;
	  q=ctbl[c];
	  banfil(q,r);
	  r=+pospch;
	}
}

banfil(c,p)
char *c;
struct bann *p;
{
	int i,j;
	for(i=0;i<nlines;i++){
	  for(j=0;j<pospch;j++){
	    if(((c[i]<<j)&msk)!=0)p->alpha[i][j] = plot;
	  }
	}
	return(0);
}

banset(c,p)
char c;
struct bann *p;
{
	int i,j;
	for(i=0;i<nlines;i++)
	  for(j=0;j<pposs-1;j++)
	    p->alpha[i][j] = c;
}

banprt(ptr,fildes)
struct bann *ptr;
int fildes;
{
	int i,j;
	for(i=0;i<nlines;i++){
	  ptr->alpha[i][pposs-1]='\0';
	  for(j=pposs-2;j>=0;j--){
	    if(ptr->alpha[i][j]!=blank)break;
	    ptr->alpha[i][j]='\0';
	  }
	fprintf(fildes,"%s\n",ptr->alpha[i]);
	}
}
legend(x,y,plane,direction)
int x,y,plane,direction;
{
	char dummy[MAXLINE];
	int i,j,k;
	float xp,yp,w;
	float ptrans();
/*
					printf("x= %d  y= %d  z= %d  plane= %d  direction= %d\n\n",x,y,z,plane,direction);
*/
	fp = fopen("/tmp/legend","w");
	banprt(bp,fp);
	fprintf(fp,"*\n");
	fclose(fp);
	fp = fopen("/tmp/legend","r");
	for(j=0;;)
	{
		fgets(dummy,MAXLINE,fp);
/*
if (flag) printf ("%s",dummy);
*/
		i=0;
		while (dummy[i] != '\n' && dummy[i] != '\0')
		{
			if (dummy[i] == 'X')
			{
				if (plane==XYP)
				{
					if (direction==HORZ)
					{
						xp=x+i;
						yp=y-j;
					}
					else
					{
						xp=x+j;
						yp=y+i;
					}
				}
				else
				{
					printf("glabel: unknown plane.\n");
					exit(-1);
				}
				openwindow((int)yp,(int)xp);
			}
			i++;
		}
		j++;
		if (strcmp(dummy,"*\n\0")==0) break;
	}
	fclose(fp);
}
squeeze(s,c)
char s[];
int c;
{
	int i,j;
	for (i=j=0;s[i]!='\0';i++)
		if (s[i]!=c)
			s[j++]=s[i];
	s[j]='\0';
}
strlen(s)
char *s;
{
	char *p = s;
	while(*p != '\0') p++;
	return(p-s);
}
bombout()
{
	printf("\nError in glabel: /tmp/gscale improperly scaled\n");
	exit(-1);
}

float setexp(pexp,pmin,pmax)
float *pexp,*pmin,*pmax;
{
	auto float exp,min,max;
	exp=1;
	max= *pmax;
	min= *pmin;
/*
printf("exp=%f min=%f max=%f\n",exp,min,max);
*/
	if (max!=0)
	{
		while(fabs(max)>10)
		{
			max =/ 10;
			exp =* 10;
		}
		while(fabs(max)<1)
		{
			max =* 10;
			exp =/ 10;
		}
		min =/ exp;
	}
	else if (min!=0)
	{
		while(fabs(min)>10)
		{
			min =/ 10;
			exp =* 10;
		}
		while(fabs(min<1))
		{
			min =* 10;
			exp =/ 10;
		}
		max =/ exp;
	}
	else bombout();
	*pexp=exp;
	*pmin=min;
	*pmax=max;
}
openwindow(y,x)
int y,x;
{
	int fd;
/*
printf("opwin: x=%d y=%d\n",x,y);
*/
	y = YSIZE*NPW - y;
	y += TMGN;
	x += LMGN;
	if (y >= 0 && y <= YSIZE*NPW && x >= 0 && x <= XSIZE*6)
	{
		if (y < YSIZE) fd = fq;
		else if (y < 2*YSIZE) fd = ft;
		else fd = fs;
		fprintf(fd,"%4d %4d\n",y,x);
	}
}


