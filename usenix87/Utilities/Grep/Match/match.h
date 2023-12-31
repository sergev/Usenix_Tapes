#define FIRSTCHAR ' '
#define MAXCHAR 0377
#define MAXBUFF 8192
#define MAXSIZE 100
#define MAXPATS 100 /* max number of patterns */
#define PSIZEDEF 1024 /* default storage for patterns from a tty */
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))
struct PattDesc {
	char *Pattern;
	int PatLen; /* pattern length */
	char *Start; /* starting position of search (at beginning of pattern) */
	int Success; /* true when pattern found */
};
