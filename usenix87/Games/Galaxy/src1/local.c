/*
 * %W% (mrdch&amnnon) %E%
 */

# include <stdio.h>

struct  chan
{
        int     ichan ;
        char    c ;
} ;

/*
 * this file is invoked to create TWO reading processes.
 * Each process reads from it's respective terminal,
 * and writes it to the main process.
 * The SOURCE is identified by the "ichan" no.
 */
main(ac, av)
int     ac ;
char    **av ;
{
        struct  chan    c ;
        if(ac != 2)
                exit(1) ;

        c.ichan = atoi(av[1]) ;
        while(!feof(stdin))
        {
                c.c = getchar() ;
                write(1, (char *)&c, sizeof(c)) ;
        }
        exit(0) ;
}
