/* tabiwp.c

	proportional nroff table for Imagewriter II (I) printer

	copyright (c) 1987 Stuart Lynne

*/

/*
	This file defines an nroff table using the imagewriter's
	built in proportional font.

	To construct tabiwp you must have the table generation software.
	This has been tested with Bruce Townsend's version.

	Because of the limitations of this table and nroff, post processing
	support is required. See the filter iw(1ml).

	Specifically this table assumes that iw is called after nroff:

		nroff .... | col ... | iw | ...
	
	The iw filter defines a special char in the alternate font. A one
	dot width space for nroff to do proportional spacing with.

	Nroff uses this in a round about way. By dropping into plot mode
	and emitting the move right sequence. In our case simply the 
	one dot width space.

*/



#include	<termio.h>	/* Req'd only for bset, breset */

#define INCH 240

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
/*bset    */		0,
/*breset  */		ONLCR | OCRNL | ONLRET,
/*Hor     */		INCH / 120, /* 24, */ /*INCH / 60,*/
/*Vert    */		40, /*INCH / 48,*/
/*Newline */		INCH / 6,
/*Char    */		INCH / 120,
/*Em      */		INCH / 120 * 7, /* seem to want size of sp */
/*Halfline*/		INCH / 12,
/*Adj     */		INCH / 120,
/*twinit  */		"", 
/*twrest  */		"",
/*twnl    */		"\033\"\r\n",	/* turn boldface off, newline */
/*hlr     */		"",
/*hlf     */		"",
/*flr     */		"",
/*bdon    */		"\033!",	/* turn boldface on */
/*bdoff   */		"\033\"",	/* turn boldface off */
/*iton    */		"\033X",	/* turn underline on */ 
/*itoff   */		"\033Y", 	/* turn underline off */
/*ploton  */		"\033'",	/* turn alternate font on */
/*plotoff */		"\033$",	/* turn alternate font off */
/*up      */		"",
/*down    */		"",
/*right   */		"~", /* requires support from iw - vis-a-vis special char */
/*left    */		"",
/* space */         	"\007 ", 
/* space   */		/*"\001\033'~\033$", */
/* ! */             	"\007!",
/* " */             	"\012\"",
/* # */             	"\016#",
/* $ */             	"\014$",
/* % */             	"\020%",
/* & */             	"\015&",
/* ' close */       	"\007'",
/* ( */             	"\007(",
/* ) */             	"\007)",
/* * */             	"\014*",
/* + */             	"\014+",
/* , */             	"\007,",
/* - hyphen */      	"\014-",
/* . */             	"\007.",
/* / */             	"\014/",
/* 0 */             	"\2140",
/* 1 */             	"\2141",
/* 2 */             	"\2142",
/* 3 */             	"\2143",
/* 4 */             	"\2144",
/* 5 */             	"\2145",
/* 6 */             	"\2146",
/* 7 */             	"\2147",
/* 8 */             	"\2148",
/* 9 */             	"\2149",
/* : */             	"\007:",
/* ; */             	"\007;",
/* < */             	"\014<",
/* = */             	"\014=",
/* > */             	"\014>",
/* ? */             	"\014?",
/* @ */             	"\016@",
/* A */             	"\220A",
/* B */             	"\217B",
/* C */             	"\216C",
/* D */             	"\217D",
/* E */             	"\217E",
/* F */             	"\217F",
/* G */             	"\216G",
/* H */             	"\217H",
/* I */             	"\211I",
/* J */             	"\215J",
/* K */             	"\214K",
/* L */             	"\215L",
/* M */             	"\221M",
/* N */             	"\220N",
/* O */             	"\217O",
/* P */             	"\215P",
/* Q */             	"\220Q",
/* R */             	"\217R",
/* S */             	"\214S",
/* T */             	"\216T",
/* U */             	"\217U",
/* V */             	"\220V",
/* W */             	"\221W",
/* X */             	"\213X",
/* Y */             	"\216Y",
/* Z */             	"\213Z",
/* [ */             	"\014[",
/* \ */             	"\014\\",
/* ] */             	"\014]",
/* ^ */             	"\014^",
/* _ dash */        	"\021_",
/* ` open */        	"\007`",
/* a */             	"\214a",
/* b */             	"\214b",
/* c */             	"\212c",
/* d */             	"\214d",
/* e */             	"\214e",
/* f */             	"\212f",
/* g */             	"\214g",
/* h */             	"\214h",
/* i */             	"\210i",
/* j */             	"\207j",
/* k */             	"\212k",
/* l */             	"\210l",
/* m */             	"\220m",
/* n */             	"\214n",
/* o */             	"\214o",
/* p */             	"\214p",
/* q */             	"\214q",
/* r */             	"\212r",
/* s */             	"\214s",
/* t */             	"\212t",
/* u */             	"\214u",
/* v */             	"\214v",
/* w */             	"\220w",
/* x */             	"\214x",
/* y */             	"\214y",
/* z */             	"\212z",
/* { */             	"\012{",
/* | */             	"\007|",
/* } */             	"\012}",
/* ~ */             	"\015~",
/* narrow sp */     	"\007 ",
/* hyphen */        	"\014-",
/* bullet */        	"\000\0",
/* square */        	"\000\0",
/* 3/4 em */        	"\014-",
/* rule */          	"\021_",
/* 1/4 */           	"\0401/4",
/* 1/2 */           	"\0401/2",
/* 3/4 */           	"\0403/4",
/* minus */         	"\014-",
/* fi */            	"\222fi",
/* fl */            	"\222fl",
/* ff */            	"\224ff",
/* ffi */           	"\232ffi",
/* ffl */           	"\203ffl",
/* degree */        	"\000\0",
/* dagger */        	"\000\0",
/* section */       	"\000\0",
/* foot mark */     	"\000\0",
/* acute accent */  	"\000\0",
/* grave accent */  	"\000\0",
/* underrule */     	"\021_",
/* slash (longer) */	"\014/",
/* half narrow space */	"\000\0",
/* unpaddable space */	"\007 ",
/* alpha */         	"\000\0",
/* beta */          	"\000\0",
/* gamma */         	"\000\0",
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
/* member of */     	"\000\0",
/* equation plus */ 	"\014+",
/* registration mk */	"\000\0",
/* copyright mk */  	"\000\0",
/* box rule */      	"\007|",
/* cent sign */     	"\000\0",
/* dbl dagger */    	"\000\0",
/* right hand */    	"\000\0",
/* left hand */     	"\000\0",
/* math *  */       	"\014*",
/* bell system sign */	"\000\0",
/* or (was star) */ 	"\007|",
/* circle */        	"\000\0",
/* left top of big curly */	"\007|",
/* left bottom of big curly */	"\007|",
/* right top of big curly */	"\007|",
/* right bottom of big curly */	"\007|",
/* left center of big curly */	"\007|",
/* right center of big curly */	"\007|",
/* bold vertical rule */	"\007|",
/* left bottom of big bracket */	"\007|",
/* right bottom of big bracket */	"\007|",
/* left top of big bracket */	"\007|",
/* right top of big bracket */	"\007|",
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
/* ??? */           	"\000\0",
};
