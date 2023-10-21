/* There are two important structures used by the command processor.
   Cmdtab encodes the current commands.  Each command corresponds to
   an element in cmdtab and is type struct cmdstruct.  Four fields
   determine the command.  Cmd_char is the character which, when
   pressed, calls the command.  Cmd_proc is the procedure which
   runs the command.  Cmd_argv is used as a parameter to that proc.
   Cmd_xdir, when on, indicates the command may be run anywhere, and
   not just on the directory page.

   The classtab is used to map statements from the .vshrc file to
   cmdtab elements.  The keyword in a .vshrc statement is looked
   up in classtab, and then the classtab element is used to load
   the cmdtab element.
*/

struct classstruct {		/* Classification structure */
	char * cl_name;		/* Name (key word) */
	int (*cl_proc)();	/* Command procedure */
	short cl_count;		/* Number of args */
	char cl_xdir;		/* Is ok outside dir page */
};

struct cmdstruct {		/* Command structure */
	char cmd_char;		/* Command char */
	int  (*cmd_proc)();	/* Command procedure */
	char ** cmd_argv;	/* Array of arguments for command */
	char cmd_xdir;		/* Is ok outside dir page */
};

struct classstruct *classloc();
struct cmdstruct   *cmdloc  ();
extern (*cmdproc ())();

extern struct classstruct classtab[];
extern struct cmdstruct   cmdtab[];

/* Command characters for important commands */

#define	CMD_DATE	0x80
#define	CMD_SE		0x81
#define	CMD_SG		0x82
