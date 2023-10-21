
extern int cmds(),ls(), cp(), rm(), do_prog(),pushd(),popd(),drive(), ver(),
		more(),fgrep(),scr_clear(),set(),ch_mod(),cat(),echo(), 
		y(),t(),dump(),
		last(),invalid(),mv(),md(),touch(),cd(),pwd(),rd(),hist(),my_exit();
typedef struct
{
	char *cmdname;
	int (*func)();
} builtin;
builtin commands[] =
{
	"a:",drive,
	"b:",drive,
	"c:",drive,
	"cat",cat,
	"cd",cd,
	"chdir",cd,
	"chmod",ch_mod,
	"cls",scr_clear,
	"commands",cmds,
	"copy",cp,
	"cp",cp,
	"copy",cp,
	"d:",drive,
	"del",rm,
	"dir",ls,
	"dump",dump,
	"e:",drive,
	"echo",echo,
	"era",rm,
	"erase",rm,
	"error",last,
	"exit",my_exit,
	"f:",drive,
	"fgrep",fgrep,
	"g:",drive,
	"h:",drive,
	"hd",dump,
	"hist",hist,
	"history",hist,
	"i:",drive,
	"j:",drive,
	"ls",ls,
	"md",md,
	"mkdir",md,
	"more",more,
	"mv",mv,
	"no history",invalid,
	"popd",popd,
	"pushd",pushd,
	"pwd",pwd,
	"rd",rd,
	"rm",rm,
	"rmdir",rd,
	"set",set,
	"tee",t,
	"touch",touch,
	"version",ver,
	"y",y
};
int numcmds =  (sizeof(commands)/sizeof(builtin));
