
#define SPACE        	' '
#define CALLER       	'*'
#define COMMENT     	'#'
#define U_OPT_ENABLE	'@'
#define DELIMITER    	'-'
#define ID_PARTITION 	':'

#define TAB         	'\t'
#define NEW_LINE    	'\n'

#define UNUSED       	-1
#define NILL          	 0
#define SINGLE_OPT    	 1
#define DOUBLE_OPT   	 2
#define MAX_FIELDS   	50
#define MAX_LINE       256

#define F_NULL        (struct field *)0
#define P_NULL        (struct passwd *)0

struct field {

	char *front;	/* Points to the front of a field */
	char *end;	/* Points to the rear of a field  */

};

char *infile;		/* Actual file to be assigned in the program code */
