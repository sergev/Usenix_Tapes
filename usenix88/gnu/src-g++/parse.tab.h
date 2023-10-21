typedef union {long itype; tree ttype; enum tree_code code; char *cptr; } YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
 {
      int timestamp;
      int first_line;
      int first_column;
 int last_line;
      int last_column;
      char *text;
   }
 yyltype;

#define YYLTYPE yyltype
#endif

#define	YYACCEPT	return(0)
#define	YYABORT	return(1)
#define	YYERROR	return(1)
#define	IDENTIFIER	258
#define	TYPENAME	259
#define	SCSPEC	260
#define	TYPESPEC	261
#define	TYPE_QUAL	262
#define	CONSTANT	263
#define	STRING	264
#define	ELLIPSIS	265
#define	SIZEOF	266
#define	ENUM	267
#define	IF	268
#define	ELSE	269
#define	WHILE	270
#define	DO	271
#define	FOR	272
#define	SWITCH	273
#define	CASE	274
#define	DEFAULT	275
#define	BREAK	276
#define	CONTINUE	277
#define	RETURN	278
#define	GOTO	279
#define	ASM	280
#define	TYPEOF	281
#define	ALIGNOF	282
#define	AGGR	283
#define	DELETE	284
#define	NEW	285
#define	OVERLOAD	286
#define	PRIVATE	287
#define	PUBLIC	288
#define	PROTECTED	289
#define	THIS	290
#define	OPERATOR	291
#define	SCOPE	292
#define	EMPTY	293
#define	ASSIGN	294
#define	RANGE	295
#define	OROR	296
#define	ANDAND	297
#define	MIN_MAX	298
#define	EQCOMPARE	299
#define	ARITHCOMPARE	300
#define	LSHIFT	301
#define	RSHIFT	302
#define	UNARY	303
#define	PLUSPLUS	304
#define	MINUSMINUS	305
#define	HYPERUNARY	306
#define	POINTSAT	307
#define	TYPENAME_COLON	308
#define	TYPENAME_SCOPE	309
#define	TYPENAME_ELLIPSIS	310
#define	PRE_PARSED_FUNCTION_DECL	311

