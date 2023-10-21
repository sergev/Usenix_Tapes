/*
 * process - process options for readnews
 */

static char *SccsId = "%W%	%G%";

#include <stdio.h>
#include <ctype.h>
#include "roptions.h"
#define TRUE 1
#define FALSE 0

int mode = UNKNOWN;


procopt(argv)
register char **argv;
{
      register int state = OPTION;
      register char *p;
      register struct optable *optpt;
      extern char *progname;
      char *savestr(), *realloc();

      /* loop once per arg. */

      while (*argv != NULL) {
          if (state == OPTION) {
                  if (**argv != '-') {
                        xerror("Bad option string \"%s\"", *argv);
                  }
                  while (*++*argv != '\0') {
                        for (optpt = options; optpt->optlet != '\0'; ++optpt) {
                              if (optpt->optlet == **argv)
                                    goto found;
                        }
                        /* unknown option letter */
                        fprintf(stderr, "Usage: %s [ -a [ date ]] [ -n newsgroups ] [ -t titles ] [ -lprxhfuM ]\n", progname);
                        fprintf(stderr, "\t[ -c [ ``mailer'' ]]\n\n");
                        fprintf(stderr, "       %s -s\n", progname);
                        exit(1);

                      found:;
                        if (mode != UNKNOWN && (mode&optpt->oldmode) == 0) {
                              xerror("Bad %c option", **argv);
                        }
                        if (mode == UNKNOWN)
                              mode = optpt->newmode;
                        optpt->flag = TRUE;
                        state = optpt->newstate;
                  }

                  argv++;            /* done with this option arg. */

          } else {

                  /*
                   * Pick up a piece of a string and put it into
                   * the appropriate buffer.
                   */
                  if (**argv == '-') {
                        state = OPTION;
                        continue;
                  }

                  p = optpt->buf;
                  if (state == STRING) {
                        if (optpt->buf != NULL)
                              free(optpt->buf);
                        optpt->buf = savestr(*argv);
                        state++;
                  } else {
                        if ((p = realloc(p, strlen(p) + strlen(*argv) + 2)) == NULL)
                              xerror("No space");
                        optpt->buf = p;
                        while (*p)  p++;
                        *p++ = optpt->filchar;
                        strcpy(p, *argv);
                  }
                  argv++;
            }
      }

      /*
       * At this point, we are done with vanilla flag processing.
       * Now deal with defaults and upward compatibility stuff.
       */
      return;
}


/*
 * Process a string containing options.  The string is broken up into
 * space separated options and then passed to procopt.
 */

#define MAXOPTS 63

procostr(rcbuf)
      char *rcbuf;
      {
      register char *ptr;
      char *rcline[MAXOPTS + 1];
      int line = -1;

      strcat(rcbuf, " \1");
      ptr = rcbuf;
      while (*++ptr)
            if (isspace(*ptr))
                  *ptr = '\0';
      for (ptr = rcbuf; ; ptr++) {
            if (!*ptr)
                  continue;
            if (*ptr == '\1')
                  break;
            if (++line >= MAXOPTS)
                  xerror("Too many options");
            rcline[line] = ptr;
            while (*ptr)
                  ptr++;
      }
      rcline[++line] = NULL;
      procopt(rcline);
}
