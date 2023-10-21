/*
 *	func.c
 *
 *	handle function definition
 */

# include	"ic.h"

static char *errs[] = {
# define NOTFUNC	0
	"Non-function used as function name",
};

definefunc (sym, args, autos, stat)
symbol	*sym;
expr	*args, *autos, *stat;
{
	int	offset, argc;
	expr	*a;
	symbol	*s, *tmp;

	if (sym->s_type != UNDEF && sym->s_type != FUNCTYPE) {
		eerror (errs[NOTFUNC]);
		return 0;
	}
	if (sym->s_expr) {
		freetree (sym->s_expr);
		freesyms (sym->s_local);
	}
	offset = 0;
	tmp = 0;
	argc = 0;
	for (a = args; a; a = a->e_right) {
		s = a->e_left->e_name;
		extractSym (s);
		s->s_next = tmp;
		tmp = s;
		s->s_type = STACKTYPE;
		s->s_offset = offset++;
		++argc;
	}
	sym->s_argc = argc;
	offset = 0;
	for (a = autos; a; a = a->e_right) {
		s = a->e_left->e_name;
		extractSym (s);
		s->s_next = tmp;
		tmp = s;
		s->s_type = STACKTYPE;
		s->s_offset = --offset;
	}
	sym->s_local = tmp;
	sym->s_expr = stat;
}

fixstack (e)
expr	*e;
{
	symbol	*s, *insertSym();
	char	*malloc (), *strcpy();

	while (e) {
		s = e->e_left->e_name;
		if (s->s_level == 0)
			e->e_left->e_name = insertSym (
				strcpy (malloc (strlen (s->s_name) + 1),
				s->s_name));
		e = e->e_right;
	}
}

freesyms (s)
symbol	*s;
{
	symbol	*t;

	while (s) {
		t = s->s_next;
		symFree (s);
		s = t;
	}
}
