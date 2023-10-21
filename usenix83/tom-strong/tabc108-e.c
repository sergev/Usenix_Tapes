
#define INCH 240
/*
Concept 108 @ 10 char/inch for equations and (unboxed) tables
	it double spaces and does halflines by full line movements
	boxed tables do not work right, nor does 2 column output
nroff driving tables
width and code tables

28 sep 81	Strong	Works for simple cases.
 4 aug 82	Strong	use with new diablo/nroff character set
			always double spaces
17 aug 82	Strong	does reverse movements correctly
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
	} t {
/*bset*/	0,
/*breset*/	0177420,
/*Hor*/		INCH/10,
/*Vert*/	INCH/12,
/*Newline*/	INCH/6,
/*Char*/	INCH/10,
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/10,
/*twinit*/	"\033j \014",
/*twrest*/	"\033t\033j \033NH",
/*twnl*/	"\015\n\n",
/*hlr*/		"\033;",		/*reverse linefeed*/
/*hlf*/		"\033<",		/*linefeed*/
/*flr*/		"\033;\033;",		/*2 reverse linefeeds*/
/*bdon*/	"\033D",
/*bdoff*/	"\033d",
/*ploton*/	"",
/*plotoff*/	"",
/*up*/		"",
/*down*/	"",
/*right*/	"",
/*left*/	"",
/*codetab*/
#include "c108"

