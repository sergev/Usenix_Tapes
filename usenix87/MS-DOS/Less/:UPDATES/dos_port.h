/* this file is necessary to handle all the defines for building the program
   using Microsoft C. The compiler will not accept them all on the command
   line when they are included in the makefile.
 */
#ifdef DOSPORT
#define MSDOS		1
#define MSC		1	/* only if using Microsoft C compiler */
#define REGEXP		1
#define EDITOR		1
#define EDIT_PGM	"memacs"
#define SHELL_ESCAPE	1
#endif
    
