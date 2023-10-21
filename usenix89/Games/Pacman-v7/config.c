/*
** config.c -	installation dependent parameters
**
**	PM_ROLL:	full pathname of score file
**
**	PM_USER:	full pathname of user log file.  used to keep
**			track of who has played
**
**	WIZARD_UID:	if argv[0] == "tester" and getuid() = WIZARD_UID
**			then game runs in diagnostic mode where special
**			commands take effect
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/
char	*pm_roll = PM_ROLL,	/* score file				*/
#ifdef	PM_USER
	*pm_user = PM_USER;	/* user file				*/
#else
	*pm_user = NULL;	/* no user file				*/
#endif
#ifdef	NOFULLPATH
int	wizard_uid = WIZARD_UID;
#endif
