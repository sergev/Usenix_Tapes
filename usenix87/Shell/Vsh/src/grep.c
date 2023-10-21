#include "hd.h"
#include "strings.h"
#include "command.h"

/* Interface to grep */

#define GBUFLEN 200
#define LIM1	(gbuf + 100)

#define QUOTE	'\''
#define BSLASH	'\\'

#define GSTR1	" -n '"
#define GSTR2	"' /dev/null "

#define CPSET	{cp = gbuf + strlen (gbuf);}

#define	GREPLEAVE	{tty_pop ();  clearmsg (0);  return NOREPLOT;}

/* User is asked to supply patterns and file names.  Grep is then
   run with its output directed to .grepout.  If the user changes his
   mind, and leaves without running grep, NOREPLOT is returned.
   Else REPLOT | CMD_SG is returned.
*/
grep () 
{
    register char *cp, *clim; register ch;
    char gbuf [GBUFLEN];
    char inline [STRMAX]; int inlength; register char *incp;

    FILE *sstream;
    extern FILE *showopen ();

    int saveout;

    extern char wdname [];

    tty_push (COOKEDMODE);
    strcpy (gbuf, GREP); strcat (gbuf, GSTR1);
    clearmsg (2); putmsg("Grep pattern:");
    printf(" ");
    CPSET; clim = LIM1;

    inlength = getline (inline); incp = inline;
    if (inlength == 0) GREPLEAVE;

    while ((ch = *incp++) && cp < clim) 
    {
	if (ch == QUOTE) 
	{
	    *cp++ = QUOTE; *cp++ = BSLASH;
	    *cp++ = QUOTE; *cp++ = QUOTE;
	}
	else *cp++ = ch;
    }
    *cp=0;
    strcat (gbuf, GSTR2);
    atxy (20 + inlength, window - 1);
    hilite ("%sGrep files:", (*SO ? "" : "-- "));
    printf(" ");

    CPSET;

	ch = inlength;
	inlength = xgetline (stdin, cp, GBUFLEN - strlen (gbuf));
	if (inlength == 0) GREPLEAVE;
	if (strcmp(cp, "$") == 0) {
		if (VSHMODE == SELECTMODE && selecttype == DIRCMD
			&& *selectname) {
			strcpy(cp, selectname);
			atxy(ch + 32 + (*SO ? 0 : 3), window - 1);
			printf("%s\r\n", cp);
		}
		else {
			hilite("Nothing selected");
			GREPLEAVE;
		}
	}

    /* Now run the command in gbuf */

    sstream = showopen ("w", GREPMODE);
    if (sstream == NULL) GREPLEAVE;

    hilite ("Searching");

    saveout = dup (outfile);	/* Set up files */
    close (outfile); dup (fileno (sstream)); fclose (sstream);

    printf ("%s is search directory\r\n", wdname);
    mysystem (gbuf);
    close (outfile); dup (saveout); close (saveout);


    tty_pop (); return CMD_SG | REPLOT;
}
