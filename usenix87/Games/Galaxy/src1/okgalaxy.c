/*
 * %W% (mrdch&amnnon) %E%
 */

# include <stdio.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/stat.h>

struct  stat    stbuf ;
char    *tty ;

/*
 * The reason for this file is that usually there is no READ
 * permission on someone else terminal. Giving this command
 * enables the main game to read whatever is typed by the
 * second player, and interpret his commands.
 */
main()
{
        char    *ttyname() ;
        int     doexit() ;

        tty = ttyname(0) ;

        if(tty == 0)
        {
                fprintf(stderr, "Don't know you.\n") ;
                exit(1) ;
        }

/*
 * collect terminal situation in a temporary buf.
 */
        if(stat(tty, &stbuf) == -1)
	{
		fprintf(stderr, "Can't stat() %s.\n", tty) ;
		exit(1) ;
	}
/*
 * prepare for leaving the game when it's all over.
 */
        signal(SIGINT, doexit) ;
        signal(SIGQUIT, doexit) ;
#ifdef SIGTSTP
        signal(SIGTSTP, doexit) ;
#endif SIGTSTP

/*
 * try to make the terminal readable to all.
 */
        if((stbuf.st_mode != 0666) && (chmod(tty, 0666) == -1))
        {
                fprintf(stderr, "Your tty (%s) ain't yours.\n", tty) ;
                exit(1) ;
        }

/*
 * all went fine, relax and let the originator to catch up.
 */
        printf("Please wait.....") ;
        fflush(stdout) ;

/*
 * Suspend all activity. To exit this mode after the game
 * has finished, one needs to interrupt it with a keybourd
 * signal.
 */
        pause() ;
}

/* restore terminal mode to original state. */
doexit()
{
        chmod(tty, stbuf.st_mode) ;
        exit(0) ;
}
