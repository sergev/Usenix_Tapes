	/*
	**	lexical analyzer for input to Redcode Assembler
	**
	** [cw by Peter Costantinidis, Jr. @ University of California at Davis]
	**
	static	char	rcsid[] = "$Header: rcasm.l,v 7.0 85/12/28 14:37:16 ccohesh Dist $";
	**
	*/
	/* definitions */
I	-?[0-9]+
C	\/.*$
W	[" "\t]+
L	"$"[a-zA-Z0-9]+
%%
{C}	return(COMMENT);
dat|DAT	return(opcode=DAT, orc=1, OPCODE);
mov|MOV	return(opcode=MOV, orc=2, OPCODE);
add|ADD	return(opcode=ADD, orc=2, OPCODE);
sub|SUB	return(opcode=SUB, orc=2, OPCODE);
jmp|JMP	return(opcode=JMP, orc=1, OPCODE);
jmz|JMZ	return(opcode=JMZ, orc=2, OPCODE);
djz|DJZ	return(opcode=DJZ, orc=2, OPCODE);
cmp|CMP	return(opcode=CMP, orc=2, OPCODE);
mul|MUL	return(opcode=MUL, orc=2, OPCODE);
div|DIV	return(opcode=DIV, orc=2, OPCODE);
rnd|RND	return(opcode=RND, orc=1, OPCODE);
"#"	return(OIMM);
"@"	return(OIND);
"\n"	return(NL);
{W}	;
^{L}":"	{
		static	char	buf[LABEL_LEN+1];
		auto	int	len = min(yyleng-2, LABEL_LEN);

		strncpy(buf, &(yytext[1]), len);
		buf[len] = '\0';
		dlabel = &(buf[0]);
		return(DLABEL);
	}
{L}	{
		static	char	bufs[2][LABEL_LEN+1];
		auto	int	len = min(yyleng+1, LABEL_LEN);
		auto	char	**ptr;

		if (orc == 1)
			ptr = &blabel;
		else
			ptr = (toggle ? &blabel : &alabel);
		strncpy(bufs[toggle], &(yytext[1]), len);
		bufs[toggle][len] = '\0';
		*ptr = bufs[toggle];
		toggle = (toggle ? 0 : 1);
		return(LABEL);
	}
{I}	{
		extern	int	yylval;

		if (sscanf(yytext, "%d", &yylval) != 1)
			return(ERROR);
		yylval = sum(yylval, 0);
		toggle = (toggle ? 0 : 1);
		return(INTEGER);
	}
.	
	{
		fprintf(stderr, "%s: unrecognized character in lex: %o\n",
			argv0, yytext[0]);
		return(ERROR);
	}
%%
