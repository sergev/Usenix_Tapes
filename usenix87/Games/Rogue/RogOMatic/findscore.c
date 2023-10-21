/*
 * findscore.c: Rog-O-Matic XIV (CMU) Fri Dec 28 23:27:10 1984 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * Read the Rogue scoreboard to determine a goal score.
 */

# include <stdio.h>
# include "install.h"
# define TEMPFL "/tmp/RscoreXXXXXX"
# define ISDIGIT(c) ((c) >= '0' && (c) <= '9')

findscore (rogue, roguename)
register char *rogue, *roguename;
{ register int score, best = -1;
  char cmd[100], buffer[BUFSIZ];
  register char *s, *tmpfname = TEMPFL;
  FILE *tmpfil;

  /* Run 'rogue -s', and put the scores into a temp file */
  sprintf (cmd, "%s -s >%s", rogue, mktemp (tmpfname)); 
  system (cmd); 

  /* If no temp file created, return default score */
  if ((tmpfil = fopen (tmpfname, "r")) == NULL)
    return (best); 

  /* Skip to the line starting with 'Rank...'. */
  while (fgets (buffer, BUFSIZ, tmpfil) != NULL)
    if (stlmatch (buffer, "Rank")) break;

  if (! feof (tmpfil)) 
  { best = BOGUS;
    while (fgets (buffer, BUFSIZ, tmpfil) != NULL)
    { s = buffer;				/* point s at buffer */
      while (ISDIGIT (*s)) s++;			/* Skip over rank */
      while (*s == ' ' || *s == '\t') s++;	/* Skip to score */
      score = atoi (s);				/* Read score */
      while (ISDIGIT (*s)) s++;			/* Skip over score */
      while (*s == ' ' || *s == '\t') s++;	/* Skip to player */

      if (score < best)				/* Save smallest score */
        best = score;
      if (stlmatch (s, roguename))		/* Found our heros name */
        break; 
    }
  }

  unlink (tmpfname); 
  return (best);
}
