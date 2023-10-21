
#define INCH 240
/*
Concept AVT @ 12 char/inch for equations and (unboxed) tables
	it double spaces and does halflines by full line movements
	boxed tables do not work right, nor does 2 column output
nroff driving tables
width and code tables

28 sep 81	Made 10 char/inch from Tc108e-12.c
 4 aug 82	use with new diablo/nroff character set
 1 feb 83	make work with AVT
17 feb 83	make 12 pitch from 10 pitch
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
/*Hor*/		INCH/12,
/*Vert*/	INCH/12,
/*Newline*/	INCH/6,
/*Char*/	INCH/12,
/*Em*/		INCH/12,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/12,
/*twinit*/	"\033(A\033)1\033[H\033[2J\033[=121l",
/*twrest*/	"\033R\017\033[=0!{\033[=121h",
/*twnl*/	"\015\n\n",
/*hlr*/		"\033[A",		/*reverse linefeed*/
/*hlf*/		"\033[B",		/*linefeed*/
/*flr*/		"\033[A\033[A",		/*2 reverse linefeeds*/
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

