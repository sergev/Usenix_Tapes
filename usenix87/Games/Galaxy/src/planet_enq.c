/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

planetenq (s)
char   *s;
{
    planet * pp;
    int     ntellers;
    int     enqtype;
    int     enqlvl = 0;

    pp = getpl (s);
    skipwhite (s);
    if (isdigit (*s)) {
        enqlvl = *s++ - '0';
/* calculate level of espionage */
        if (enqlvl > ESPSIZ) {
            say ("Sir, we don't have such level of spies!");
            return;
        }
        skipwhite (s);
    }
/* see what subject he wants to enquire */
    switch (*s++) {
        case 's':
            enqtype = ESPKIND;
            break;
        case 'p':
            enqtype = ESPPOP;
            break;
        case 'k':
            enqtype = ESPKNOW;
            break;
        case 't':
            enqtype = ESPMTL;
            break;
        case 'f':
            enqtype = ESPSHIP;
            break;
        case 'a':
            enqtype = ESPALM;
            break;
        case 'm':
            enqtype = ESPMSL;
            break;
        default:
            say ("Our spies don't recognize that kind of info, Sir. ");
            return;
    }
    skipwhite (s);
    ntellers = atoi (s);
    assert_money (ntellers);
/* see if enough money is available */
    teller[player] -= ntellers;
    if (pp -> whos == 2) {
        if (ntellers >= MIN_ESP) {
            say ("The rumors has it that this planet is uninhabited.");
            spy_msg ("An empty planet", player, pp, 0);
        }
        else
            say ("But sir, that little money won't bring any information !!!");
        return;
    }
/*  perform actual investment */
    pp -> espion[player][enqtype][enqlvl] += ntellers;
    say ("The spies are on their way, sir.");
}
