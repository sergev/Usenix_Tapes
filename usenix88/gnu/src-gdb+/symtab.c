/* Symbol table lookup for the GNU debugger, GDB.
   Copyright (C) 1986, 1987, 1988 Free Software Foundation, Inc.

GDB is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the GDB General Public License for full details.

Everyone is granted permission to copy, modify and redistribute GDB,
but only under the conditions described in the GDB General Public
License.  A copy of this license is supposed to have been given to you
along with GDB so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

In other words, go ahead and share GDB, but don't try to stop
anyone else from sharing it farther.  Help stamp out software hoarding!
*/

#include "defs.h"
#include "initialize.h"
#include "symtab.h"
#include "param.h"

#include <stdio.h>
#include <obstack.h>

START_FILE

/* Allocate an obstack to hold objects that should be freed
   when we load a new symbol table.
   This includes the symbols made by dbxread
   and the types that are not permanent.  */

struct obstack obstack1;

struct obstack *symbol_obstack = &obstack1;

/* These variables point to the objects
   representing the predefined C data types.  */

struct type *builtin_type_void;
struct type *builtin_type_char;
struct type *builtin_type_short;
struct type *builtin_type_int;
struct type *builtin_type_long;
struct type *builtin_type_unsigned_char;
struct type *builtin_type_unsigned_short;
struct type *builtin_type_unsigned_int;
struct type *builtin_type_unsigned_long;
struct type *builtin_type_float;
struct type *builtin_type_double;

/* Lookup the symbol table of a source file named NAME.  */

struct symtab *
lookup_symtab (name)
     char *name;
{
  register struct symtab *s;
  register char *copy;

  for (s = symtab_list; s; s = s->next)
    if (!strcmp (name, s->filename))
      return s;

  /* If name not found as specified, see if adding ".c" helps.  */

  copy = (char *) alloca (strlen (name) + 3);
  strcpy (copy, name);
  strcat (copy, ".c");
  for (s = symtab_list; s; s = s->next)
    if (!strcmp (copy, s->filename))
      return s;

  return 0;
}

/* Lookup a typedef or primitive type named NAME,
   visible in lexical block BLOCK.
   If NOERR is nonzero, return zero if NAME is not suitably defined.  */

struct type *
lookup_typename (name, block, noerr)
     char *name;
     struct block *block;
     int noerr;
{
  register struct symbol *sym = lookup_symbol (name, block, VAR_NAMESPACE);
  if (sym == 0 || SYMBOL_CLASS (sym) != LOC_TYPEDEF)
    {
      if (!strcmp (name, "int"))
	return builtin_type_int;
      if (!strcmp (name, "long"))
	return builtin_type_long;
      if (!strcmp (name, "short"))
	return builtin_type_short;
      if (!strcmp (name, "char"))
	return builtin_type_char;
      if (!strcmp (name, "float"))
	return builtin_type_float;
      if (!strcmp (name, "double"))
	return builtin_type_double;
      if (!strcmp (name, "void"))
	return builtin_type_void;

      if (noerr)
	return 0;
      error ("No type named %s.", name);
    }
  return SYMBOL_TYPE (sym);
}

struct type *
lookup_unsigned_typename (name)
     char *name;
{
  if (!strcmp (name, "int"))
    return builtin_type_unsigned_int;
  if (!strcmp (name, "long"))
    return builtin_type_unsigned_long;
  if (!strcmp (name, "short"))
    return builtin_type_unsigned_short;
  if (!strcmp (name, "char"))
    return builtin_type_unsigned_char;
  error ("No type named unsigned %s.", name);
}

/* Lookup a structure type named "struct NAME",
   visible in lexical block BLOCK.  */

struct type *
lookup_struct (name, block)
     char *name;
     struct block *block;
{
  register struct symbol *sym = lookup_symbol (name, block, STRUCT_NAMESPACE);
  if (sym == 0)
    error ("No struct type named %s.", name);
  if (TYPE_CODE (SYMBOL_TYPE (sym)) != TYPE_CODE_STRUCT)
    error ("This context has class, union or enum %s, not a struct.", name);
  return SYMBOL_TYPE (sym);
}

/* Lookup a union type named "union NAME",
   visible in lexical block BLOCK.  */

struct type *
lookup_union (name, block)
     char *name;
     struct block *block;
{
  register struct symbol *sym = lookup_symbol (name, block, STRUCT_NAMESPACE);
  if (sym == 0)
    error ("No union type named %s.", name);
  if (TYPE_CODE (SYMBOL_TYPE (sym)) != TYPE_CODE_UNION)
    error ("This context has class, struct or enum %s, not a union.", name);
  return SYMBOL_TYPE (sym);
}

/* Lookup an enum type named "enum NAME",
   visible in lexical block BLOCK.  */

struct type *
lookup_enum (name, block)
     char *name;
     struct block *block;
{
  register struct symbol *sym = lookup_symbol (name, block, STRUCT_NAMESPACE);
  if (sym == 0)
    error ("No enum type named %s.", name);
  if (TYPE_CODE (SYMBOL_TYPE (sym)) != TYPE_CODE_ENUM)
    error ("This context has class, struct or union %s, not an enum.", name);
  return SYMBOL_TYPE (sym);
}

/* Given a type TYPE, return a type of pointers to that type.
   May need to construct such a type if this is the first use.

   C++: use TYPE_MAIN_VARIANT and TYPE_CHAIN to keep pointer
   to member types under control.  */

struct type *
lookup_pointer_type (type)
     struct type *type;
{
  register struct type *ptype = TYPE_POINTER_TYPE (type);
  if (ptype) return TYPE_MAIN_VARIANT (ptype);

  /* This is the first time anyone wanted a pointer to a TYPE.  */
  if (TYPE_FLAGS (type) & TYPE_FLAG_PERM)
    ptype  = (struct type *) xmalloc (sizeof (struct type));
  else
    ptype  = (struct type *) obstack_alloc (symbol_obstack,
					    sizeof (struct type));

  bzero (ptype, sizeof (struct type));
  TYPE_MAIN_VARIANT (ptype) = ptype;
  TYPE_TARGET_TYPE (ptype) = type;
  TYPE_POINTER_TYPE (type) = ptype;
  /* New type is permanent if type pointed to is permanent.  */
  if (TYPE_FLAGS (type) & TYPE_FLAG_PERM)
    TYPE_FLAGS (ptype) |= TYPE_FLAG_PERM;
  /* We assume the machine has only one representation for pointers!  */
  TYPE_LENGTH (ptype) = sizeof (char *);
  TYPE_CODE (ptype) = TYPE_CODE_PTR;
  return ptype;
}

struct type *
lookup_reference_type (type)
     struct type *type;
{
  register struct type *rtype = TYPE_REFERENCE_TYPE (type);
  if (rtype) return TYPE_MAIN_VARIANT (rtype);

  /* This is the first time anyone wanted a pointer to a TYPE.  */
  if (TYPE_FLAGS (type) & TYPE_FLAG_PERM)
    rtype  = (struct type *) xmalloc (sizeof (struct type));
  else
    rtype  = (struct type *) obstack_alloc (symbol_obstack,
					    sizeof (struct type));

  bzero (rtype, sizeof (struct type));
  TYPE_MAIN_VARIANT (rtype) = rtype;
  TYPE_TARGET_TYPE (rtype) = type;
  TYPE_REFERENCE_TYPE (type) = rtype;
  /* New type is permanent if type pointed to is permanent.  */
  if (TYPE_FLAGS (type) & TYPE_FLAG_PERM)
    TYPE_FLAGS (rtype) |= TYPE_FLAG_PERM;
  /* We assume the machine has only one representation for pointers!  */
  TYPE_LENGTH (rtype) = sizeof (char *);
  TYPE_CODE (rtype) = TYPE_CODE_REF;
  return rtype;
}


/* Given a type TYPE, return a type of pointers to that type.
   May need to construct such a type if this is the first use.
   The TYPE is the type of the member.  The DOMAIN is the type
   of the aggregate that the member belongs to.  */

struct type *
lookup_member_pointer_type (type, domain)
     struct type *type, *domain;
{
  register struct type *ptype = TYPE_POINTER_TYPE (type);
  struct type *main_type;

  if (ptype)
    {
      ptype = TYPE_MAIN_VARIANT (ptype);
      main_type = ptype;
      while (ptype)
	{
	  if (TYPE_DOMAIN_TYPE (ptype) == domain)
	    return ptype;
	  ptype = TYPE_CHAIN (ptype);
	}
    }
  else
    {
      main_type = lookup_pointer_type (type);
      TYPE_POINTER_TYPE (type) = main_type;
    }

  /* This is the first time anyone wanted a pointer to a TYPE.  */
  if (TYPE_FLAGS (type) & TYPE_FLAG_PERM)
    ptype  = (struct type *) xmalloc (sizeof (struct type));
  else
    ptype  = (struct type *) obstack_alloc (symbol_obstack,
					    sizeof (struct type));

  bzero (ptype, sizeof (struct type));
  TYPE_MAIN_VARIANT (ptype) = main_type;
  TYPE_TARGET_TYPE (ptype) = type;
  TYPE_DOMAIN_TYPE (ptype) = domain;
  TYPE_POINTER_TYPE (type) = ptype;
  /* New type is permanent if type pointed to is permanent.  */
  if (TYPE_FLAGS (type) & TYPE_FLAG_PERM)
    TYPE_FLAGS (ptype) |= TYPE_FLAG_PERM;
  /* We assume the machine has only one representation for pointers!  */
  TYPE_LENGTH (ptype) = sizeof (char *);
  TYPE_CODE (ptype) = TYPE_CODE_MPTR;

  /* Now splice in the new member pointer type.  */
  if (main_type)
    {
      /* This type was not "smashed".  */
      TYPE_CHAIN (ptype) = TYPE_CHAIN (main_type);
      TYPE_CHAIN (main_type) = ptype;
    }

  return ptype;
}

/* Given a type TYPE, return a type of functions that return that type.
   May need to construct such a type if this is the first use.  */

struct type *
lookup_function_type (type, argtypes)
     struct type *type;
     struct type **argtypes;
{
  register struct type *ptype = TYPE_FUNCTION_TYPE (type);
  if (ptype) return ptype;

  /* This is the first time anyone wanted a function returning a TYPE.  */
  if (TYPE_FLAGS (type) & TYPE_FLAG_PERM)
    ptype  = (struct type *) xmalloc (sizeof (struct type));
  else
    ptype  = (struct type *) obstack_alloc (symbol_obstack,
					    sizeof (struct type));

  bzero (ptype, sizeof (struct type));
  TYPE_TARGET_TYPE (ptype) = type;
  TYPE_FUNCTION_TYPE (type) = ptype;
  /* New type is permanent if type returned is permanent.  */
  if (TYPE_FLAGS (type) & TYPE_FLAG_PERM)
    TYPE_FLAGS (ptype) |= TYPE_FLAG_PERM;
  TYPE_LENGTH (ptype) = 1;
  TYPE_CODE (ptype) = TYPE_CODE_FUNC;
  TYPE_NFIELDS (ptype) = 0;
  return ptype;
}

/* Smash TYPE to be a type of pointers to TO_TYPE.
   If TO_TYPE is not permanent and has no pointer-type yet,
   record TYPE as its pointer-type.  */

void
smash_to_pointer_type (type, to_type)
     struct type *type, *to_type;
{
  bzero (type, sizeof (struct type));
  TYPE_TARGET_TYPE (type) = to_type;
  /* We assume the machine has only one representation for pointers!  */
  TYPE_LENGTH (type) = sizeof (char *);
  TYPE_CODE (type) = TYPE_CODE_PTR;

  TYPE_MAIN_VARIANT (type) = type;

  if (TYPE_POINTER_TYPE (to_type) == 0
      && !(TYPE_FLAGS (type) & TYPE_FLAG_PERM))
    {
      TYPE_POINTER_TYPE (to_type) = type;
    }
}

/* Smash TYPE to be a type of pointers to TO_TYPE.
   If TO_TYPE is not permanent and has no pointer-type yet,
   record TYPE as its pointer-type.

   TYPE is the type of the member.  DOMAIN is the type of
   the aggregate that the member belongs to.  */

void
smash_to_member_pointer_type (type, domain, to_type)
     struct type *type, *domain, *to_type;
{
  bzero (type, sizeof (struct type));
  TYPE_TARGET_TYPE (type) = to_type;
  TYPE_DOMAIN_TYPE (type) = domain;
  /* We assume the machine has only one representation for pointers!  */
  TYPE_LENGTH (type) = sizeof (char *);
  TYPE_CODE (type) = TYPE_CODE_MPTR;

  TYPE_MAIN_VARIANT (type) = lookup_pointer_type (to_type);

  if (TYPE_POINTER_TYPE (to_type) == 0
      && !(TYPE_FLAGS (type) & TYPE_FLAG_PERM))
    {
      TYPE_POINTER_TYPE (to_type) = type;
    }
}

/* Smash TYPE to be a type of reference to TO_TYPE.
   If TO_TYPE is not permanent and has no pointer-type yet,
   record TYPE as its pointer-type.  */

void
smash_to_reference_type (type, to_type)
     struct type *type, *to_type;
{
  bzero (type, sizeof (struct type));
  TYPE_TARGET_TYPE (type) = to_type;
  /* We assume the machine has only one representation for pointers!  */
  TYPE_LENGTH (type) = sizeof (char *);
  TYPE_CODE (type) = TYPE_CODE_REF;

  TYPE_MAIN_VARIANT (type) = type;

  if (TYPE_REFERENCE_TYPE (to_type) == 0
      && !(TYPE_FLAGS (type) & TYPE_FLAG_PERM))
    {
      TYPE_REFERENCE_TYPE (to_type) = type;
    }
}

/* Smash TYPE to be a type of functions returning TO_TYPE.
   If TO_TYPE is not permanent and has no function-type yet,
   record TYPE as its function-type.  */

void
smash_to_function_type (type, to_type)
     struct type *type, *to_type;
{
  bzero (type, sizeof (struct type));
  TYPE_TARGET_TYPE (type) = to_type;
  TYPE_LENGTH (type) = 1;
  TYPE_CODE (type) = TYPE_CODE_FUNC;
  TYPE_NFIELDS (type) = 0;

  if (TYPE_FUNCTION_TYPE (to_type) == 0
      && !(TYPE_FLAGS (type) & TYPE_FLAG_PERM))
    {
      TYPE_FUNCTION_TYPE (to_type) = type;
    }
}

static struct symbol *lookup_block_symbol ();

/* Find the definition for a specified symbol name NAME
   in namespace NAMESPACE, visible from lexical block BLOCK.
   Returns the struct symbol pointer, or zero if no symbol is found.  */

struct symbol *
lookup_symbol_1 (name, block, namespace)
     char *name;
     register struct block *block;
     enum namespace namespace;
{
  register int i, n;
  register struct symbol *sym;
  register struct symtab *s;
  struct blockvector *bv;

  /* Search specified block and its superiors.  */

  while (block != 0)
    {
      sym = lookup_block_symbol (block, name, namespace);
      if (sym) return sym;
      block = BLOCK_SUPERBLOCK (block);
    }
  return 0;
}

struct symbol *
lookup_symbol_2 (name, block, namespace)
     char *name;
     register struct block *block; /* ignored as parameter */
     enum namespace namespace;
{
  register int i, n;
  register struct symbol *sym;
  register struct symtab *s;
  struct blockvector *bv;

  /* Now search all symtabs' global blocks.  */

  for (s = symtab_list; s; s = s->next)
    {
      bv = BLOCKVECTOR (s);
      block = BLOCKVECTOR_BLOCK (bv, 0);
      sym = lookup_block_symbol (block, name, namespace);
      if (sym) return sym;
    }

  /* Now search all symtabs' per-file blocks.
     Not strictly correct, but more useful than an error.  */

  for (s = symtab_list; s; s = s->next)
    {
      bv = BLOCKVECTOR (s);
      block = BLOCKVECTOR_BLOCK (bv, 1);
      sym = lookup_block_symbol (block, name, namespace);
      if (sym) return sym;
    }
  return 0;
}

struct symbol *
lookup_symbol (name, block, namespace)
     char *name;
     register struct block *block;
     enum namespace namespace;
{
  register int i, n;
  register struct symbol *sym;
  register struct symtab *s;
  struct blockvector *bv;

  /* Search specified block and its superiors.  */

  while (block != 0)
    {
      sym = lookup_block_symbol (block, name, namespace);
      if (sym) return sym;
      block = BLOCK_SUPERBLOCK (block);
    }

  /* Now search all symtabs' global blocks.  */

  for (s = symtab_list; s; s = s->next)
    {
      bv = BLOCKVECTOR (s);
      block = BLOCKVECTOR_BLOCK (bv, 0);
      sym = lookup_block_symbol (block, name, namespace);
      if (sym) return sym;
    }

  /* Now search all symtabs' per-file blocks.
     Not strictly correct, but more useful than an error.  */

  for (s = symtab_list; s; s = s->next)
    {
      bv = BLOCKVECTOR (s);
      block = BLOCKVECTOR_BLOCK (bv, 1);
      sym = lookup_block_symbol (block, name, namespace);
      if (sym) return sym;
    }
  return 0;
}

/* Look for a symbol in block BLOCK using binary search.  */

static struct symbol *
lookup_block_symbol (block, name, namespace)
     register struct block *block;
     char *name;
     enum namespace namespace;
{
  register int bot, top, inc;
  register struct symbol *sym;

  top = BLOCK_NSYMS (block);
  bot = 0;

  /* First, advance BOT to not far before
     the first symbol whose name is NAME.  */

  while (1)
    {
      inc = (top - bot + 1);
      /* No need to keep binary searching for the last few bits worth.  */
      if (inc < 7)
	break;
      inc >>= 1;
      sym = BLOCK_SYM (block, bot + inc);
      if (strcmp (SYMBOL_NAME (sym), name) < 0)
	bot += inc;
      else
	top = bot + inc;
    }

  /* Now scan forward until we run out of symbols,
     find one whose name is greater than NAME,
     or find one we want.
     If there is more than one symbol with the right name and namespace,
     we return the first one.  dbxread.c is careful to make sure
     that if one is a register then it comes first.  */

  top = BLOCK_NSYMS (block);
  while (bot < top)
    {
      sym = BLOCK_SYM (block, bot);
      inc = strcmp (SYMBOL_NAME (sym), name);
      if (inc == 0 && SYMBOL_NAMESPACE (sym) == namespace)
	return sym;
      if (inc > 0)
	return 0;
      bot++;
    }
  return 0;
}

/* Return the symbol for the function which contains a specified
   lexical block, described by a struct block BL.  */

struct symbol *
block_function (bl)
     struct block *bl;
{
  while (BLOCK_FUNCTION (bl) == 0 && BLOCK_SUPERBLOCK (bl) != 0)
    bl = BLOCK_SUPERBLOCK (bl);

  return BLOCK_FUNCTION (bl);
}

/* Subroutine of find_pc_line */

static struct symtab *
find_pc_symtab (pc)
     register CORE_ADDR pc;
{
  register struct block *b;
  struct blockvector *bv;
  register struct symtab *s;

  /* Search all symtabs for one whose file contains our pc */

  for (s = symtab_list; s; s = s->next)
    {
      bv = BLOCKVECTOR (s);
      b = BLOCKVECTOR_BLOCK (bv, 0);
      if (BLOCK_START (b) <= pc
	  && BLOCK_END (b) > pc)
	break;
    }

  return s;
}

/* Find the source file and line number for a given PC value.
   Return a structure containing a symtab pointer, a line number,
   and a pc range for the entire source line.
   The value's .pc field is NOT the specified pc.
   NOTCURRENT nonzero means, if specified pc is on a line boundary,
   use the line that ends there.  Otherwise, in that case, the line
   that begins there is used.  */

struct symtab_and_line
find_pc_line (pc, notcurrent)
     CORE_ADDR pc;
     int notcurrent;
{
  struct symtab *s;
  register struct linetable *l;
  register int len;
  register int i, item;
  int line;
  struct symtab_and_line value;
  struct blockvector *bv;

  /* Info on best line seen so far, and where it starts, and its file.  */

  int best_line = 0;
  CORE_ADDR best_pc = 0;
  CORE_ADDR best_end = 0;
  struct symtab *best_symtab = 0;

  /* Store here the first line number
     of a file which contains the line at the smallest pc after PC.
     If we don't find a line whose range contains PC,
     we will use a line one less than this,
     with a range from the start of that file to the first line's pc.  */
  int alt_line = 0;
  CORE_ADDR alt_pc = 0;
  struct symtab *alt_symtab = 0;

  /* Info on best line seen in this file.  */

  int prev_line;
  CORE_ADDR prev_pc;

  /* Info on first line of this file.  */

  int first_line;
  CORE_ADDR first_pc;

  /* If this pc is not from the current frame,
     it is the address of the end of a call instruction.
     Quite likely that is the start of the following statement.
     But what we want is the statement containing the instruction.
     Fudge the pc to make sure we get that.  */

  if (notcurrent) pc -= 1;

  s = find_pc_symtab (pc);
  if (s == 0)
    {
      value.symtab = 0;
      value.line = 0;
      value.pc = pc;
      return value;
    }

  bv = BLOCKVECTOR (s);

  /* Look at all the symtabs that share this blockvector.
     They all have the same apriori range, that we found was right;
     but they have different line tables.  */

  for (; s && BLOCKVECTOR (s) == bv; s = s->next)
    {
      /* Find the best line in this symtab.  */
      l = LINETABLE (s);
      len = l->nitems;
      prev_line = -1;
      first_line = -1;
      for (i = 0; i < len; i++)
	{
	  item = l->item[i];
	  if (item < 0)
	    line = - item - 1;
	  else
	    {
	      line++;
	      if (first_line < 0)
		{
		  first_line = line;
		  first_pc = item;
		}
	      /* Return the last line that did not start after PC.  */
	      if (pc >= item)
		{
		  prev_line = line;
		  prev_pc = item;
		}
	      else
		break;
	    }
	}

      /* Is this file's best line closer than the best in the other files?
	 If so, record this file, and its best line, as best so far.  */
      if (prev_line >= 0 && prev_pc > best_pc)
	{
	  best_pc = prev_pc;
	  best_line = prev_line;
	  best_symtab = s;
	  if (i < len)
	    best_end = item;
	  else
	    best_end = 0;
	}
      /* Is this file's first line closer than the first lines of other files?
	 If so, record this file, and its first line, as best alternate.  */
      if (first_line >= 0 && first_pc > pc
	  && (alt_pc == 0 || first_pc < alt_pc))
	{
	  alt_pc = first_pc;
	  alt_line = first_line;
	  alt_symtab = s;
	}
    }
  if (best_symtab == 0)
    {
      value.symtab = alt_symtab;
      value.line = alt_line - 1;
      value.pc = BLOCK_END (BLOCKVECTOR_BLOCK (bv, 0));
      value.end = alt_pc;
    }
  else
    {
      value.symtab = best_symtab;
      value.line = best_line;
      value.pc = best_pc;
      value.end = (best_end ? best_end
		   : (alt_pc ? alt_pc
		      : BLOCK_END (BLOCKVECTOR_BLOCK (bv, 0))));
    }
  return value;
}

/* Find the PC value for a given source file and line number.
   Returns zero for invalid line number.
   The source file is specified with a struct symtab.  */

CORE_ADDR
find_line_pc (symtab, line)
     struct symtab *symtab;
     int line;
{
  register struct linetable *l;
  register int index;
  int dummy;

  if (symtab == 0)
    return 0;
  l = LINETABLE (symtab);
  index = find_line_common(l, line, &dummy);
  return index ? l->item[index] : 0;
}

/* Find the range of pc values in a line.
   Store the starting pc of the line into *STARTPTR
   and the ending pc (start of next line) into *ENDPTR.
   Returns 1 to indicate success.
   Returns 0 if could not find the specified line.  */

int
find_line_pc_range (symtab, thisline, startptr, endptr)
     struct symtab *symtab;
     int thisline;
     CORE_ADDR *startptr, *endptr;
{
  register struct linetable *l;
  register int index;
  int exact_match;		/* did we get an exact linenumber match */
  register CORE_ADDR prev_pc;
  CORE_ADDR last_pc;

  if (symtab == 0)
    return 0;

  l = LINETABLE (symtab);
  index = find_line_common (l, thisline, &exact_match);
  if (index)
    {
      *startptr = l->item[index];
      /* If we have not seen an entry for the specified line,
	 assume that means the specified line has zero bytes.  */
      if (!exact_match || index == l->nitems-1)
	*endptr = *startptr;
      else
	/* Perhaps the following entry is for the following line.
	   It's worth a try.  */
	if (l->item[index+1] > 0)
	  *endptr = l->item[index+1];
	else
	  *endptr = find_line_pc (symtab, thisline+1);
      return 1;
    }

  return 0;
}

/* Given a line table and a line number, return the index into the line
   table for the pc of the nearest line whose number is >= the specified one.
   Return 0 if none is found.  The value is never zero is it is an index.

   Set *EXACT_MATCH nonzero if the value returned is an exact match.  */

static int
find_line_common (l, lineno, exact_match)
     register struct linetable *l;
     register int lineno;
     int *exact_match;
{
  register int i;
  register int len;

  /* BEST is the smallest linenumber > LINENO so far seen,
     or 0 if none has been seen so far.
     BEST_INDEX identifies the item for it.  */

  int best_index = 0;
  int best = 0;

  int nextline = -1;

  if (lineno <= 0)
    return 0;

  len = l->nitems;
  for (i = 0; i < len; i++)
    {
      register int item = l->item[i];

      if (item < 0)
	nextline = - item - 1;
      else
	{
	  nextline++;
	  if (nextline == lineno)
	    {
	      *exact_match = 1;
	      return i;
	    }

	  if (nextline > lineno && (best == 0 || nextline < best))
	    {
	      best = lineno;
	      best_index = i;
	    }
	}
    }

  /* If we got here, we didn't get an exact match.  */

  *exact_match = 0;
  return best_index;
}

int
find_pc_line_pc_range (pc, startptr, endptr)
     CORE_ADDR pc;
     CORE_ADDR *startptr, *endptr;
{
  struct symtab_and_line sal;
  sal = find_pc_line (pc, 0);
  *startptr = sal.pc;
  *endptr = sal.end;
  return sal.symtab != 0;
}

/* Parse a string that specifies a line number.
   Pass the address of a char * variable; that variable will be
   advanced over the characters actually parsed.

   The string can be:

   LINENUM -- that line number in current file.  PC returned is 0.
   FILE:LINENUM -- that line in that file.  PC returned is 0.
   FUNCTION -- line number of openbrace of that function.
      PC returned is the start of the function.
   FILE:FUNCTION -- likewise, but prefer functions in that file.
   *EXPR -- line in which address EXPR appears.

   FUNCTION may be an undebuggable function found in misc_function_vector.

   If the argument FUNFIRSTLINE is nonzero, we want the first line
   of real code inside a function when a function is specified.

   DEFAULT_SYMTAB specifies the file to use if none is specified.
   It defaults to current_source_symtab.
   DEFAULT_LINE specifies the line number to use for relative
   line numbers (that start with signs).  Defaults to current_source_line.

   Note that it is possible to return zero for the symtab
   if no file is validly specified.  Callers must check that.
   Also, the line number returned may be invalid.  */

struct symtabs_and_lines
decode_line_1 (argptr, funfirstline, default_symtab, default_line)
     char **argptr;
     int funfirstline;
     struct symtab *default_symtab;
     int default_line;
{
  struct symtabs_and_lines decode_line_2 ();
  struct symtabs_and_lines values;
  struct symtab_and_line value;
  register char *p, *p1;
  register struct symtab *s;
  register struct symbol *sym;
  register CORE_ADDR pc;
  register int i;
  char *copy;
  struct symbol *sym_class;
  char *class_name, *method_name, *phys_name;
  int method_counter;
  int i1;
  struct symbol **sym_arr;
  struct type *t, *field;
  char **physnames;
  
  /* Defaults have defaults.  */

  if (default_symtab == 0)
    {
      default_symtab = current_source_symtab;
      default_line = current_source_line;
    }

  /* See if arg is *PC */

  if (**argptr == '*')
    {
      (*argptr)++;
      pc = parse_and_eval_address_1 (argptr);
      values.sals = (struct symtab_and_line *)malloc (sizeof (struct symtab_and_line));
      values.nelts = 1;
      values.sals[0] = find_pc_line (pc, 0);
      values.sals[0].pc = pc;
      return values;
    }

  /* Maybe arg is FILE : LINENUM or FILE : FUNCTION */

  s = 0;

  for (p = *argptr; *p; p++)
    {
      if (p[0] == ':' || p[0] == ' ' || p[0] == '\t')
	break;
    }
  while (p[0] == ' ' || p[0] == '\t') p++;

  if (p[0] == ':')
    {

      /*  C++  */
      if (p[1] ==':')
	{
	  /* Extract the class name.  */
	  p1 = p;
	  while (p != *argptr && p[-1] == ' ') --p;
	  copy = (char *) alloca (p - *argptr + 1);
	  bcopy (*argptr, copy, p - *argptr);
	  copy[p - *argptr] = 0;

	  /* Discard the class name from the arg.  */
	  p = p1 + 2;
	  while (*p == ' ' || *p == '\t') p++;
	  *argptr = p;

	  sym_class = lookup_symbol (copy, 0, STRUCT_NAMESPACE);
       
	  if (sym_class &&
	      (TYPE_CODE (SYMBOL_TYPE (sym_class)) == TYPE_CODE_STRUCT
	       || TYPE_CODE (SYMBOL_TYPE (sym_class)) == TYPE_CODE_UNION))
	    {
	      /* Arg token is not digits => try it as a function name
		 Find the next token (everything up to end or next whitespace). */
	      p = *argptr;
	      while (*p && *p != ' ' && *p != '\t' && *p != ',' && *p !=':') p++;
	      copy = (char *) alloca (p - *argptr + 1);
	      bcopy (*argptr, copy, p - *argptr);
	      copy[p - *argptr] = '\0';

	      /* no line number may be specified */
	      while (*p == ' ' || *p == '\t') p++;
	      *argptr = p;

	      sym = 0;
	      i1 = 0;		/*  counter for the symbol array */
	      t = SYMBOL_TYPE (sym_class);
	      sym_arr = (struct symbol **) alloca(TYPE_NFN_FIELDS_TOTAL (t) * sizeof(struct symbol*));
	      physnames = (char **) alloca (TYPE_NFN_FIELDS_TOTAL (t) * sizeof(char*));

	      if (destructor_name_p (copy, t))
		{
		  /* destructors are a special case.  */
		  struct fn_field *f = TYPE_FN_FIELDLIST1 (t, 0);
		  int len = TYPE_FN_FIELDLIST_LENGTH (t, 0) - 1;
		  phys_name = TYPE_FN_FIELD_PHYSNAME (f, len);
		  physnames[i1] = (char *)alloca (strlen (phys_name) + 1);
		  strcpy (physnames[i1], phys_name);
		  sym_arr[i1] = lookup_symbol (phys_name, SYMBOL_BLOCK_VALUE (sym_class), VAR_NAMESPACE);
		  if (sym_arr[i1]) i1++;
		}
	      else while (t)
		{
		  int constructor_p;

		  class_name = TYPE_NAME (t);
		  while (*class_name++ != ' ');

		  constructor_p = ! strcmp (class_name, copy);

		  sym_class = lookup_symbol (class_name, 0, STRUCT_NAMESPACE);
		  for (method_counter = TYPE_NFN_FIELDS (SYMBOL_TYPE (sym_class)) - 1;
		       method_counter >= 0;
		       --method_counter)
		    {
		      int field_counter;
		      struct fn_field *f =
			TYPE_FN_FIELDLIST1 (SYMBOL_TYPE (sym_class), method_counter);

		      method_name = TYPE_FN_FIELDLIST_NAME (SYMBOL_TYPE (sym_class), method_counter);
		      if (!strcmp (copy, method_name))
			for (field_counter = TYPE_FN_FIELDLIST_LENGTH (SYMBOL_TYPE (sym_class), method_counter) - (1 + constructor_p);
			     field_counter >= 0;
			     --field_counter)
			  {
			    phys_name = TYPE_FN_FIELD_PHYSNAME (f, field_counter);
			    physnames[i1] = (char*) alloca (strlen (phys_name) + 1);
			    strcpy (physnames[i1], phys_name);
			    sym_arr[i1] = lookup_symbol (phys_name, SYMBOL_BLOCK_VALUE (sym_class), VAR_NAMESPACE);
			    if (sym_arr[i1]) i1++;
			  }
		    }
		  t = TYPE_BASECLASS(t);
		}

	      if (i1 == 1)
		{
		  sym = sym_arr[0];

		  if (sym && SYMBOL_CLASS (sym) == LOC_BLOCK)
		    {
		      /* Arg is the name of a function */
		      pc = BLOCK_START (SYMBOL_BLOCK_VALUE (sym)) + FUNCTION_START_OFFSET;
		      if (funfirstline)
			SKIP_PROLOGUE (pc);
		      values.sals = (struct symtab_and_line *)malloc (sizeof (struct symtab_and_line));
		      values.nelts = 1;
		      values.sals[0] = find_pc_line (pc, 0);
		      values.sals[0].pc = (values.sals[0].end && values.sals[0].pc != pc) ? values.sals[0].end : pc;
		    }
		  else
		    {
		      values.nelts = 0;
		    }
		  return values;
		}
	      if (i1 > 0)
		{
		  return decode_line_2 (argptr, sym_arr, physnames, i1, funfirstline);
		}
	      else
		error ("that class does not have any method named %s",copy);
	    }
	  else
	    error("no class, struct, or union named %s", copy );
	}
      /*  end of C++  */


      /* Extract the file name.  */
      p1 = p;
      while (p != *argptr && p[-1] == ' ') --p;
      copy = (char *) alloca (p - *argptr + 1);
      bcopy (*argptr, copy, p - *argptr);
      copy[p - *argptr] = 0;

      /* Find that file's data.  */
      s = lookup_symtab (copy);
      if (s == 0)
	{
	  if (symtab_list == 0)
	    error ("No symbol table is loaded.  Use the \"symbol-file\" command.");
	  error ("No source file named %s.", copy);
	}

      /* Discard the file name from the arg.  */
      p = p1 + 1;
      while (*p == ' ' || *p == '\t') p++;
      *argptr = p;
    }

  /* S is specified file's symtab, or 0 if no file specified.
     arg no longer contains the file name.  */

  /* Check whether arg is all digits (and sign) */

  p = *argptr;
  if (*p == '-' || *p == '+') p++;
  while (*p >= '0' && *p <= '9')
    p++;

  if (p != *argptr && (*p == 0 || *p == ' ' || *p == '\t' || *p == ','))
    {
      /* We found a token consisting of all digits -- at least one digit.  */
      enum sign {none, plus, minus} sign = none;

      if (**argptr == '+')
	sign = plus, (*argptr)++;
      else if (**argptr == '-')
	sign = minus, (*argptr)++;
      value.line = atoi (*argptr);
      switch (sign)
	{
	case plus:
	  if (p == *argptr)
	    value.line = 5;
	  if (s == 0)
	    value.line = default_line + value.line;
	  break;
	case minus:
	  if (p == *argptr)
	    value.line = 15;
	  if (s == 0)
	    value.line = default_line - value.line;
	  else
	    value.line = 1;
	  break;
	}

      while (*p == ' ' || *p == '\t') p++;
      *argptr = p;
      if (s == 0)
	s = default_symtab;
      value.symtab = s;
      value.pc = 0;
      values.sals = (struct symtab_and_line *)malloc (sizeof (struct symtab_and_line));
      values.sals[0] = value;
      values.nelts = 1;
      return values;
    }

  /* Arg token is not digits => try it as a function name
     Find the next token (everything up to end or next whitespace).  */
  p = *argptr;
  while (*p && *p != ' ' && *p != '\t' && *p != ',') p++;
  copy = (char *) alloca (p - *argptr + 1);
  bcopy (*argptr, copy, p - *argptr);
  copy[p - *argptr] = 0;
  while (*p == ' ' || *p == '\t') p++;
  *argptr = p;

  /* Look up that token as a function.
     If file specified, use that file's per-file block to start with.  */

  sym = lookup_symbol (copy, s ? BLOCKVECTOR_BLOCK (BLOCKVECTOR (s), 1) : 0,
		       VAR_NAMESPACE);

  if (sym && SYMBOL_CLASS (sym) == LOC_BLOCK)
    {
      /* Arg is the name of a function */
      pc = BLOCK_START (SYMBOL_BLOCK_VALUE (sym)) + FUNCTION_START_OFFSET;
      if (funfirstline)
	SKIP_PROLOGUE (pc);
      value = find_pc_line (pc, 0);
      value.pc = (value.end && value.pc != pc) ? value.end : pc;
      values.sals = (struct symtab_and_line *)malloc (sizeof (struct symtab_and_line));
      values.sals[0] = value;
      values.nelts = 1;
      return values;
    }

  if (sym)
    error ("%s is not a function.", copy);

  if ((i = lookup_misc_func (copy)) < 0)
    error ("Function %s not defined.", copy);
  else
    {
      value.symtab = 0;
      value.line = 0;
      value.pc = misc_function_vector[i].address + FUNCTION_START_OFFSET;
      if (funfirstline)
	SKIP_PROLOGUE (value.pc);
      values.sals = (struct symtab_and_line *)malloc (sizeof (struct symtab_and_line));
      values.sals[0] = value;
      values.nelts = 1;
      return values;
    }

  if (symtab_list == 0)
    error ("No symbol table is loaded.  Use the \"symbol-file\" command.");
  error ("Function %s not defined.", copy);
}

struct symtabs_and_lines
decode_line_spec (string, funfirstline)
     char *string;
     int funfirstline;
{
  struct symtabs_and_lines sals;
  if (string == 0)
    error ("Empty line specification.");
  sals = decode_line_1 (&string, funfirstline,
			current_source_symtab, current_source_line);
  if (*string)
    error ("Junk at end of line specification: %s", string);
  return sals;
}

struct symtabs_and_lines
decode_line_2 (argptr, sym_arr, physnames, nelts, funfirstline)
     char **argptr;
     struct symbol *sym_arr[];
     char *physnames[];
     int nelts;
     int funfirstline;
{
  char *getenv();
  struct symtabs_and_lines values, return_values;
  register CORE_ADDR pc;
  char *args, *arg1, *read_line ();
  int i;
  char *prompt;

  values.sals = (struct symtab_and_line *) alloca (nelts * sizeof(struct symtab_and_line));
  return_values.sals = (struct symtab_and_line *) malloc (nelts * sizeof(struct symtab_and_line));

  i = 0;
  printf("[0] cancel\n[1] all\n");
  while (i < nelts)
    {
      if (sym_arr[i] && SYMBOL_CLASS (sym_arr[i]) == LOC_BLOCK)
	{
	  /* Arg is the name of a function */
	  pc = BLOCK_START (SYMBOL_BLOCK_VALUE (sym_arr[i])) 
	       + FUNCTION_START_OFFSET;
	  if (funfirstline)
	    SKIP_PROLOGUE (pc);
	  values.sals[i] = find_pc_line (pc, 0);
	  printf("[%d] file:%s; line number:%d\n",
		 (i+2), values.sals[i].symtab->filename, values.sals[i].line);
	}
      else printf ("?HERE\n");
      i++;
    }
  
  if ((prompt = getenv ("PS2")) == NULL)
    {
      prompt = ">";
    }
  printf("%s ",prompt);
  fflush(stdout);

  args = read_line (0);
  
  if (args == 0)
    error_no_arg ("one or more choice numbers");

  i = 0;
  while (*args)
    {
      int num;

      arg1 = args;
      while (*arg1 >= '0' && *arg1 <= '9') arg1++;
      if (*arg1 && *arg1 != ' ' && *arg1 != '\t')
	error ("Arguments must be choice numbers.");

      num = atoi (args);

      if (num == 0)
	error ("cancelled");
      else if (num == 1)
	{
	  bcopy (values.sals, return_values.sals, (nelts * sizeof(struct symtab_and_line)));
	  return_values.nelts = nelts;
	  return return_values;
	}

      if (num > nelts + 2)
	{
	  printf ("No choice number %d.\n", num);
	}
      else
	{
	  num -= 2;
	  if (values.sals[num].pc)
	    {
	      return_values.sals[i++] = values.sals[num];
	      values.sals[num].pc = 0;
	    }
	  else
	    {
	      printf ("duplicate request for %d ignored.\n", num);
	    }
	}

      args = arg1;
      while (*args == ' ' || *args == '\t') args++;
    }
  return_values.nelts = i;
  return return_values;
}

/* Return the index of misc function named NAME.  */

static
lookup_misc_func (name)
     register char *name;
{
  register int i;

  for (i = 0; i < misc_function_count; i++)
    if (!strcmp (misc_function_vector[i].name, name))
      return i;
  return -1;		/* not found */
}

static void
sources_info ()
{
  register struct symtab *s;
  register int column = 0;

  if (symtab_list == 0)
    {
      printf ("No symbol table is loaded.\n");
      return;
    }
  printf ("Source files for which symbol table is known:\n");
  for (s = symtab_list; s; s = s->next)
    {
      if (column != 0 && column + strlen (s->filename) >= 70)
	{
	  printf ("\n");
	  column = 0;
	}
      else if (column != 0)
	{
	  printf (" ");
	  column++;
	}
      printf ("%s", s->filename);
      column += strlen (s->filename);
      if (s->next)
	{
	  printf (",");
	  column++;
	}
    }
  printf ("\n");
}

/* List all symbols (if REGEXP is 0) or all symbols matching REGEXP.
   If CLASS is zero, list all symbols except functions and type names.
   If CLASS is 1, list only functions.
   If CLASS is 2, list only type names.  */

#define MORE  \
{ print_count++;		\
  if (print_count >= 21)	\
    { printf ("--Type Return to print more--");	\
      print_count = 0;		\
      fflush (stdout);		\
      read_line (); } }

static void
list_symbols (regexp, class)
     char *regexp;
     int class;
{
  register struct symtab *s;
  register struct blockvector *bv;
  struct blockvector *prev_bv = 0;
  register struct block *b;
  register int i, j;
  register struct symbol *sym;
  char *val;
  int found_in_file;
  static char *classnames[]
    = {"variable", "function", "type", "method"};
  int print_count = 0;

  if (regexp)
    if (val = (char *) re_comp (regexp))
      error ("Invalid regexp: %s", val);

  printf (regexp
	  ? "All %ss matching regular expression \"%s\":\n"
	  : "All defined %ss:\n",
	  classnames[class],
	  regexp);

  for (s = symtab_list; s; s = s->next)
    {
      found_in_file = 0;
      bv = BLOCKVECTOR (s);
      /* Often many files share a blockvector.
	 Scan each blockvector only once so that
	 we don't get every symbol many times.
	 It happens that the first symtab in the list
	 for any given blockvector is the main file.  */
      if (bv != prev_bv)
	for (i = 0; i < 2; i++)
	  {
	    b = BLOCKVECTOR_BLOCK (bv, i);
	    for (j = 0; j < BLOCK_NSYMS (b); j++)
	      {
		QUIT;
		sym = BLOCK_SYM (b, j);
		if ((regexp == 0 || re_exec (SYMBOL_NAME (sym)))
		    && ((class == 0 && SYMBOL_CLASS (sym) != LOC_TYPEDEF
			 && SYMBOL_CLASS (sym) != LOC_BLOCK)
			|| (class == 1 && SYMBOL_CLASS (sym) == LOC_BLOCK)
			|| (class == 2 && SYMBOL_CLASS (sym) == LOC_TYPEDEF)
			|| (class == 3 && SYMBOL_CLASS (sym) == LOC_BLOCK)))
		  {
		    if (!found_in_file)
		      {
			printf ("\nFile %s:\n", s->filename);
			print_count += 2;
		      }
		    found_in_file = 1;
		    MORE;
		    if (class != 2 && i == 1)
		      printf ("static ");
		    if (class == 2
			&& SYMBOL_NAMESPACE (sym) != STRUCT_NAMESPACE)
		      printf ("typedef ");

		    if (class < 3)
		      {
			type_print (SYMBOL_TYPE (sym),
				    (SYMBOL_CLASS (sym) == LOC_TYPEDEF
				     ? "" : SYMBOL_NAME (sym)),
				    stdout, 0);
			printf (";\n");
		      }
		    else
		      {
			char buf[1024];
# if 0
			type_print_base (TYPE_FN_FIELD_TYPE(t, i), stdout, 0, 0); 
			type_print_varspec_prefix (TYPE_FN_FIELD_TYPE(t, i), stdout, 0); 
			sprintf (buf, " %s::", TYPE_NAME (t));
			type_print_method_args (TYPE_FN_FIELD_ARGS (t, i), buf, name, stdout);
# endif
		      }
		    if (class == 2
			&& SYMBOL_NAMESPACE (sym) != STRUCT_NAMESPACE
			&& (TYPE_NAME ((SYMBOL_TYPE (sym))) == 0
			    || 0 != strcmp (TYPE_NAME ((SYMBOL_TYPE (sym))),
					    SYMBOL_NAME (sym))))
		      printf (" %s", SYMBOL_NAME (sym));
		  }
	      }
	  }
      prev_bv = bv;
    }
}

static void
variables_info (regexp)
     char *regexp;
{
  list_symbols (regexp, 0);
}

static void
functions_info (regexp)
     char *regexp;
{
  list_symbols (regexp, 1);
}

static void
types_info (regexp)
     char *regexp;
{
  list_symbols (regexp, 2);
}

static void
methods_info (regexp)
     char *regexp;
{
  list_symbols (regexp, 3);
}

/* Initialize the standard C scalar types.  */

static
struct type *
init_type (code, length, uns, name)
     enum type_code code;
     int length, uns;
     char *name;
{
  register struct type *type;

  type = (struct type *) xmalloc (sizeof (struct type));
  bzero (type, sizeof *type);
  TYPE_MAIN_VARIANT (type) = type;
  TYPE_CODE (type) = code;
  TYPE_LENGTH (type) = length;
  TYPE_FLAGS (type) = uns ? TYPE_FLAG_UNSIGNED : 0;
  TYPE_FLAGS (type) |= TYPE_FLAG_PERM;
  TYPE_NFIELDS (type) = 0;
  TYPE_NAME (type) = name;

  /* C++ fancies.  */
  TYPE_NFN_FIELDS (type) = 0;
  TYPE_BASECLASS (type) = 0;
  return type;
}

static
initialize ()
{
  add_info ("variables", variables_info,
	    "All global and static variable names, or those matching REGEXP.");
  add_info ("functions", functions_info,
	    "All function names, or those matching REGEXP.");
  add_info ("types", types_info,
	    "All types names, or those matching REGEXP.");
  add_info ("methods", methods_info,
	    "All method names, or those matching REGEXP::REGEXP.\n\
If the class qualifier is ommited, it is assumed to be the current scope.\n\
If the first REGEXP is ommited, then all methods matching the second REGEXP\n\
are listed.");
  add_info ("sources", sources_info,
	    "Source files in the program.");

  obstack_init (symbol_obstack);

  builtin_type_void = init_type (TYPE_CODE_VOID, 0, 0, "void");

  builtin_type_float = init_type (TYPE_CODE_FLT, sizeof (float), 0, "float");
  builtin_type_double = init_type (TYPE_CODE_FLT, sizeof (double), 0, "double");

  builtin_type_char = init_type (TYPE_CODE_INT, sizeof (char), 0, "char");
  builtin_type_short = init_type (TYPE_CODE_INT, sizeof (short), 0, "short");
  builtin_type_long = init_type (TYPE_CODE_INT, sizeof (long), 0, "long");
  builtin_type_int = init_type (TYPE_CODE_INT, sizeof (int), 0, "int");

  builtin_type_unsigned_char = init_type (TYPE_CODE_INT, sizeof (char), 1, "unsigned char");
  builtin_type_unsigned_short = init_type (TYPE_CODE_INT, sizeof (short), 1, "unsigned short");
  builtin_type_unsigned_long = init_type (TYPE_CODE_INT, sizeof (long), 1, "unsigned long");
  builtin_type_unsigned_int = init_type (TYPE_CODE_INT, sizeof (int), 1, "unsigned int");
}

END_FILE
