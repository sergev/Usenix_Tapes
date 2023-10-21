/*
 * %W% (mrdch&amnnon) %E%
 */

# include <sys/types.h>
# include "constants.h"
# include "score.h"
# include <stdio.h>

char   *tmpf = "/usr/games/lib/galaxy/scoreXXXXXX";
char   *ctime ();
int     tmp,
        sc;

main () {
    struct score    s;
    char    c;
    long time();

    sc = open (GALSCOR, 0);
    if (sc < 0) {
        perror (GALSCOR);
        exit (1);
    }
    tmp = creat (mktemp (tmpf), 0600);
    if (tmp < 0) {
        perror (tmpf);
        exit (1);
    }

    while (read (sc, &s, sizeof (s)) == sizeof (s)) {
        if (c != 'q') {
            char   *str,
                   *index ();
            str = ctime (&s.played_at);
            *index (str, '\n') = '\0';
            printf ("%s,%s %d %s ", s.win, s.los, s.years, str);
            c = getchar ();
            if (c != '\n')
                while (getchar () != '\n');
        }
        switch (c) {
            case 'd':
                break;
            case 'q':
            default:
                write (tmp, &s, sizeof (s));
                break;
            case 'a':
                write (tmp, &s, sizeof (s));
                printf("winner ? ");
                gets(s.win);
                printf("looser ? ");
                gets(s.los);
                printf("years ? ");
                rint(&s.years);
                s.played_at = time(0);
                write(tmp, &s, sizeof(s));
                break;
        }
    }
    if (unlink (GALSCOR) < 0) {
        fprintf (stderr, "unlink ");
        perror (GALSCOR);
        exit (1);
    }
    if (link (tmpf, GALSCOR) < 0) {
        fprintf (stderr, "link ");
        perror (GALSCOR);
        exit (1);
    }
    if (unlink (tmpf) < 0) {
        fprintf (stderr, "unlink ");
        perror (tmpf);
        exit (1);
    }
    exit (0);
}

rint(i)
int *i;
{
        char s[100];
        gets(s);
        *i = atoi(s);
}
