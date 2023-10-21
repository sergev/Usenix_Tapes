/*
** score.c -	code dealing with maintaining the score file and the user log
**
**	Note:	the score file is not encrypted
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/
#include "pm.h"
#include <pwd.h>

extern	int	fwrite(), fread();

/*
** check_scrs()	- print out the pm roll (only for score enquiries)
*/
void	check_scrs ()
{
	reg	int	i;
	auto	score	scrs[MAX_SCORES];

	if (rwscore(0, scrs))
	{
		printf("No scores recorded at this time\n");
		return;
	}
	printf("\nTop Ten Players:\n");
	printf("Rank\tScore\tName\n");
	for (i = 0; i < MAX_SCORES && scrs[i].sc_uid != -1; i++)
	{
		print_scrs(&scrs[i], i + 1);
		printf(".\n");
	}
}

/*
** get_hi_scr()	- get the highest score from the score file
*/
long	get_hi_scr ()
{
	auto	score	scrs[MAX_SCORES];

	if (rwscore(0, scrs))
		return(0L);
	if (scrs[0].sc_uid == uid)
	{	/* taunt the high scorer */
		printf("\nWelcome back %s,\n", scrs[0].sc_name);
		printf("do you think that you can do better than\n");
		printf("last time?  Just a second while a think\n");
		printf("of a better strategy!\n");
		sleep(1);
#ifdef	PATTERNS
		/*
		** they seem to have this pattern mastered, so
		** lets be tricky and give them a different pattern
		** (i.e. change the seed)
		*/
		randomize(getpid());
#endif
	}
	return(scrs[0].sc_score);
}

#ifdef	PM_USER
/*
** chk_pm_user() -	check the user file
**			return non-zero on error
*/
int	chk_pm_user ()
{
	reg	int	i;
	reg	int	bit, byte;
	auto	char	buf[MAX_BYTES];

	if (!pm_user ||
	    uid > (unsigned) MAX_USERS)	/* wraps on 16 bit machines */
		return(FALSE);
	byte = uid / BYTE_SIZE;
	bit = 1 << (uid % BYTE_SIZE);
	for (i=0; i<MAX_BYTES; i++)
		buf[i] = '\0';
	if (rwuser(0, buf))
		fprintf(stderr, "%s: creating user file\n", argv0);
	if (!(buf[byte] & bit)) /* have they played before? */
	{
		directions(); 			/* give directions and */
		buf[byte] |= bit;
		if (rwuser(1, buf))
		{
			fprintf(stderr, "%s: cannot update user file\n", argv0);
			return(TRUE);
		}
	}
	return(FALSE);
}
#endif

/*
** print_scrs()	- print out the score record
*/
void	print_scrs (scr, rank)
reg	score	*scr;
reg	int	rank;
{
	static	char	*reason[] =
	{
		"eaten",
		"quit",
	};

	if (scr->sc_uid < 0)
		return;
	printf("%d\t%6ld\t%s: %s after %d screens",
		rank, scr->sc_score, scr->sc_name,
		reason[scr->sc_flags], scr->sc_level+1);
	if (scr->sc_flags == FL_DIE)
		printf(" by %s", mons_str(scr->sc_mons));
}

/*
** scores()	- print the list of scores and conditionally add
**		  the new one
*/
void	scores (mon, flags)
char	mon;
int	flags;
{
	reg	int	i;
	auto	score	scrs[MAX_SCORES];

	if (rwscore(0, &(scrs[0])))
	{
		fprintf(stderr, "%s: creating the score file\n", argv0);
		for (i=0; i<MAX_SCORES; i++)
			scrs[i].sc_uid = -1;	/* indicates an invalid score */
	}
	if (madeit(thescore, scrs))
	{
		auto	score	newscr;
		auto	char	buf[BUFSIZ];

		printf("You made it on the pm roll with ");
		printf("your score of %ld!\n", thescore);
		printf("Your name please: ");
		if (!gets(buf))
			printf("Sorry, that is not acceptable!\n");
		newscr.sc_score = thescore;
		newscr.sc_uid = uid;
		newscr.sc_level = level;
		newscr.sc_mons = mon;
		newscr.sc_flags = flags;
		strucpy(newscr.sc_name, buf, NAME_SIZE);
		newscr.sc_name[NAME_SIZE - 1] = NULL;
		/*
		** find where to insert their score
		*/
		for (i = 0; i < MAX_SCORES && scrs[i].sc_uid != -1; i++)
		{
			auto	score	scr;

			if (thescore < scrs[i].sc_score)
				continue;
			scrcpy(&scr, &(scrs[i]));
			scrcpy(&(scrs[i]), &newscr);
			for (++i; i < MAX_SCORES; i++)
			{
				auto	score	tmp;

				scrcpy(&tmp,&(scrs[i]));
				scrcpy(&(scrs[i]),&scr);
				scrcpy(&scr,&tmp);
			}
			break;
		}
		if (scrs[i].sc_uid == -1)	/* add at end of roll */
			scrcpy(&(scrs[i]), &newscr);
		if (rwscore(1, scrs))
			fprintf(stderr, "%s: cannot write score file\n", argv0);
	}
	check_scrs();
}

/*
** madeit()	- return true if the given score will make on the given roll
*/
static	int	madeit (l, scrs)
reg	long	l;
reg	score	*scrs;
{
	reg	int	i;

	if (!l || was_wiz)
		return(FALSE);
	for (i=0; i<MAX_SCORES; i++, scrs++)
		if (scrs->sc_uid == -1)
			return(TRUE);
		else if (l > scrs->sc_score)
			return(TRUE);
	return(FALSE);
}

/*
** pmers()	- called by main() to print the list of players
*/
void	pmers ()
{
#ifdef	PM_USER
	auto	char	buf[MAX_BYTES];
	reg	int	j;

	if (rwuser(0, buf))
	{
		perror(pm_user);
		exit(1);
	}
	for (j = 0; j < MAX_BYTES; j++)
	{
		reg	int	i;

		if (!buf[j])
			continue;
		for (i = 0; i < BYTE_SIZE; i++)
		{
			auto	int	puid;
			struct	passwd	*pw;
			extern	struct passwd *getpwuid();

			if (!((char) (1 << i) & buf[j]))
				continue;
			if (pw = getpwuid(puid = ((j * BYTE_SIZE) + i)))
				printf("%s\n", pw->pw_name);
			else
				fprintf(stderr, "%s: getpwuid(%d) error\n",
					argv0, puid);
		}
	}
#else
	printf("%s: there is no pm user file\n", argv0);
#endif
}

/*
** rwscore()	- read/write the contents of the pm_roll file
**		- if flag is true, write, else read
**		- return non-zero on error
*/
static	int	rwscore (flag, scrs)
reg	int	flag;
reg	score	*scrs;
{
	reg	FILE	*fp;
	reg	int	(*func)();

	if (!(fp = fopen(pm_roll, flag ? "w" : "r")))
		return(TRUE);
	func = flag ? fwrite : fread;
	if ((*func)(scrs, sizeof(*scrs), MAX_SCORES, fp) != MAX_SCORES)
	{
		perror(pm_roll);
		fclose(fp);
		return(TRUE);
	}
	fclose(fp);
	return(FALSE);
}

#ifdef	PM_USER
/*
** rwuser()	- read/write the contents of the pm_user file
**		- if flag is true, write, else read
**		- return non-zero on error
*/
static	int	rwuser (flag, buf)
reg	int	flag;
reg	char	*buf;
{
	reg	FILE	*fp;
	reg	int	(*func)();

	if (!(fp = fopen(pm_user, flag ? "w" : "r")))
		return(TRUE);
	func = flag ? fwrite : fread;
	if ((*func)(buf, sizeof(*buf), MAX_BYTES, fp) != MAX_BYTES)
	{
		perror(pm_user);
		fclose(fp);
		return(TRUE);
	}
	fclose(fp);
	return(FALSE);
}
#endif
