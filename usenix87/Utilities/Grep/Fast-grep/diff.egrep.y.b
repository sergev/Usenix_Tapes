19a20
> #include <ctype.h>
27a29
> char cmap[256];
51a54
> int 	iflag;
425a429
> 	register int i;
426a431,433
> 	for ( i = 0; i < 256; i++ )
> 		cmap[i] = (char) i;
> 
454a462,467
> 		case 'i':
> 			iflag++;
> 			for ( i = 'A'; i <= 'Z'; i++ )
> 				cmap[i] = (char) tolower ( i );
> 			continue;
> 
483a497,502
> 	if ( iflag ) {
> 		register char *s;
> 		for ( s = input; *s != '\0'; s++ )
> 			if ( isupper ( (int)(*s) ) )
> 				*s = (char) tolower ( (int)(*s) );
> 	}
508a528
> 	register char *cmapr = cmap;
544c564
< 		cstat = gotofn[cstat][*p&0377]; /* all input chars made positive */
---
> 		cstat = gotofn[cstat][cmapr[*(unsigned char *)p]]; 
