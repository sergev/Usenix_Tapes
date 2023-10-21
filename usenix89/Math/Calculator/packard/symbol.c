/*
 *	symbol.c
 *
 *	deal with the symbol table
 */

# include "ic.h"

# define HASHSIZE	63
# define SYMALLOC	64

static struct symbol	*htable[HASHSIZE];

static int	level;

hash (name)
register char	*name;
{
	register value;

	value = 0;
	while (*name)
		value += *name++;
	return value % HASHSIZE;
}

symbol *
lookUp (name)
char	*name;
{
	register symbol	*sym;
	symbol	**queue;
	symbol	*insertSym();
	char	*malloc(), *strcpy();

	queue = & htable[hash(name)];
	for (sym = *queue; sym; sym = sym->s_next)
		if (!strcmp (sym->s_name, name))
			return sym;
	return insertSym (strcpy (malloc (strlen(name)+1), name));
}

symbol *
insertSym (name)
char	*name;
{
	symbol	**queue, *symAlloc(), *sym;

	queue = & htable[hash(name)];
	sym = symAlloc ();
	sym->s_name = name;
	sym->s_back = 0;
	sym->s_level = level;
	if (sym->s_next = *queue)
		(*queue)->s_back = sym;
	sym->s_value = 0;
	sym->s_type = UNDEF;
	*queue = sym;
	return sym;
}

extractSym (s)
symbol	*s;
{
	if (s->s_back)
		s->s_back->s_next = s->s_next;
	else
		htable[hash(s->s_name)] = s->s_next;
	if (s->s_next)
		s->s_next->s_back = s->s_back;
}

pushlevel()
{
	++level;
}

poplevel()
{
	--level;
}

static struct symbol	initblock[SYMALLOC];
static int	initused = 0;
static struct symbol	*freelist;

symbol *
symAlloc ()
{
	char		*malloc ();
	register symbol	*s;
	
	if (!freelist) {
		if (!initused)
			s = initblock;
		else
			s = (symbol *) malloc (sizeof (symbol) * SYMALLOC);
		freelist = s;
		while (s != freelist + SYMALLOC - 1) {
			s->s_next = s+1;
			++s;
		}
		s->s_next = (symbol *) 0;
	}
	s = freelist;
	freelist = s->s_next;
	s->s_next = 0;
	return s;
}

symFree (s)
symbol	*s;
{
	s->s_next = freelist;
	freelist = s;
}
