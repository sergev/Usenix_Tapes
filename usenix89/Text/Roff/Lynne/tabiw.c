/* tabiw.c

	non-proportional nroff table for Imagewriter II (I) printer

	copyright (c) 1987 Stuart Lynne

*/

/*
	This file defines an nroff table using the imagewriter's
	built in fonts.

	To construct tabiwp you must have the table generation software.
	This has been tested with Bruce Townsend's version.

*/



#ifndef	INCH
#define	INCH	240
#endif

struct t {
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
	char *iton;
	char *itoff;
	char *ploton;
	char *plotoff;
	char *up;
	char *down;
	char *right;
	char *left;
	char *codetab[256-32];
	char *zzz;
	} t = {
/*bset    */		00,
/*breset  */		054,
/*Hor     */		4,
/*Vert    */		5,
/*Newline */		40,
/*Char    */		24,
/*Em      */		24,
/*Halfline*/		20,
/*Adj     */		24,
/*twinit  */		"", /* we could put stuff here, but leave it for 
				lp filter instead  sl */
/*twrest  */		"",
/*twnl    */		"\033\"\r\n",	/* turn boldface off, newline */
/*hlr     */		"",
/*hlf     */		"",
/*flr     */		"",
/*bdon    */		"\033!",	/* turn boldface on */
/*bdoff   */		"\033\"",	/* turn boldface off */
/*iton    */		"\033X",	/* turn underline on */
/*itoff   */		"\033Y",	/* turn underline off */
/*ploton  */		"",
/*plotoff */		"",
/*up      */		"",
/*down    */		"",
/*right   */		"",
/*left    */		"",
/* space */         	"\001 ",
/* ! */             	"\001!",
/* " */             	"\001\"",
/* # */             	"\001#",
/* $ */             	"\001$",
/* % */             	"\001%",
/* & */             	"\001&",
/* ' close */       	"\001'",
/* ( */             	"\001(",
/* ) */             	"\001)",
/* * */             	"\001*",
/* + */             	"\001+",
/* , */             	"\001,",
/* - hyphen */      	"\001-",
/* . */             	"\001.",
/* / */             	"\001/",
/* 0 */             	"\2010",
/* 1 */             	"\2011",
/* 2 */             	"\2012",
/* 3 */             	"\2013",
/* 4 */             	"\2014",
/* 5 */             	"\2015",
/* 6 */             	"\2016",
/* 7 */             	"\2017",
/* 8 */             	"\2018",
/* 9 */             	"\2019",
/* : */             	"\001:",
/* ; */             	"\001;",
/* < */             	"\001<",
/* = */             	"\001=",
/* > */             	"\001>",
/* ? */             	"\001?",
/* @ */             	"\001@",
/* A */             	"\201A",
/* B */             	"\201B",
/* C */             	"\201C",
/* D */             	"\201D",
/* E */             	"\201E",
/* F */             	"\201F",
/* G */             	"\201G",
/* H */             	"\201H",
/* I */             	"\201I",
/* J */             	"\201J",
/* K */             	"\201K",
/* L */             	"\201L",
/* M */             	"\201M",
/* N */             	"\201N",
/* O */             	"\201O",
/* P */             	"\201P",
/* Q */             	"\201Q",
/* R */             	"\201R",
/* S */             	"\201S",
/* T */             	"\201T",
/* U */             	"\201U",
/* V */             	"\201V",
/* W */             	"\201W",
/* X */             	"\201X",
/* Y */             	"\201Y",
/* Z */             	"\201Z",
/* [ */             	"\001[",
/* \ */             	"\001\\",
/* ] */             	"\001]",
/* ^ */             	"\001^",
/* _ dash */        	"\001_",
/* ` open */        	"\001`",
/* a */             	"\201a",
/* b */             	"\201b",
/* c */             	"\201c",
/* d */             	"\201d",
/* e */             	"\201e",
/* f */             	"\201f",
/* g */             	"\201g",
/* h */             	"\201h",
/* i */             	"\201i",
/* j */             	"\201j",
/* k */             	"\201k",
/* l */             	"\201l",
/* m */             	"\201m",
/* n */             	"\201n",
/* o */             	"\201o",
/* p */             	"\201p",
/* q */             	"\201q",
/* r */             	"\201r",
/* s */             	"\201s",
/* t */             	"\201t",
/* u */             	"\201u",
/* v */             	"\201v",
/* w */             	"\201w",
/* x */             	"\201x",
/* y */             	"\201y",
/* z */             	"\201z",
/* { */             	"\001{",
/* | */             	"\001|",
/* } */             	"\001}",
/* ~ */             	"\001~",
/* narrow sp */     	"\000\0",
/* hyphen */        	"\001-",
/* bullet */        	"\000\0",
/* square */        	"\000\0",
/* 3/4 em */        	"\001-",
/* rule */          	"\001_",
/* 1/4 */           	"\0031/4",
/* 1/2 */           	"\0031/2",
/* 3/4 */           	"\0033/4",
/* minus */         	"\001-",
/* fi */            	"\202fi",
/* fl */            	"\202fl",
/* ff */            	"\202ff",
/* ffi */           	"\203ffi",
/* ffl */           	"\203ffl",
/* degree */        	"\000\0",
/* dagger */        	"\000\0",
/* section */       	"\000\0",
/* foot mark */     	"\000\0",
/* acute accent */  	"\000\0",
/* grave accent */  	"\000\0",
/* underrule */     	"\001_",
/* slash (longer) */	"\001/",
/* half narrow space */	"\000\0",
/* unpaddable space */	"\001 ",
/* alpha */         	"\000\0",
/* beta */          	"\000\0",
/* gamma */         	"\000\0",
/* delta */         	"\000\0",
/* epsilon */       	"\000\0",
/* zeta */          	"\000\0",
/* eta */           	"\000\0",
/* theta */         	"\000\0",
/* iota */          	"\000\0",
/* kappa */         	"\000\0",
/* lambda */        	"\000\0",
/* mu */            	"\000\0",
/* nu */            	"\000\0",
/* xi */            	"\000\0",
/* omicron */       	"\000\0",
/* pi */            	"\000\0",
/* rho */           	"\000\0",
/* sigma */         	"\000\0",
/* tau */           	"\000\0",
/* upsilon */       	"\000\0",
/* phi */           	"\000\0",
/* chi */           	"\000\0",
/* psi */           	"\000\0",
/* omega */         	"\000\0",
/* Gamma */         	"\000\0",
/* Delta */         	"\000\0",
/* Theta */         	"\000\0",
/* Lambda */        	"\000\0",
/* Xi */            	"\000\0",
/* Pi */            	"\000\0",
/* Sigma */         	"\000\0",
/* Tau */           	"\000\0",
/* Upsilon */       	"\000\0",
/* Phi */           	"\000\0",
/* Psi */           	"\000\0",
/* Omega */         	"\000\0",
/* square root */   	"\000\0",
/* terminal sigma */	"\000\0",
/* root en */       	"\000\0",
/* >= */            	"\000\0",
/* <= */            	"\000\0",
/* identically equal */	"\000\0",
/* equation minus */	"\000\0",
/* approx = */      	"\000\0",
/* approximates */  	"\000\0",
/* not equal */     	"\000\0",
/* right arrow */   	"\000\0",
/* left arrow */    	"\000\0",
/* up arrow */      	"\000\0",
/* down arrow */    	"\000\0",
/* eqn equals */    	"\000\0",
/* multiply */      	"\000\0",
/* divide */        	"\000\0",
/* plus-minus */    	"\000\0",
/* cup (union) */   	"\000\0",
/* cap (intersection) */	"\000\0",
/* subset of */     	"\000\0",
/* superset of */   	"\000\0",
/* improper subset */	"\000\0",
/*  improper superset */	"\000\0",
/* infinity */      	"\000\0",
/* pt deriv */      	"\000\0",
/* gradient */      	"\000\0",
/* not */           	"\000\0",
/* integral */      	"\000\0",
/* proportional to */	"\000\0",
/* empty set */     	"\000\0",
/* member of */     	"\001+",
/* equation plus */ 	"\000\0",
/* registration mk */	"\000\0",
/* copyright mk */  	"\001|",
/* box rule */      	"\000\0",
/* cent sign */     	"\000\0",
/* dbl dagger */    	"\000\0",
/* right hand */    	"\000\0",
/* left hand */     	"\001*",
/* math *  */       	"\000\0",
/* bell system sign */	"\001|",
/* or (was star) */ 	"\000\0",
/* circle */        	"\001|",
/* left top of big curly */	"\001|",
/* left bottom of big curly */	"\001|",
/* right top of big curly */	"\001|",
/* right bottom of big curly */	"\001|",
/* left center of big curly */	"\001|",
/* right center of big curly */	"\001|",
/* bold vertical rule */	"\001|",
/* left bottom of big bracket */	"\001|",
/* right bottom of big bracket */	"\001|",
/* left top of big bracket */	"\001|",
/* right top of big bracket */	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\000\0",
/* ??? */           	"\033E\033D\001",
};
