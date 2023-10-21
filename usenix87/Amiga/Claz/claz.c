/*              CLAZ.c  v2.0                     */
/*              by Steve Ludtke                  */
/*              Created : 05/31/87               */

#include<stdio.h>
#include<math.h>
#include<exec/types.h>

/* define IFF structures */
struct CMAP {
UBYTE r,g,b;
};

struct BMHD {
UWORD w,h;
WORD x,y;
UBYTE npl;
UBYTE masking,compression,pad1;
UWORD tcolor;
UBYTE xas,yas;
WORD pWid,pHig;
};

char *malloc();

main (argc,argv)
int argc;
char *argv[];
{
struct BMHD mhd;
long *len1,*len2,cnt,rsz,m,h,i,j,k,d,lr,lb,lg,q1,q2,x1,x2,y1,y2,fl;
int fl1,b;
BYTE t;
UBYTE cmap[100];
char *raster,*rp,*fspin,*outfsp,hex[20],msc1[12],q,x,y,c,msc2[8],*misc;
FILE *fin,*fout;

hex[0]='0'; hex[1]='1'; hex[2]='2'; hex[3]='3';
hex[4]='4'; hex[5]='5'; hex[6]='6'; hex[7]='7';
hex[8]='8'; hex[9]='9'; hex[10]='A'; hex[11]='B';
hex[12]='C'; hex[13]='D'; hex[14]='E'; hex[15]='E';
fl=0; x1=10; y1=10; x2=550; y2=480;

/* Get enough memory, figure out filespecs */
raster=malloc(200000);
misc=malloc(2000);
if (raster==NULL) { printf("Not enough memory.\n"); exit(0); }
if (argc<3) outfsp="out.ps";
if (argc>2) outfsp=argv[2];
if (argc>3) {
	sscanf(argv[3],"%ld",&x1);
	sscanf(argv[4],"%ld",&y1);
	sscanf(argv[5],"%ld",&x2);
	sscanf(argv[6],"%ld",&y2);
	}
if (argc==8 || argc==4) fl|=1;

printf("\n\n                CLAZ - IFF to POSTSCRIPT converter V2.0\n");
printf("                           by Steve Ludtke\n");
printf("                       N\251 - No Copyright 1987\n\n");
printf("File in : ");
if (argc>1) {
	fspin=argv[1];
	printf("%s\n",fspin);
	}
if (argc==1) {
	fspin=malloc(80);
	scanf("%s",fspin);
	}
fin=fopen(fspin,"r");
if (fin==NULL) { printf("Sorry, can't open input file.\n"); exit(0); }
if ((fread(msc1,12,1,fin))==0) { printf("Sorry, input file problem #1. \n"); exit (0); }
len1=(long *)&msc1[4];
printf("main : %d,%s\n",*len1,msc1);
cnt=14;

/* *len1=# bytes in file according to IFF */
/* msc2=the name of the current chunk */
/* *len2=# bytes in current chunk */
/* by the way, UNID is symply any chunk the program doesn't deal with */

/* Reads in the IFF file : */

while(cnt < *len1) {
fl1=0;
if ((fread(msc2,8,1,fin))==0) { printf("Sorry, bad input file. \n"); exit(0); }
len2=(long *)&msc2[4];

if (strncmp(msc2,"CMAP",4)==0) {
	if ((fread(cmap,*len2,1,fin))==0) { printf("Sorry, bad CMAP. \n"); exit(0); }
	printf("CMAP\n");
	fl1=1;
	}

if ((strncmp(msc2,"BMHD",4))==0) {
	if ((fread(&mhd,*len2,1,fin))==0) { printf("Sorry, bad BMHD. \n"); exit(0); }
	printf("BMHD\n");
	fl1=1;
	}

if (strncmp(msc2,"BODY",4)==0) {
	printf("BODY\n");
	fl1=1;
	m=0; rp=raster;
	if (mhd.compression==1) {
		while (m<=*len2) {
			t=getc(fin); m++;
			if (t>=0) { 
				t++;
				fread(rp,(long) t,1,fin);
				rp+=t;
				m+=t;
				}
			if (t<0) {
				t=(-t)+1;
				c=getc(fin);
				m++;
				for (i=0; i<t; i++) {
						*rp=c; rp++; }
				}
			}
		}
	if (mhd.compression==0) {
		if ((fread(rp,*len2,1,fin))==0) { printf("Sorry, bad BODY. \n"); exit(0); }
		printf("No Comression\n");
		}
	if (mhd.compression>1) { printf("Sorry, unknown compression type.\n");
		exit(0); }
	}

if (fl1==0) {
	if (*len2>2000) { printf("Sorry, UNID too long.\n"); exit(0); }
	if ((fread(misc,*len2,1,fin))==0) { printf("Sorry, UNID bad. \n"); exit(0); }
	printf("UNID\n");
	}
cnt+=*len2;
cnt+=8;
}

fclose(fin);
free(fspin);

if ((fout=fopen(outfsp,"a"))==NULL) { printf("Sorry, cannot open output file.\n"); exit(0); }

/* Standard POSTSCRIPT program, the only part following the data is */
/* the showpage command. */

fprintf(fout,"[0 0 0 0 0 0] defaultmatrix setmatrix");
fprintf(fout,"/picstr %d string def\r",mhd.w);
fprintf(fout,"%ld %ld translate\r",x1,y1);
fprintf(fout,"%ld %ld scale\r",x2,y2);
fprintf(fout,"%d %d 4 [%d 0 0 -%d 0 %d]\r",mhd.w,mhd.h,mhd.w,mhd.h,mhd.h);
fprintf(fout,"{currentfile picstr readhexstring pop} image\r");

/* calculate and output file */

lb=lr=lg=0;
rsz=mhd.w*mhd.h/8;
printf("rast size : %d     #planes : %d\n",rsz,mhd.npl);
for (h=0; h<96; h++) cmap[h]=cmap[h]/16; 
for (h=0; h<mhd.h; h++) {
for (i=0; i<mhd.w/8; i++) {
	for(k=7;k>=0;k--) {
		b=0;
		for(j=0; j<mhd.npl; j++) {
			c=(raster[h*(mhd.w/8)*mhd.npl+i+((mhd.npl-j-1)*mhd.w/8)]);
			c=c>>k;
			c&=1;
			b|=c;
			b<<=1;
			}
		b>>=1;

/* this is for HAM pictures */
		if (mhd.npl==6) {
			c=(b&48)>>4;
			b&=15;
			if (c==0) {
				b*=3;
				lr=cmap[b]; lb=cmap[b+1]; lg=cmap[b+2];
				}
			if (c==1) lb=b;
			if (c==2) lr=b;
			if (c==3) lg=b;
			}
		if (mhd.npl!=6) {
			b&=31;
			b*=3;
			lr=cmap[b]; lb=cmap[b+1]; lg=cmap[b+2];
			}
		m=lr+lb+lg; m=m/3;
		if (m<0) m=0;
		if (m>14) m=14;
		putc(hex[m],fout);
		}
	}
}
fprintf(fout,"\r");
/* Hold the 'showpage' if there is an appended character */
if (!(fl&1)) fprintf(fout,"showpage\r");
fclose(fout);
printf("Done !!!\n");
free(misc);
free(raster);
}
