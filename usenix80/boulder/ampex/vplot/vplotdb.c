#
/*  A program to list the intermediate plot file
 *  in a readable form.
 */
#include	<stdio.h>
#define SCALE	16384.0
#define PLTCMD	0200
#define CMDMSK	0177
#define ENDRAS	-1
#define ESC	033
#define ESCMAP	'\\'
char inbuf[BUFSIZ];
FILE *pltin;
main(argc,argv)
int argc; char **argv;
   {
	register char *cptr;
	FILE *temp, *freopen();

	pltin= stdin;
	setbuf(pltin,inbuf);
	do
	   {
		argc--; argv++;
		cptr= *argv;
		if(argc)
			if((temp=freopen(cptr,"r",pltin))==NULL)
			   {
				printf("plotdb: cannot open %s\n",cptr);
				continue;
			   }
			  else pltin= temp;
		doplot();
	   } while( argc );
   }

doplot()
   {
	int i,size,orient,fat,dotsize;
	char tfont;
	float x,y,xmin,xmax,ymin,ymax,angle,tsize;
	int nlines,nesc,offset,count,header,omin,omax,cmin,cmax;
	register int c;
	while((c=getc(pltin)) != EOF)
	   {
		if ((c&PLTCMD) == 0 || ((c&CMDMSK) < 'a' || (c&CMDMSK) > 'z'))
		   {
			printf("straight text:\n");
			do
			  {	if (c&PLTCMD)
					putchar(ESCMAP);
				putchar(c&CMDMSK);
			  }
			while(((c=getc(pltin))&PLTCMD)==0);
			if( c == EOF ) break;
			ungetc(c,pltin);
			continue;
		   }
		switch(c =& CMDMSK)
		  {
		  case 'e':
		  case 'b':
			for(i=0;i<10;i++) printf("**");
			printf("\n%c:\n",c);
			break;
		  case 'm':
		  case 'd':
			x=getw(pltin)/SCALE;
			y=getw(pltin)/SCALE;
			printf("%c: %-9.6f %-9.6f\n",c,x,y);
			break;
		  case 's':
			tsize= (getc(pltin)&0377)*64./SCALE;
			tfont= getc(pltin)&0377;
			if (tfont == '\0') tfont = 'A';
			angle = getw(pltin)*360./32768.;
			printf("s: font=%c, width=%f, angle=%f degrees\n",
				tfont, tsize, angle);
			break;
		  case 't':
			printf("t: ");
			while((c=getc(pltin)))
			  {	if(c==ESC)
				  {	switch(c=getc(pltin))
					  {
					  case 'u':
					  case 'U':
					  case 'd':
					  case 'D':
					  case 'b':
					  case 's':
					  case 'S':
					  case 'f':
					  case 'F':
					  case 'g':
						putchar(ESCMAP);
						putchar(c);
						break;
					  default:
						putchar(ESCMAP);
						putchar(c);
						putchar('?');
						break;
					  }
				  }
				else	printf("%c",c);
			  }
			printf("\n");
			break;
		  case 'f':
			fat=getw(pltin);
			printf("f: %d\n",fat);
			break;
		  case 'w':
			xmin=getw(pltin)/SCALE;
			xmax=getw(pltin)/SCALE;
			ymin=getw(pltin)/SCALE;
			ymax=getw(pltin)/SCALE;
			printf("w: %-9.6f %-9.6f %-9.6f %-9.6f\n",xmin,xmax,ymin,ymax);
			break;
		  case 'r':
			dotsize=getc(pltin);
			nlines=0;
			nesc = 0;
			cmax= 0;
			cmin= 10000;
			omax= 0;
			omin= 10000;
			while((header=getw(pltin)) != ENDRAS)
			  {
				count= header&0377;
				offset= (header>>8)&0377;
				if (count == 0377)	/* escape */
				  {	switch (offset)
					  {
					  case 't':
					  case 'T':
						for (count=0; getc(pltin)>0; )
							count++;
						if (count&1)
							getc(pltin);
						break;
					  }
					nesc++;
				  }
				else		/* raster line */
				  {	if(cmin>count) cmin=count;
					if(cmax<count) cmax=count;
					if(omin>offset) omin=offset;
					if(omax<offset) omax=offset;
					while(count--) getw(pltin);
					nlines++;
				  }
			   }
			printf("r: %d pels/dot, %d lines, %d escapes,\n",
				dotsize, nlines, nesc);
			printf("offset(min,max)= (%d,%d), ",omin,omax);
			printf("count(min,max)= (%d,%d)\n",cmin,cmax);
			break;
		  case 'x':
			printf("display X-hairs\n");
			break;
		  case 'p':
			printf("purge plot buffers\n");
			break;
		  case 'n':
			printf("n:\n");
			break;
		  default:
			printf("unknown command %c %o\n",c,c);
	   }
   }
   }
