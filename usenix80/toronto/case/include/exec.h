/*
 *	Format of an a.out header
 */

struct exec {
	int		a_magic;	/* magic number */
	unsigned	a_text;		/* text size */
	unsigned	a_data;		/* data size */
	unsigned	a_bss;		/* bss size */
	unsigned	a_syms;		/* symbol size */
	unsigned	a_entry;	/* entry point */
	unsigned	a_unused;	/* not used */
	unsigned	a_flag;		/* relocation info stripped */
};

# define A_MAGIC1	0407	/* normal */
# define A_MAGIC2	0410	/* RO text */
# define A_MAGIC3	0411	/* separated I&D */
# define A_MAGIC4	0405	/* overlay */

/*
 *	Format of a symbol table entry
 */

struct nlist {
	char		n_name[8];	/* symbol name */
	int		n_type;		/* type flag */
	unsigned	n_value;	/* value */
};

/*	Values for n_type	*/
# define N_UNDF	0	/* undefined */
# define N_ABS	01	/* absolute */
# define N_TEXT	02	/* text */
# define N_DATA	03	/* data */
# define N_FN	037	/* file name symbol */
# define N_EXT	040	/* external */
