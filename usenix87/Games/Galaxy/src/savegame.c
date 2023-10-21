/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"
# include <signal.h>
# include <pwd.h>

extern char *sys_errlist[];
char errmsg[511];
extern errno;

movedir (saver)
char   *saver;
{
    struct passwd  *p;
    struct passwd  *getpwnam ();

    p = getpwnam (saver);
    if (p == 0)
        return;
    else
        if (chdir (p -> pw_dir) != -1)
            return;
        else
            return;
}

# include "table.h"
# define TABSIZ 100
# define K 1024

int     SWITCH_FLAG;

char    version[80] = "Galaxy: version 1 6/9/84";

/*
 *  A few helpfull hints about galaxy's save file structure:
 *    The save file is made of three parts:
 *    1. Galaxy version.
 *    2. The program break.
 *    3. global information.
 *
 *  save should read the version, and compare it to the current.
 *  if they don't match, save must abort.
 *  after reading the program break, it should be re-set using
 *  brk(1).
 *  Now, save is prepared to read back all global data.
 */

char   *
        save (f)
char   *f;
{
    extern  end;
    extern  edata;
    extern  etext;
    char   *sbrk ();
    char   *sbrk0;
    int     savef;
    int     nwrite;
    char   *realetext;

    umask (0);
    realetext = (char *) & etext;
    realetext = (char *) ((int) (realetext + K - 1) & ~(K - 1));
    savef = creat (f, 0600);

    if (savef == -1) {
        (void) sprintf(errmsg, "%s: %s", f, sys_errlist[errno]);
        return(errmsg);
   }

    sbrk0 = sbrk (0);

    enc_write (savef, version, 80);
    if (enc_write (savef, (char *) & sbrk0, sizeof (sbrk0)) != sizeof (sbrk0)) {
        (void) close (savef);
        unlink (f);
        return ("read error");
    }

    nwrite = (int) sbrk0 - (int) realetext;

    if (enc_write (savef, realetext, nwrite) != nwrite) {
        (void) close (savef);
        unlink (f);
        return ("read error");
    }
    (void) close (savef);

    return (0);
}

char   *
        restor (f)
char   *f;
{
    extern  etext;
    extern  edata;
    extern  end;
    int     savef;
    char   *brk ();
    char   *sbrk0;
    int     nread;
    char   *realetext;
    char    vers[81];

    realetext = (char *) & etext;
    realetext = (char *) ((int) (realetext + K - 1) & ~(K - 1));

    savef = open (f, 0);

    if (savef == -1) {
        (void) sprintf(errmsg, "%s: %s", f, sys_errlist[errno]);
        return(errmsg);
    }

    enc_read (savef, vers, 80);
    if (strcmp (vers, version) != 0) {
        close (savef);
    /* don't unlink here! */
        return ("ill formated save file");
    }
    if (enc_read (savef, (char *) & sbrk0, sizeof (sbrk0)) != sizeof (sbrk0)) {
        perror ("read sbrk0");
        (void) close (savef);
        return ("read error");
    }

    if (brk (sbrk0) == (char *) - 1) {
        perror ("brk");
        (void) close (savef);
        return ("not enough memory");
    }

    nread = (int) sbrk0 - (int) realetext;

    if (enc_read (savef, realetext, nread) != nread) {
        perror ("read etext");
        (void) close (savef);
        return ("read error");
    }

    return (0);
}

enc_write (fd, buf, nwrite)
int     fd,
        nwrite;
char   *buf;
{
    char    buf2[TABSIZ + 1];
    char   *cp;
    char   *bp = buf;
    int     i;
    int     nw = nwrite;

    while (nwrite > 0) {
        cp = buf2;
        for (i = 0; i < TABSIZ && nwrite > 0; i++, nwrite--) {
            *cp++ = crypt_tab[i] ^ *bp++;
        }

        if (write (fd, buf2, i) != i)
            return (-1);

    }

    return (nw);
}

enc_read (fd, buf, nread)
int     fd,
        nread;
char   *buf;
{
    char    buf2[TABSIZ + 1];
    char   *cp;
    char   *bp = buf;
    int     i;
    int     cc;
    int     nr = nread;

    while (nread > 0) {
        cp = buf2;
        cc = read (fd, cp, nread > TABSIZ ? TABSIZ : nread);

        if (cc != (nread > TABSIZ ? TABSIZ : nread))
            return (nr - nread);

        for (i = 0; i < TABSIZ && nread > 0; i++, nread--)
            *bp++ = crypt_tab[i] ^ *cp++;
    }

    return (nr);
}

savegame (fn)
char   *fn;
{
    static char fn1[100];
    char   *c;

    skipwhite (fn);
    if (wants_save[player]) {
        say ("save canceled");
        wants_save[player] = 0;
        return;
    }
    wants_save[player] = 1;

    if (wants_save[!player])
        goto saveit;

    strcpy (fn1, fn);
    dowrite_enemy ("Lets save the game, O.K ?", !player);
    say ("Ok, saving when he agrees...");
    if (player == 0)
        movedir (ply0);
    else
        movedir (ply1);
    return;

saveit:
    if (nullstr (fn) && nullstr (fn1))
        strcpy (fn, "galaxy.save");
    if (nullstr (fn1))
        strcpy (fn1, fn);
    if (nullstr (fn))
        strcpy (fn, fn1);
    if (strcmp (fn, fn1) != 0) {
        say ("Hey, he wants to save on \"%s\"!", fn1);
        wants_save[0] = wants_save[1] = 0;
        return;
    }

    wants_save[0] = wants_save[1] = 0;
    c = save (fn);
    if (c != 0)
        say (c);
    if (iswiz[1] || iswiz[2])
        return;
    endgame (-1);
}

restorgame (fn)
char   *fn;
{
    char   *c;
    static char fn1[100];
    char    pl[100];

    skipwhite (fn);
    if (wants_restor[player]) {
        say ("restor canceled");
        wants_restor[player] = 0;
        return;
    }
    wants_restor[player] = 1;

    if (wants_restor[!player])
        goto restorit;

    strcpy (fn1, fn);
    dowrite_enemy ("Lets restor, O.K ?", !player);
    if (player == 0)
        movedir (ply0);
    else
        movedir (ply1);
    return;

restorit:
    if (nullstr (fn) && nullstr (fn1))
        strcpy (fn, "galaxy.save");
    if (nullstr (fn1))
        strcpy (fn1, fn);
    if (nullstr (fn))
        strcpy (fn, fn1);
    if (strcmp (fn, fn1) != 0) {
        say ("Hey, he wants to restore on \"%s\"!", fn1);
        wants_restor[0] = wants_restor[1] = 0;
        return;
    }

 /*
  * save player0's name.
  */
    strcpy (pl, ply0);
    c = restor (fn);
    if (c != 0)
        say (c);
 /*
  * This is the hard part.  When restoring a game, the users
  * may have switched terminals. in this case, we have to
  * switch the terminals.
  */

    if (strcmp (ply0, pl) == 0) /* it's ok. */
        return;
 /* else                            o o, switched */

    SWITCH_FLAG = 1;
}

nullstr (s)
char   *s;
{
    return (*s == '\0');
}
