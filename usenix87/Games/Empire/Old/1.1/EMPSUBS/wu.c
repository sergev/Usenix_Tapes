#define D_FILES
#define D_NATSTR
#define D_TELSTR
#include        "empdef.h"

wu(from, to, letter)
char    *letter;
int     to, from;
{
        register        char    *cp;
        struct  telstr  tele;
        char    *mailbox();
        long    lseek();

        if( getnat(to) == -1 || nat.nat_stat == 0 ) return(-1);
        telf = open(mailbox(to), O_RDWR);
        if( telf == -1 ) return(-1);
        lseek(telf, 0L, 2);
        sigsave();
        tele.tel_from = from;
        time(&tele.tel_date);
        cp = letter;
        while( *cp++ ) continue;
        if( (tele.tel_length = cp - letter) > 512 ) {
                tele.tel_length = 512;
        }
        write(telf, &tele, sizeof(tele));
        write(telf, letter, tele.tel_length);
        close(telf);
        nat.nat_tgms++;
        if( putnat(to) == -1 ) return(-1);
        return(0);
}
