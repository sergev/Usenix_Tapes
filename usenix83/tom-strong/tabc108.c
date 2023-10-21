
#define INCH 240
/*
Concept 108 @ 10 char/inch
nroff driving tables
width and code tables

28 sep 81	Strong	written
31 jul 82	Strong	use new Diablo/nroff character set
			don't clear terminal before outputting
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
/*Vert*/	INCH/6,
/*Newline*/	INCH/6,
/*Char*/	INCH/10,
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/10,
/*twinit*/	"\033j ",
/*twrest*/	"\033t\033j \033NH",
/*twnl*/	"\015\n",
/*hlr*/		"",
/*hlf*/		"",
/*flr*/		"\033 L",
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

