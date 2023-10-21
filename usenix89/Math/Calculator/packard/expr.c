
/*
 *	expr.c
 *
 *	handle expression trees
 */

# include	"ic.h"
# include	"y.tab.h"

extern double	pow();

# define NSTACK	200

double	stack[NSTACK];
double	*fstack[NSTACK];
double	*stackp = stack + NSTACK;
double	**fstackp = fstack + NSTACK;
double	*framep;

# define N_NODES	200

int	usedfirst = 0;
expr	firstblock[N_NODES];

expr *exprhead;

expr *
allocexpr()
{
	expr	*e;

	if (!exprhead) {
		if (!usedfirst) {
			exprhead = firstblock;
			++usedfirst;
		} else
			exprhead = (expr *) malloc (N_NODES * sizeof (expr));
		e = exprhead;
		while (e < exprhead + N_NODES - 1) {
			e->e_left = e + 1;
			++e;
		}
		e->e_left = 0;
	}
	e = exprhead;
	exprhead = e->e_left;
	e->e_tag = 0;
	return e;
}

freeexpr (e)
expr	*e;
{
	if (!e)
		return;
	if (e->e_tag == STRING)
		free (e->e_string);
	e->e_left = exprhead;
	exprhead = e;
}

freetree (e)
expr	*e;
{
	if (!e)
		return;
	switch (e->e_tag) {
	case NAME:
	case NUMBER:
	case STRING:
		break;
	default:
		freetree (e->e_left);
		freetree (e->e_right);
	}
	freeexpr (e);
}

expr *
buildOp(val, left, right)
expr *left, *right;
{
	register expr *foo = allocexpr();

	foo->e_tag = val;
	foo->e_left = left;
	foo->e_right = right;
	return foo;
}

expr *
buildNum(val)
double	val;
{
	register expr *foo = allocexpr();

	foo->e_tag = NUMBER;
	foo->e_number = val;
	return foo;
}

expr *
buildStr(s)
char	*s;
{
	register expr *foo = allocexpr();
	foo->e_tag = STRING;
	foo->e_string = s;
	return foo;
}

expr *
buildVar(val)
symbol	*val;
{
	register expr *foo = allocexpr();

	foo->e_tag = NAME;
	foo->e_name = val;
	return foo;
}

double	result;
# define NSTRINGS	100
char	*stringstack[NSTRINGS];
char	**stringsp = stringstack + NSTRINGS;
# define MAXARGS	10

double
eeval(f)
register expr	*f;
{
	register symbol	*s;
	register double	r;
	register int	args;
	register char	**strsp;
	double		argt[MAXARGS];
	double		*argp;

	if (!f)
		return 1.0;
	switch (f->e_tag) {
	case NAME:
		s = f->e_name;
		switch (s->s_type) {
		case UNDEF:
			s->s_type = VARTYPE;
			s->s_value = 0.0;
		case VARTYPE:
			return s->s_value;
		case STACKTYPE:
			return framep[s->s_offset];
		default:
			eerror ("illegal use of identifier");
			return 0.0;
		}
	case OP:	/* call function */
		s = f->e_left->e_name;
		if (s->s_type != FUNCTYPE && s->s_type != BUILTIN) {
			eerror ("illegal use of identifier");
			return 0.0;
		}
		f = f->e_right;
		argp = argt + MAXARGS;
		args = 0;
		strsp = stringsp;
		while (f) {
			*--argp = eeval (f->e_left);
			f = f->e_right;
			++args;
		}
		result = call (s, args, argp);
		stringsp = strsp;
		return result;
	case NUMBER:
		return f->e_number;
	case STRING:
		*--stringsp = f->e_string;
		return 0.0;
	case PLUS:
		return eeval(f->e_left) + eeval(f->e_right);
	case MINUS:
		return eeval(f->e_left) - eeval(f->e_right);
	case DIVIDE:
		return eeval(f->e_left) / eeval(f->e_right);
	case TIMES:
		return eeval(f->e_left) * eeval(f->e_right);
	case MOD:
		return (double) (((int) eeval(f->e_left)) % ((int) eeval(f->e_right)));
	case POW:
		return pow (eeval (f->e_left), eeval (f->e_right));
	case EQ:
		return eeval(f->e_left) == eeval(f->e_right);
	case NE:
		return eeval(f->e_left) != eeval(f->e_right);
	case LT:
		return eeval(f->e_left) < eeval(f->e_right);
	case GT:
		return eeval(f->e_left) > eeval(f->e_right);
	case LE:
		return eeval(f->e_left) <= eeval(f->e_right);
	case GE:
		return eeval(f->e_left) >= eeval(f->e_right);
	case UMINUS:
		return -eeval(f->e_left);
	case FACT:
		args = eeval (f->e_right);
		r = 1;
		while (args > 0)
			r *= args--;
		return r;
	case BANG:
		return !eeval(f->e_left);
	case QUEST:
		return (eeval(f->e_left) ?
			eeval(f->e_right->e_left) :
			eeval(f->e_right->e_right));
	case AND:
		return eeval(f->e_left) && eeval(f->e_right);
	case OR:
		return eeval(f->e_left) || eeval(f->e_right);
	case ASSIGN:
		s = f->e_left->e_name;
		r = eeval(f->e_right);
		switch (s->s_type) {
		case UNDEF:
			s->s_type = VARTYPE;
		case VARTYPE:
			s->s_value = r;
			break;
		case STACKTYPE:
			framep[s->s_offset] = r;
			break;
		default:
			eerror ("illegal use of identifier");
		}
		return r;
	case INC:
		if (f->e_left == 0) {
			s = f->e_right->e_name;
			switch (s->s_type) {
			case UNDEF:
				s->s_type = VARTYPE;
			case VARTYPE:
				r = s->s_value;
				s->s_value += 1;
				break;
			case STACKTYPE:
				r = framep[s->s_offset];
				framep[s->s_offset] += 1;
			}
			return r;
		} else {
			s = f->e_right->e_name;
			switch (s->s_type) {
			case UNDEF:
				s->s_type = VARTYPE;
			case VARTYPE:
				return (s->s_value += 1);
			case STACKTYPE:
				return (framep[s->s_offset] += 1);
			}
		}
	case DEC:
		if (f->e_left == 0) {
			s = f->e_right->e_name;
			switch (s->s_type) {
			case UNDEF:
				s->s_type = VARTYPE;
			case VARTYPE:
				r = s->s_value;
				s->s_value -= 1.0;
				break;
			case STACKTYPE:
				r = framep[s->s_offset];
				framep[s->s_offset] -= 1.0;
			}
			return r;
		} else {
			s = f->e_right->e_name;
			switch (s->s_type) {
			case UNDEF:
				s->s_type = VARTYPE;
			case VARTYPE:
				return (s->s_value -= 1.0);
			case STACKTYPE:
				return (framep[s->s_offset] -= 1.0);
			}
		}
	}
}

double
call(s, argc, argv)
register symbol	*s;
register double	*argv;
{		
	int	c;

	if (argc != (s->s_argc & 255) && s->s_argc != -1) {
		char	buf[256];
		
		sprintf (buf,
		    "function %s requiring %d arguments was called with %d",
		    s->s_name, s->s_argc, argc);
		eerror (buf);
		return 0.0;
	}
	if (s->s_type == FUNCTYPE) {
		argv += argc;
		c = argc;
		while (c-- > 0) {
			*--stackp = *--argv; 
		}
		*--fstackp = framep;
		framep = stackp;
		if (!s->s_expr) {
			eerror ("function is not compiled yet");
			return 0.0;
		}
		eval (s->s_expr);
		framep = *fstackp++;
		stackp += argc;
		return result;
	} else {
		switch (s->s_argc) {
		case -1:
			return (*s->s_builtin)(argc, argv);
		case 0:
			return (*s->s_builtin)();
		case 1:
			return (*s->s_builtin)(argv[0]);
		case 2:
			if (s->s_argc & 256)
				return (*s->s_builtin)((int) argv[0], argv[1]);
			else
				return (*s->s_builtin)(argv[0], argv[1]);
		case 3:
			return (*s->s_builtin)(argv[0], argv[1],
				argv[2]);
		case 4:
			return (*s->s_builtin)(argv[0], argv[1],
				argv[2], argv[3]);
		}
	}
}

eval(f)
expr	*f;
{
	register int tmp;

	switch (f->e_tag) {
	case EXPR:
		eeval(f->e_left);
		break;
	case IF:
		if (eeval(f->e_left))
			return eval(f->e_right);
		break;
	case ELSE:
		if (eeval(f->e_left))
			return eval(f->e_right->e_left);
		else
			return eval(f->e_right->e_right);
	case WHILE:
		while (eeval(f->e_left))
			switch (eval(f->e_right)) {
			case BRK:
				return 0;
			case RET:
				return RET;
			}
		break;
	case DO:
		do
			switch (eval(f->e_right)) {
			case BRK:
				return 0;
			case RET:
				return RET;
			}
		while (eeval(f->e_right));
		break;
	case FOR:
		for (eeval(f->e_left->e_left); eeval(f->e_left->e_right);
			eeval(f->e_right->e_left))
			switch (eval(f->e_right->e_right)) {
			case BRK:
				return 0;
			case RET:
				return RET;
			}
		break;
	case OC:
		do {
			switch (tmp = eval(f->e_left)) {
			case CONT:
			case BRK:
			case RET:
				return tmp;
			}
			f = f->e_right;
		} while (f != 0);
		break;
	case BREAK:
		return BRK;
	case CONTINUE:
		return CONT;
	case RETURN:
		result = eeval (f->e_right);
		return RET;
	}
	return 0;
}
