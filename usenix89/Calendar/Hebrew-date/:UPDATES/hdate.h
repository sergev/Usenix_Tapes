#define MINWEST (-120)	/* minutes west of GMT */
/* #define HEBREW 	/* output for Hebrew printer */
/* #define REV	/* " that is not bi-directional */
/* #define BIGLONG	/* if your longs >= 34 bits */
 struct hdate {
	int hd_day;
	int hd_mon;
	int hd_year;
	int hd_dw;
	int hd_flg;
} *hdate();

char	*mname[], *hmname[];
#ifdef HEBREW
	char *hnum();
#ifdef REV
char *rev();
#else
#define rev(s) s
#endif
#endif
