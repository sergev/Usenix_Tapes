%{
/*----------------------------------------------------------------------*
 *	if command - shell help routine.				*
 *									*
 *	Yacc driven parser allows constructs of the form:		*
 *		if <expression> <executable statement>			*
 *	note: <executable statement> may NOT be another if		*
 *		if <expression> then					*
 *			<statement list>				*
 *		fi							*
 *	where <statement list> is ANY sequence of executable		*
 *	commands (i.e. it may include if's). This construct		*
 *	may only be executed inside a shell script - attempt		*
 *	to execute an if-then from the terminal is ignored.		*
 *									*
 *	The <expression> is built from any of the following connectives	*
 *		!	- logical not
 *		-a	- logical and					*
 *		-o	- logical or					*
 *		()	- parentheses for grouping			*
 *			 note: these are significant to the shell	*
 *			 and must be escaped at the command level	*
 *	and the following primitives:					*
 *		-r <file>	- is file readable			*
 *		-w <file>	- is file writable			*
 *		-s <file>	- is file of nonzero length		*
 *		-f <file>	- is file plain file			*
 *		-d <file>	- is file a directory			*
 *		-z <string>	- is string of zero length		*
 *		-n <string>	- is string of nonzero length		*
 *	<string> = <string>	- are strings equal			*
 *	<string> != <string>	- are strings unequal			*
 *	<number> -eq <number>	- are numbers equal			*
 *	<number> -ne <number>	- are numbers unequal			*
 *	<number> -gt <number>	- greater than relation			*
 *	<number> -ge <number>	- greater than or equal relation	*
 *	<number> -lt <number>	- less than relation			*
 *	<number> -le <number>	- less than or equal relation		*
 *		{ command }	- use exit status of command		*
 *									*
 *	The connectives are related in precedence as follows		*
 *		! *> -a *> -o						*
 *	Bundling is used in the parser, and the scanner expects		*
 *	all arguments to be passed through the main program.		*
 *----------------------------------------------------------------------*/
int iflag = 0;
%}
%token IF	257
%token THEN	258
%token not	259
%token or	260
%token and	261
%token _read	262
%token _write	263
%token size	264
%token ordinary	265
%token directory 266
%token zero	267
%token nonzero	268
%token equals	269
%token notequals 270
%token equ	271
%token neq	272
%token gtr	273
%token geq	274
%token lss	275
%token leq	276
%token string	277

%{
# define READ	0
# define WRITE	1
# define tok_count	20
# define bsize	35
# define max_args	15

# define tok_string	277

# include "stat.h"

# define FTYPE	060000
char *tokens[tok_count] = {
	"if", "then", "!", "-o", "-a", "-r", "-w", "-s",
	"-f", "-d", "-z", "-n", "=", "!=", "-eq", "-ne",
	"-gt", "-ge", "-lt", "-le" };
unsigned *g_value;
int *bspace[bsize], **bptr = bspace;
char *arglist[max_args];
int g_argc;
char **g_argv;
%}

%%

/*	1	*/S :	IF expr_part S
		={ if($2){
			dosys($3);
			dexit(0);
		   }else
			dexit(1);
		}
		| IF expr_part THEN
		={
			if($2){
				dexit(0);
			}else
				dexit(2);
		}
		| command = { $$ = $1; }
		| error ;

/*	5	*/expr_part : expr = { $$ = $1; iflag = 1; }

/*	5	*/command : arg_list = { $$ = $1; }

/*	6	*/arg_list : arg_list arg = { $$ = bundle($1, $2); }
		| arg = { $$ = g_value; } 

/*	8	*/expr : term = { $$ = $1; }
		| expr or term = { $$ = ($1 || $3); }
		| not expr = { $$ = !($2); }

/*	11	*/term : factor = { $$ = $1; }
		| term and factor = { $$ = ($1 && $3); }

/*	13	*/factor : primitive = { $$ = $1; }
		| '(' expr ')' ={ $$ = $2; } 

/*	15	*/primitive : _read file
		= {int fd;
			$$ = (((fd = open(g_value, READ)) < 0) ? 0 : 1);
			close(fd);
		}
		| _write file
		= { int fd;
			$$ = (((fd = open(g_value, WRITE)) < 0) ? 0 : 1);
			close(fd);
		}
		| size file
		= {
		struct inode buf;
			stat(g_value, &buf);
			$$ = !((buf.i_size0 == 0) && (buf.i_size1 == 0));
		}
		| ordinary file
		={
		struct inode buf;
			stat(g_value, &buf);
			$$ = (buf.i_mode == 0);
		}
		| directory file
		= {
		struct inode buf;
			stat(g_value, &buf);
			$$ = (buf.i_mode == IFDIR);
		}
		| zero STRING = { $$ = (*g_value ? 0 : 1); }
		| nonzero STRING = { $$ = (*g_value != 0 ? 1 : 0); }
		| STRING equals STRING = { $$ = (compar($1, $3) ? 0 : 1); }
		| STRING notequals STRING = { $$ = (compar($1, $3) ? 1 : 0); }
		| STRING equ STRING = { $$ = (atoi($1) == atoi($3)); }
		| STRING neq STRING = { $$ = (atoi($1) != atoi($3)); }
		| STRING gtr STRING = { $$ = (atoi($1) > atoi($3)); }
		| STRING geq STRING = { $$ = (atoi($1) >= atoi($3)); }
		| STRING lss STRING = { $$ = (atoi($1) < atoi($3)); }
		| STRING leq STRING = { $$ = (atoi($1) <= atoi($3)); }
		| '{' command '}' 
		={ int status;
			if(fork())
				wait(&status);
			else
				dosys($2);
			$$ = (status ? 0 : 1);
		}
	
/*	31	*/STRING : string = { $$ = g_value; }

/*	33	*/arg :	string = { $$ = g_value; }

/*	34	*/file : string = { $$ = g_value; }

%%

/*	Lexical scanner - matches input from main's arguments	*/

yylex()
{
register j;
register char *s_argv;

	if(g_argc-- > 0){
		if((**g_argv == '(') | (**g_argv == ')') | (**g_argv == '{')
			| (**g_argv == '}'))
			return(**g_argv++);
		if(iflag){
			g_value = *g_argv++;
			return(tok_string);
		}
		for(j=0; j< tok_count; j++)
			if(compar(tokens[j], *g_argv) == 0){
				g_argv++;
# ifdef DEBUG
				printf("match on token %s\n", tokens[j]);
# endif
				return(257+j);
			}
		s_argv = g_value = *g_argv++;
# ifdef DEBUG
		printf("match on string : %s\n", g_value);
# endif
		return(tok_string);
	}
	return(0);
}

/*	Bundling routines to handle commands and their arguments	*/

bundle(a1, a2, a3)
{
register int i, j, *p;
int *obp;

	p = &a1;
	i = nargs();
	obp = bptr;
	if(obp > &bspace[bsize-1])
		dexit(8);

	for(j=0; j<i; ++j)
		*bptr++ = *p++;
	*bptr++ = 0;
	return(obp);
}
/*	performs pexec call on command bundle passed	*/

dosys(p)
int *p;
{
int status;
int i;
char *args[2];
	makeargs(p);
	if(fork())
		wait(&status);
	else{
		pexec(arglist[0], arglist);
		dexit(10);
	}
}

/*	unbundles arguments to command - inverse function of bundle (sort of) */
makeargs(p)
int *p;
{
static int i;
	if((p>= bspace) && (p < &bspace[bsize]))
		while(*p)
			makeargs(*p++);
	else{
		arglist[i++] = p;
		arglist[i] = 0;
	}
}

compar(s1, s2)
register char *s1, *s2;
{
register char c1, c2;

	while((c1 = *s1++) == (c2 = *s2++))
		if(c1 == 0)
			return(0);
	return(c1 - c2);
}

/*	main allows global routines to see argc and argv	*/

main(argc, argv)
int argc; char *argv[];
{

	g_argc = argc;
	g_argv = argv;
	pexinit();
	yyparse();
}

/*
 *	dexit - exit and cleanup when neccessary
 *	dexit switches on three types of exit conditions :
 *		normal exit - no error, return 0 exit status
 *		error exit  - error, return > 0 exit status
 *		if-fi type exit - search standard input for matching
 *		 fi when expression in if <expr> then construct is false,
 *		 there is checking as to allow nesting of if-fi's.
 *	All error messages should come from here, or yyerror (for a bad parse)
 *	Exit status value tells what the error code was that was passed to 
 *		dexit.
 */

dexit(code)
int code;
{
	switch(code){
	case 0:
	case 1:
		exit(0);
	case 2:
		{
		int type, depth;
		struct inode buf;
			depth = 0;
			fstat(0, &buf);
			if(buf.i_mode & FTYPE) exit(0);
			while(1){
				if((type = gettok()) == -1){
					write(2, "if: missing fi\n", 15);
					exit(2); 
				}
				if(type == 0)
					depth++;
				if(type == 1){
					if(depth) depth--;
					else
						exit(0);
				}
			}
		}
	case 8:
		write(2, "if: too many args\n", 18);
		exit(8);
	case 10:
		write(2, "if: no command\n", 15);
		exit(10);
	default:
		write(2, "if: internal error\n", 19);
		exit(-1);
	}
}

/*
 *	quick and dirty finite state machine to recognize 'then' and 'fi'
 *	Returns -1 on end of file.
 */
gettok()
{
char ch;
register state;
int type;
extern char getchar();

	state = 0;
	while((ch = getchar()) > 0){
		switch(ch){
		case '\t':
		case ' ':
			if((state == 4)|(state == 5))
				return(type);
			else
				state = 0;
			break;
		case 't':
			if(state == 0)
				state = 1;
			else
				state = -1;
			break;
		case 'h':
			if(state == 1)
				state = 2;
			else
				state = -1;
			break;
		case 'e':
			if(state == 2)
				state = 3;
			else
				state = -1;
			break;
		case 'n':
			if(state == 3){
				state = 4;
				type = 0;
			}else
				state = -1;
			break;
		case 'i':
			if(state == 6){
				state = 5;
				type = 1;
			}else
				state = -1;
			break;
		case 'f':
			if((state == 7)|(state == 0))
				state = 6;
			else
				state = -1;
			break;
		case '\n':
			if((state == 5)|(state == 4))
				return(type);
			else
				state = 7;
			break;
		default:
			state = -1;
		}
	}
	if((state == 5)|(state == 4)) return(type);
	return(-1);
}
char getchar()
{
char c;
if(read(0, &c, 1) == 1)
	return(c);
else
	return(-1);
}

yyerror()
{
write(2, "if: syntax error\n", 17);
exit(1);
}
