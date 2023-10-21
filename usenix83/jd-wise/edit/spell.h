struct process spellproc;
#define spellpid spellproc.pr_pid
#define fmspell  spellproc.pr_fmbkg
#define tospell  spellproc.pr_tobkg

int	onemt();		/* handler for spell signal */

char	badword[128];		/* most recent misspelled word */
