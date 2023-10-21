/*
 *	ic.h
 *
 */

# define UNDEF		0
# define VARTYPE	1
# define FUNCTYPE	2
# define ARRAYTYPE	3
# define STACKTYPE	4
# define BUILTIN	5

typedef struct symbol {
	struct symbol	*s_next;	/* linked hash chains */
	struct symbol	*s_back;	/* doubly linked for deleting */
	char		*s_name;
	int		s_type;
	int		s_level;
	union {
		double		S_value;
		int		S_offset;
		struct {
			double		*S_data;
			int		S_size;
		} S_array;
		struct {
			int		S_argc;
			union {
				struct {
					struct symbol	*S_local;
					struct expr	*S_expr;
				} S_user;
				double		(*S_builtin)();
			} S_f;
		} S_func;
	} Su;
} symbol;

# define s_value	Su.S_value
# define s_offset	Su.S_offset
# define s_data		Su.S_array.S_data
# define s_size		Su.S_array.S_size
# define s_local	Su.S_func.S_f.S_user.S_local
# define s_expr		Su.S_func.S_f.S_user.S_expr
# define s_builtin	Su.S_func.S_f.S_builtin
# define s_argc		Su.S_func.S_argc

# define	NOTHING	0
# define	CONT	1
# define	BRK	2
# define	RET	3

typedef struct expr {
	int		e_tag;
	union {
		struct {
			struct expr	*Left;
			struct expr	*Right;
		} Es;
		double	Number;
		symbol	*Name;
		char	*String;
	} Eu;
} expr;

# define e_left		Eu.Es.Left
# define e_right	Eu.Es.Right
# define e_number	Eu.Number
# define e_name		Eu.Name
# define e_string	Eu.String

double	call();
