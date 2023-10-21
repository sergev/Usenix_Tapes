
struct COMMAND {
    int  (*func)();
    int  stat;
    int  val;
    char *name;
};

extern struct COMMAND Command[];
extern char *Desc[];

