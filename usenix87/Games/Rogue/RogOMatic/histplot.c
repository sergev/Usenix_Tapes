/*
 * histplot.c: Rog-O-Matic XIV (CMU) Fri Dec 28 22:13:21 1984 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * This program takes a Rog-O-Matic log file and produces a histogram
 * of the scores.
 */

# include <stdio.h>
# define SKIPARG	while (*++(*argv)); --(*argv)

# define BWIDTH 200
# define NUMBUK 51
# define BUCKET(n) (((n)+BWIDTH/2)/BWIDTH)
# define isdigit(c) ((c) >= '0' && (c) <= '9')
# define NOMON 29

int cheat = 0;

main (argc, argv)
int argc;
char *argv[];
{ int score = 0, maxfreq = 0, lowscore = 0, min = 200, killnum = 0;
  int bucket[NUMBUK], killed[NUMBUK][NOMON], level = 0, dolev = 0;
  int total[NOMON];
  register int i, j, h, f;
  char killer[100], plot[128];  

  /* Zero the buckets */
  for (i = NUMBUK; i--; )
  { bucket[i] = 0;
    for (j = NOMON; j--; )
      killed[i][j] = 0;
  }
  for (j = NOMON; j--;)
    total[j] = 0;

  /* Get the options */
  while (--argc > 0 && (*++argv)[0] == '-')
    while (*++(*argv))
    { switch (**argv)
      { case 'c': cheat++; break; /* List cheat games */
        case 'l': dolev++; break; /* Plot level instead of score */
	case 'a': min = atoi (*argv+1); SKIPARG; break;
        default:  printf ("Usage: histplot [-c]\n");
                  exit (1);
      }
    }

  /*  Print out the header */
  printf ("         %s  Histogram of Rog-O-Matic %s\n\n",
          dolev ? "" : "            ", dolev ? "Levels" : "Scores");
  printf ("\n");
  if (dolev)
    printf ("Games     1   5   10   15   20   25   30\n");
  else
    printf ("Games    0      2000      4000      6000      8000     10000\n");

  /* While more scores do action for each score */
  while (getscore (&score, killer, &level) != EOF)
  {
    if (score < min) { lowscore++; continue; }

    if (dolev) { h = level; }
    else       { if ((h = BUCKET(score)) >= NUMBUK) h = NUMBUK-1; }

    bucket[h]++;
    
    if (stlmatch (killer, "arrow"))			killnum = 1;
    else if (stlmatch (killer, "black unicorn"))	killnum = 'u'-'a'+2;
    else if (stlmatch (killer, "bolt"))			killnum = 1;
    else if (stlmatch (killer, "dart"))			killnum = 1;
    else if (stlmatch (killer, "fatal error trap"))	killnum = 0;
    else if (stlmatch (killer, "floating eye"))		killnum = 'e'-'a'+2;
    else if (stlmatch (killer, "gave"))			killnum = 0;
    else if (stlmatch (killer, "giant ant"))		killnum = 'a'-'a'+2;
    else if (stlmatch (killer, "hypothermia"))		killnum = 'i'-'a'+2;
    else if (stlmatch (killer, "quit"))			killnum = 28;
    else if (stlmatch (killer, "starvation"))		killnum = 'e'-'a'+2;
    else if (stlmatch (killer, "user"))			killnum = 0;
    else if (stlmatch (killer, "venus flytrap"))	killnum = 'f'-'a'+2;
    else if (stlmatch (killer, "violet fungi"))		killnum = 'f'-'a'+2;
    else killnum = *killer - 'a' + 2;

    killed[h][killnum]++;
        
    if (bucket[h] > maxfreq) maxfreq = bucket[h];
  }

  for (f = ((maxfreq+9)/10)*10; f; f--)
  { if (dolev)
    { if (f%10 == 0)
        sprintf (plot, "|----+----|----+----|----+----|");
      else if (f%5 == 0)
        sprintf (plot, "|    +    |    +    |    +    |");
      else
        sprintf (plot, "|         |         |         |");
    }
    else
    { if (f%10 == 0)
        sprintf (plot, "|----+----|----+----|----+----|----+----|----+----|");
      else if (f%5 == 0)
        sprintf (plot, "|    +    |    +    |    +    |    +    |    +    |");
      else
        sprintf (plot, "|         |         |         |         |         |");
    }
    
    for (i = 0; i < NUMBUK; i++)
      if (bucket[i] >= f)
      { plot[i] = '#';
        for (j = NOMON; j--;)
        { if (killed[i][j] > 0)
	  { killed[i][j]--;
	    plot[i] = "$@ABCDEFGHIJKLMNOPQRSTUVWXYZ#"[j];
	    total[j]++;
	    break;
	  }
	}
      }
    
    if (f%5 == 0)
      printf ("     %3d %s\n", f, plot);
    else
      printf ("         %s\n", plot);
  }

  if (dolev)
  {
    printf ("         |----+----|----+----|----+----|\n");
    printf ("          1   5   10   15   20   25   30\n");
  }
  else
  {
    printf ("         |----+----|----+----|----+----|----+----|----+----|\n");
    printf ("         0      2000      4000      6000      8000     10000\n");
  }


  printf ("\n\n");
  if (total[28])
    printf ("             # Quit\n");
  printf ("           A-Z Monster which killed Rog-O-Matic\n");
  if (total[1])
    printf ("             @ Killed by an arrow, bolt, or dart\n");
  if (total[0])
    printf ("             $ Killed by user or error\n");
  if (lowscore)
    printf ("      %8d scores below %d not printed.\n", lowscore, min);
}

# define LEVELPOS 47

getscore (score, killer, level)
int *score, *level;
char *killer;
{ int dd, yy;
  char line[128], mmstr[8], player[16], cheated=' ';
  while (fgets (line, 128, stdin))
  { dd = yy = *score = 0;
    sscanf (line, "%s %d, %d %10s%d%c%17s",
            mmstr, &dd, &yy, player, score, cheated, killer);
    if (strlen (line) > LEVELPOS) *level = atoi (line+LEVELPOS);
    if (yy > 0 &&
        (cheated != '*' || cheat) &&
        !stlmatch (killer, "saved") &&
        (*score > 2000 || !stlmatch (killer, "user")))
      return (1);
  }
  return (EOF);
}
