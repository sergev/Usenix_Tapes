
#define INCH 240
/*
Concept AVT @ 10 char/inch
nroff driving tables
width and code tables

28 sep 81	c108 driver written
31 jul 82	use new Diablo/nroff character set
			don't clear terminal before outputting
 1 Feb 83	modify for AVT
Tom Strong	Penny Penny & Strong	(415) 524-2000
*/

struct {
	int bset;
	int breset;
	int Hor;
	int Vert;
	int Newline;
	int Char;
	int Em;
	int Halfline;
	int Adj;
	char *twinit;
	char *twrest;
	char *twnl;
	char *hlr;
	char *hlf;
	char *flr;
	char *bdon;
	char *bdoff;
	char *ploton;
	char *plotoff;
	char *up;
	char *down;
	char *right;
	char *left;
	char *codetab[256-32];
	int zzz;
	} t= {
/*bset*/	0,
/*breset*/	0177420,
/*Hor*/		INCH/10,
/*Vert*/	INCH/6,
/*Newline*/	INCH/6,
/*Char*/	INCH/10,
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/10,
/*twinit*/	"\033(A\033)1\033[=121l",
/*twrest*/	"\033R\017\033[=0!{\033[=121h",
/*twnl*/	"\015\n",
/*hlr*/		"",
/*hlf*/		"",
/*flr*/		"\033[A",
/*bdon*/	"\033[1m",
/*bdoff*/	"\033[1!{",
/*ploton*/	"",
/*plotoff*/	"",
/*up*/		"",
/*down*/	"",
/*right*/	"",
/*left*/	"",
/*codetab*/
#include "avt"

