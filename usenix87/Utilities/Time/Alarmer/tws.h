/* tws.h */

struct tws {
    int     tw_sec;
    int     tw_min;
    int     tw_hour;
    int     tw_mday;
    int     tw_mon;
    int     tw_year;
    int     tw_wday;
    int     tw_yday;
    int     tw_isdst;
    int     tw_zone;
    int     tw_sday;
};

char   *dtime (), *dasctime (), *dtimezone (), *dctime (), *dtimenow ();
struct tws *dlocaltime (), *dparsetime ();
