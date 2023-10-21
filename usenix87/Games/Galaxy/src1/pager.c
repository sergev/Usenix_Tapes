/*
 * %W% (mrdch&amnnon) %E%
 */

# include <stdio.h>

main(ac, av)
int     ac ;
char    **av ;
{
        FILE    *tty ;

        if(ac != 3)
                getout("arg count") ;

        tty = fopen(av[1], "w") ;
        if(tty == NULL)
                getout("cannot open terminal") ;

        fprintf(tty, "\07\07%s would like to play galaxy with you.\n\r",av[2]) ;
        fprintf(tty, "\07\07If you wish to play exec /usr/games/okgalaxy\n\r") ;

        exit(0) ;
}

getout(s)
char    *s ;
{
        fprintf(stderr, "pager: %s.\n", s) ;
        exit(1) ;
}
