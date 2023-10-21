/*
**	HASHMAX is in effect the number of newsgroups allowed.
**	HASHMAX MUST be less than HASHSIZE.
**	HASHSIZE should be prime - size of hash table newsgroups
**		are entered in, linear probe to resolve collisions
**		(HASHMAX / HASHSIZE = max. density).
*/
#define HASHMAX 500
#define HASHSIZE 809

#define DEF_ED "/usr/ucb/vi"	/* editor to use if no EDITOR variable */
#define DEF_PS1 "$ "		/* ! command prompt if no PS1 */
#define DEF_SAVE "vn.save"	/* save file */
#define DEF_MAIL "/bin/mail"	/* mailer */

#define DEF_PRINT "/usr/ucb/lpr"		/* print command */
#define DEF_POST "/usr/lib/news/inews -h"	/* followup posting command */

#define DEF_NEWSRC ".newsrc"

#define SPOOLDIR "/usr/spool/news"
#define ACTFILE "/usr/lib/news/active"
