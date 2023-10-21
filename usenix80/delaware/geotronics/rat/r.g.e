38c
newdo	: NEWDO		={ docode($1); }
.
19c
	| ASSERT	={ asscode($1); }
.
15c
	| repeat stat	={ untils(0,$1); }
	| repeat stat UNTIL	={ untils(1,$1); }
.
5c
%term	REPEAT UNTIL STRING PROGRAM ASSERT REFERENCE
.
3c
%term	NEWDO
.
0a
/*
 * r.g - yacc definitions for Ratfor language
 *
 *	modified 28-Apr-1980 by D A Gwyn:
 *	1) repeat without until supported;
 *	2) do with label no longer supported;
 *	3) assert statement supported.
 */
.
w
q
