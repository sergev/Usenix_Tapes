/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

char    buffer1[BUFSIZ];
char    buffer2[BUFSIZ];

main (argc, argv)
int     argc;
char   *argv[];
{
/* If any argument present, go to score analysis */
    if (argc > 1){
        tty = stdout;
        glxscore (argc, argv);
        exit(0) ;
    }
    init_getid ();
    term2init ();
/* Get the terminal from the player enviroment */
    fillterm (getenv ("TERM"), &ttycs[1]);
/* Now go and start playing */
    play ();
/*
 * And when it's all over, return the terminal to the
 * original state. then kill the two reading processes
 */
    term1 ();
    ctrlreset ();
    term0 ();
    ctrlreset ();
    (void) kill (getpid (), 9);
}

play () {
/* Open the chanels for writing */
    ttys[0] = fdopen (chand[0], "w");
    ttys[1] = fdopen (chand[1], "w");
    setbuf (ttys[0], buffer1);
    setbuf (ttys[1], buffer2);
/* Initial the first terminal */
    term0 ();
    cap_set (ttyc);
    clear ();
    if (!iswiz[player])
        show_name ();
    fflush(tty) ;
/* Initial the second terminal */
    term1 ();
    cap_set (ttyc);
    clear ();
    if (!iswiz[player])
        show_name ();
    fflush(tty) ;

/* transform the name of the game to it's map */
    if (!iswiz[player]) {
        sleep (5) ;
        prologue ();
        sleep (3) ;
    }

/* redraw the map, enabling inverse video */
   termn (0);
   init_dis();
   cleol(18,0) ;
   cleol(19,0) ;
   cleol(20,0) ;
   cleol(21,0) ;
   cleol(22,20) ;
   cleol(23,0) ;
   prteller () ;
   fflush(tty) ;

   termn (1);
   init_dis();
   cleol(18,0) ;
   cleol(19,0) ;
   cleol(20,0) ;
   cleol(21,0) ;
   cleol(22,20) ;
   cleol(23,0) ;
   prteller () ;
   fflush(tty) ;

/* Go and start reading what the players are writing */
    parse ();
}

/* Functions to set the active terminal and the current player */
term0 () {
    tty = ttys[0];
    ttyc = &ttycs[0];
    player = 0;
}

term1 () {
    tty = ttys[1];
    ttyc = &ttycs[1];
    player = 1;
}
