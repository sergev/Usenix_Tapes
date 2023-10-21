/*
 * %W% (mrdch&amnnon) %G%
 */

/*
 * this program displays best scores ( optional all ) from the score
 * file.
 * options:
 *      -t              10 best winners and related stat.
 *      -a              all scores sorted by time.
 *      -m              only my games.
 *      user            only `user''s games.
 */

# include       <sys/types.h>
# include       <sys/stat.h>
# include       "score.h"
# include       "constants.h"

struct reslts
{
        char    namepl[20] ;
        int     nwon ;
        int     nlost ;
        int     nyears ;
};

int     n_entries ;
struct  score   *entries ;
struct  reslts  *uprslts ;

glxscore(ac, av)
int     ac ;
char    **av ;
{
        struct  stat    stbuf ;
        char    *malloc() ;
        int     fd ;
        register int    i ;
        struct  reslts *scrpn ;
        char    *getlogin();


        if(stat(GALSCOR, &stbuf) == -1) {
                print("No score file.\n\r") ;
                return;
        }

        n_entries = stbuf.st_size / sizeof(struct score) ;
        if((n_entries * sizeof(struct score)) != stbuf.st_size) {
                print("Ill formatted score file.\n\r") ;
                return;
        }

        entries = (struct score *) malloc(stbuf.st_size) ;
        uprslts = (struct reslts *)
                        malloc(n_entries * sizeof(struct reslts)) ;
        if(entries == 0 || uprslts == 0) {
                print("cannot allocate entry space.\n\r") ;
                return;
        }
        scrpn = uprslts ;

        for (i = 0 ; i < n_entries ; i++ ,scrpn++ ) {
                scrpn->nwon = 0 ;
                scrpn->nlost = 0 ;
                scrpn->nyears = 0 ;
        }

        fd = open(GALSCOR, 0) ;
        if(fd == -1) {
                perror(GALSCOR) ;
                return;
        }

        if(read(fd, (char *) entries, stbuf.st_size) != stbuf.st_size) {
                perror("read error") ;
                return;
        }

        ac-- , av++ ;

        for(i = 0 ; i < ac ; i++) {
                if(av[i][0] == '-')
                        switch(av[i][1]) {
                                case 't' :
                                        dispbest(10) ;
                                        break;
                                case 'a' :
                                        dispall() ;
                                        break ;
                                case 'm' :
                                        dispuser(getlogin()) ;
                                        break ;
                                default  :
                                        print("unknown option.\n\r") ;
                                        return;
                        }
                else
                        dispuser(av[i]) ;
        }
}

dispuser(plname)
char    *plname ;
{
        struct  reslts *scrpn ;
        int     i, nply ;

        nply = findbest() ;
        scrpn = uprslts ;

        for (i = 0 ; i < nply ; i++ ,scrpn++)
                if (strcmp(scrpn->namepl , plname) == 0)
                        break ;

        if ( i == nply ) {
        print("\n\rIt seems that %s never played galaxy. Poor man...\n\r",plname) ;
                return;
        }
        print("\n\rThe GALAXY results for %s are:\n\r",plname) ;
        print("==================================\n\r") ;
        print("Games won: %d\tGames lost: %d\tTotal years played: %d\n\r\n\r",
                scrpn->nwon, scrpn->nlost, scrpn->nyears) ;

        print("This is the complete list:\n\r") ;
        print("\n\rWinner\tLooser\tYears\tWinning date and time\n\r") ;
        print("------------------------------------------------\n\r") ;
        for ( i = 0 ; i < n_entries ; i++ ) {
                if (strcmp(plname , entries[i].win) == 0 )
                        dispentr(i) ;
                if(strcmp (plname , entries[i].los) == 0 )
                        dispentr(i) ;
        }
}

dispbest(nbest)
{
        struct  reslts *scrpn ;
        int i, nply;

        nply = findbest() ;
        scrpn = uprslts ;
        print("\n\r******************************************************") ;
        print("\n\r\t\tBest players in GALAXY game\n\r") ;
        print("******************************************************\n\r\n\r") ;
        print("Rank\tPlayer\tWon\tLost\tYears\tScore\n\r") ;
        print("----------------------------------------------\n\r") ;

        for (i = 0 ; i < nbest && i < nply ; i++, scrpn++)
                print("%d\t%s\t%3d\t%3d\t%3d\t%3d\n\r",i+1, scrpn->namepl,
                scrpn->nwon, scrpn->nlost, scrpn->nyears, 2*scrpn->nwon - scrpn->nlost) ;
        print("\n\r") ;
}

findbest()
{
        int     i, j, k , p_players ;
        int     which ;
        int     qcmp() ;
        i = 0 ;
        p_players = 0 ;

        for(i = 0 ; i < n_entries ; i++) {
                if ( strcmp(entries[i].win , "-null") == 0 )
                        continue ;
                if ( strcmp(entries[i].los , "-null") == 0 )
                        continue ;
                if ( strcmp(entries[i].los , entries[i].win) == 0 )
                        continue ;
                which = -1 ;
                for (j = 0 ; j < p_players ; j++ ) {
                if (strcmp(uprslts[j].namepl,entries[i].win) == 0 ) {
                                which = j ;
                        }
                }
                if (which != -1)
                        updt_res(i,1,which) ;
                else
                        add_player(i,1,p_players++) ;
                which = -1 ;
                for (j = 0 ; j < p_players ; j++ ) {
                if(strcmp (uprslts[j].namepl,entries[i].los) == 0 ) {
                                which = j ;
                        }
                }
                if (which != -1)
                        updt_res(i,0,which ) ;
                else
                        add_player(i,0,p_players++) ;
        }
        qsort(uprslts, p_players, sizeof uprslts[0], qcmp);
        return(p_players) ;
}

add_player(at_entry,ifwon,new_player)
int     at_entry;       /* where is the player located at entry table */
int     ifwon;          /* did he win this time */
int     new_player;     /* pointer to the new players entry */
{
        if (ifwon)
                strcpy(uprslts[new_player].namepl,entries[at_entry].win);
        else
                strcpy(uprslts[new_player].namepl,entries[at_entry].los);
        updt_res(at_entry,ifwon,new_player) ;

}

updt_res(i,winner,player)
{
        uprslts[player].nwon  += winner ;
        uprslts[player].nlost += !winner ;
        uprslts[player].nyears  += entries[i].years ;
}


qcmp(p1,p2)
struct reslts *p1 ,*p2 ;
{
        return((2*p2->nwon - p2->nlost) - (2*p1->nwon - p1->nlost ) ) ;
}

dispall()
{
        register int    i ;

        print("\n\r******************************************************") ;
        print("\n\r\tThe complete list of the GALAXY games\n\r") ;
        print("******************************************************\n\r") ;
        print("\n\rWinner\tLooser\tYears\tWinning date and time\n\r") ;
        print("------------------------------------------------\n\r") ;
        for(i = 0 ; i < n_entries ; i++)
                dispentr(i) ;
}

dispentr(i)
{
        char *ctime();

                print("%s\t%s\t%d\t%s",
                        entries[i].win, entries[i].los,
                        entries[i].years,
                        ctime(&entries[i].played_at)) ;
}
