%token NHTYPES CHARTAB SPECIAL
%token WORD STRING NUMBER CCONST DIRECT

%{
#include <stdio.h>

#define HDBITS 5	/* number of bits in a new header line type */
#define EXTBITS 5	/* number of bits for extended character type */
#define MAXBITS 10	/* max number of bits in an encoded character */

#define equal(s1, s2)	(strcmp(s1, s2) == 0)

struct values {
	int nvalues ;
	int value[100] ;
}
value[MAXBITS+1] ;

struct etab {
	short cbits ;
	short clen ;
}
etab[256] ;

struct dtab {
	char cval ;
	char clen ;
}
dtab[1<<MAXBITS] ;

struct pair {
	char *name ;
	char *pval ;
}
htypes[1<<HDBITS] ;
int nhead, nprthead ;

char *special[128] ;
int nspecials ;

char extchar[1<<EXTBITS] ;
int nechars ;
%}

%%

input:	input NHTYPES nhtypes = {
	printnhtypes() ;
	}
|	input CHARTAB chartab = {
	printctabs() ;
	}
|	input DIRECT ={
	fputs($2, stdout) ;
	free($2) ;
	}
|	/* empty */
;


nhtypes: nhtypes WORD optstring ={
	if (nhead >= 1<<HDBITS)
		yyerror("Too many header types") ;
	if ($3) {
		if (nprthead != nhead)
			yyerror("Bad header type order") ;
		nprthead++ ;
	}
	htypes[nhead].name = $2 ;
	htypes[nhead].pval = $3 ;
	nhead++ ;
	}
|	/* empty */
;

optstring: STRING
|	/* empty */ = {
	$$ = NULL ;
	}
;

chartab: chartab val number ={
	value[$3].value[value[$3].nvalues++] = $2 ;
	}
|	chartab WORD number ={
	special[nspecials] = $2 ;
	value[$3].value[value[$3].nvalues++] = nspecials + 128 ;
	nspecials++ ;
	}
|	/* empty */
;

number:	NUMBER ={
	$$ = atoi($1) ;
	free($1) ;
	}

val:	number
|	CCONST
;

%%

printnhtypes() {
	int i ;

	for (i = 0 ; i < nhead ; i++) {
		printf("#define %s %d\n", htypes[i].name, i) ;
	}
	printf("\nchar *hprefix[%d] = {\n", nprthead + 1) ;
	for (i = 0 ; i < nprthead ; i++) {
		printf("\t%s,\n", htypes[i].pval) ;
	}
	printf("\t0\n} ;\n") ;
}


printctabs() {
	int len, i, v ;
	int bitpat, mask ;
	int rc ;

	bitpat = 0 ;
	rc = 0 ;
	for (len = 1 ; len <= MAXBITS ; len++) {
		for (i = 0 ; i < value[len].nvalues ; i++) {
			if (rc < 0)
				yyerror("ran out of bit patterns") ;
			v = value[len].value[i] ;
			etab[v].cbits = bitpat ;
			etab[v].clen = len ;
			rc = nextpat(&bitpat, len) ;
		}
	}
	for (i = 0 ; i < 256 ; i++) {
		if (etab[i].clen)
			setcode(etab[i].cbits, etab[i].clen, i) ;
	}
	for (i = 0 ; i < 128 ; i++) {
		if (etab[i].clen == 0) {
			if (nechars > (1<<EXTBITS))
				yyerror("too many extended characters") ;
			extchar[nechars] = i ;
			if ((v = lookspecial("EXTCHAR")) < 0)
				yyerror("EXTCHAR not defined") ;
			etab[i].clen = etab[v+128].clen + EXTBITS ;
			etab[i].cbits = etab[v+128].cbits + (nechars << etab[v+128].clen) ;
			nechars++ ;
		}
	}

	printf("#ifdef UNBAT\n\n") ;
	for (i = 0 ; i < nspecials ; i++) {
		printf("#define %s %d\n", special[i], i) ;
	}
	printf("\nstruct decode dtab[%d] = {\n", 1<<MAXBITS) ;
	for (i = 0 ; i < 1<<MAXBITS ; i++) {
		printf("\t{0%03o, %d},", dtab[i].cval, dtab[i].clen) ;
		if (i & 01)  putchar('\n') ;
	}
	printf("} ;\n\n") ;
	printf("char extchar[%d] = {\n", 1<<EXTBITS) ;
	for (i = 0 ; i < nechars ; i++) {
		printf("\t0%03o,\n", extchar[i]) ;
	}
	printf("} ;\n\n#else\n\n") ;
	for (i = 0 ; i < nspecials ; i++) {
		if (! equal(special[i], "EXTCHAR")) {
			printf("#define PK%s 0%o\n", special[i], etab[i+128].cbits) ;
			printf("#define L%s %d\n", special[i], etab[i+128].clen) ;
		}
	}
	printf("\nstruct encode etab[128] = {\n") ;
	for (i = 0 ; i < 128 ; i++) {
		printf("\t{0%o, %d},\n", etab[i].cbits, etab[i].clen) ;
	}
	printf("} ;\n\n#endif\n") ;
}



nextpat(b, len)
	int *b ;
	{
	int bit = 1 << (len - 1) ;

	if (len <= 0)
		return -1 ;
	*b ^= bit ;
	if ((*b & bit) == 0)
		return nextpat(b, len - 1) ;
	return 0 ;
}


setcode(b, len, val) {
	int i ;

	b &= (1 << len) - 1 ;
	for (i = b ; i < 1<<MAXBITS ; i += 1 << len) {
		dtab[i].cval = val ;
		dtab[i].clen = len ;
	}
}


lookspecial(s)
	char *s ;
	{
	int i ;

	for (i = 0 ; i < nspecial ; i++)
		if (equal(special[i], s))
			return i ;
	return -1 ;
}


yyerror(msg)
	char *msg ;
	{
	extern int yylineno ;
	extern int yytext ;

	fprintf(stderr, "%s, line %d\n", msg, yylineno) ;
	fprintf(stderr, "	input token = \"%s\"\n", yytext) ;
}
