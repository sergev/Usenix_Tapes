/*
**	1stlatex.w	1st_word to latex conversion program
**	file : 1stwart.w	automaton.
**	version 1.0
**
**	Jerome M. Lang
**	Public Domain
**	7 March 1985.
**
**	This file needs to be preprocessed with ckwart, the function
**	wart() will be produced. Do not compile the .c file but
**	include it in 1stlatex.
*/
/*
**	1986-04-28	: added double line spacing.
*/
%states	charmode condmode stylemode formatmode igncrlf ignlf

%%
<charmode>C	{ output(Ch); }			/* reg char: pass it through */
<charmode>B	{ BEGIN	condmode; }		/* cond. page break */
<charmode>E	{ if( INSPACE == Prch){
			BEGIN igncrlf;		/* double line spacing */
		  }
		}				/* ignore stretch space */
<charmode>F	{ outstr("\n\\pagebreak \n"); } /* new page command */
<charmode>H	{ BEGIN formatmode; }		/* header line */
<charmode>I	{ }				/* indent space, ign. for now */
<charmode>L	{ output(Ch); }			/* line feed */
<charmode>N	{ }				/* Null, ignored */
<charmode>Q	{ output('{'); quote( Ch );	/* Quoted characters */ 
			output('}'); }
<charmode>R	{ if ( (VSPACE != Prch) && !underline )	/* carriage return */
			{ outstr("\n"); }}	/*  - try separate paragraphs */
<charmode>S	{ BEGIN stylemode; }		/* ok, here comes style info */
<charmode>T	{ }				/* Tab - ignored */
<charmode>U	{ Cconws("\007Warning, not implemented:"); /* Unimplemented */
			Cconout(Ch);Cconws("\r\n"); }
<charmode>V	{ output(' '); }		/* variable space */
<charmode>X	{ output(' '); }		/* fixed space */
<charmode>Z	{ return; }
<charmode>.	{ fatal( " charmode: weird mode, internal error" ); }
<condmode>Z	{ fatal( " End of file reached while waiting for cond.mode arg"
			); }
<condmode>.	{ BEGIN charmode; }		/* ignore cond. page break */
<formatmode>L	{ BEGIN charmode; }		/* ignore header */
<formatmode>Z	{ fatal( " End of file reached while reading format" ); }
<formatmode>.	{ }				/*   up to end of line */
<stylemode>Z	{ fatal( " End of file reached while reading style arg" ); }
<stylemode>.	{ /*Crawio(Prch);Crawio(Ch); Cconin()*/; doStyle( Ch ); BEGIN charmode; }	/* process style info */
<igncrlf>R	{ BEGIN	ignlf; }			/* ignore CRLF chars */
<igncrlf>Z	{ fatal( " End of file reached while reading format" ); }
<igncrlf>.	{ output(Ch); BEGIN charmode; }
<ignlf>L	{ BEGIN charmode; } /* ignore line feed char */
<ignlf>Z	{ fatal( " End of file reached while reading format" ); }
<ignlf>.	{ output(Ch); BEGIN charmode; }
%%
