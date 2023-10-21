/*--- ktypes.h ------------------------------------------------------
Keyword types for the Pascal to C translator.
3/25/87 Daniel Kegel (seismo!rochester!srs!dan)
---------------------------------------------------------------------*/
#define T_ZIP		0	/* Nondescript identifier */
#define T_BEGIN		1	/* BEGIN */
#define T_END		2	/* END */
#define T_PROC		3	/* PROCEDURE */
#define T_FUNC		4	/* FUNCTION */
#define T_FORWARD	5	/* FORWARD */
#define T_CONST 	6	/* CONST */
#define T_VAR	 	7	/* VAR */
#define T_COMPARE	8	/* ==, <>, >, < */
#define T_EQUALS	9	/* = alone; in CONST, TYPE or comparison */
#define T_COLON 	10	/* : alone; in VAR, READ, or WRITE */
#define T_SEMI		11	/* ; alone */
#define T_LPAREN	12	/* ( alone */
#define T_RPAREN	13	/* ) alone */
#define T_SPACE 	14	/* a string of blanks, tabs, and/or newlines */
#define T_STRUCTMEMBER	15	/* ^. */
#define T_ASSIGN	16	/* := */
#define T_STRING	17	/* quoted string */
#define T_COMMENT	18	/* comment text */
#define T_EOF		19	/* end of source file */
#define T_COMMA		20	/* , */
#define T_LABEL		21	/* LABEL */
#define T_DEREF		22	/* ^ alone */
#define T_LBRACKET	23	/* [ */
#define T_RBRACKET	24	/* ] */
#define T_ARRAY		25	/* ARRAY */
#define T_RANGE		26	/* .. */
#define T_OF		27	/* OF */
#define T_RECORD	28	/* RECORD */
#define T_FILE		29	/* FILE */
#define T_TYPE		30	/* TYPE */
#define T_STRINGTYPE	31	/* STRING(n) or STRING[n] type */
#define T_CASE		32	/* CASE */
