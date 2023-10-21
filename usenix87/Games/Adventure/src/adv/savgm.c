/*
**		    Copyright (c) 1985	Ken Wellsch
**
**     Permission is hereby granted to all users to possess, use, copy,
**     distribute, and modify the programs and files in this package
**     provided it is not for direct commercial benefit and secondly,
**     that this notice and all modification information be kept and
**     maintained in the package.
**
*/

#include "adefs.h"

#define READONLY	0
#define WRITEONLY	1
#define READWRITE	2

struct saveb
{
	int s_uid ;
	short int s_ov[OBJECTS], s_ob[OBJECTS], s_ol[OBJECTS] ;
	short int s_pb[PLACES] ;
	short int s_vv[VARS], s_vb[VARS] ;
} ;

struct saveb sb ;

int savgm ()
{
	register int i ;
	register int uid ;
	register int fd ;

	if ( ( fd = open (FREEZER,READWRITE) ) < 0 )
	{
		if ( ( fd = creat (FREEZER,0600) ) < 0 )
			return (ERROR) ;
		close (fd) ;
		if ( ( fd = open (FREEZER,READWRITE) ) < 0 )
			return (ERROR) ;
	}

	uid = getuid() + 1 ;

	forever
	{
		if ( read(fd,(char *)&sb,sizeof(sb)) != sizeof(sb) )
			break ;
		if ( sb.s_uid == uid || sb.s_uid == 0 )
		{
			(void) lseek (fd,-(sizeof(sb)),1) ;
			break ;
		}
	}

	sb.s_uid = uid ;

	for ( i = 0 ; i < OBJECTS ; i++ )
	{
		sb.s_ov[i] = objval[i] ;
		sb.s_ob[i] = objbit[i] ;
		sb.s_ol[i] = objloc[i] ;
	}

	for ( i = 0 ; i < PLACES ; i++ )
	{
		sb.s_pb[i] = plcbit[i] ;
	}

	for ( i = 0 ; i < VARS ; i++ )
	{
		sb.s_vv[i] = varval[i] ;
		sb.s_vb[i] = varbit[i] ;
	}

	if ( write (fd,(char *)&sb,sizeof(sb)) != sizeof(sb) )
	{
		(void) close(fd) ;
		return (ERROR) ;
	}
	(void) close(fd) ;
	return (OK) ;
}

int resgm ()
{
	register int i ;
	register int uid ;
	register int fd ;

	if ( ( fd = open (FREEZER,READONLY) ) < 0 )
		return (ERROR) ;

	uid = getuid() + 1 ;

	forever
	{
		if ( read(fd,(char *)&sb,sizeof(sb)) != sizeof(sb) )
		{
			(void) close (fd) ;
			return (ERROR) ;
		}
		if ( sb.s_uid == uid )
			break ;
	}

	for ( i = 0 ; i < OBJECTS ; i++ )
	{
		objval[i] = sb.s_ov[i] ;
		objbit[i] = sb.s_ob[i] ;
		objloc[i] = sb.s_ol[i] ;
	}

	for ( i = 0 ; i < PLACES ; i++ )
	{
		plcbit[i] = sb.s_pb[i] ;
	}

	for ( i = 0 ; i < VARS ; i++ )
	{
		varval[i] = sb.s_vv[i] ;
		varbit[i] = sb.s_vb[i] ;
	}

	(void) close(fd) ;
	return (OK) ;
}

int delgm ()
{
	register int uid ;
	register int fd ;

	if ( ( fd = open (FREEZER,READWRITE) ) < 1 )
		return (ERROR) ;

	uid = getuid() + 1 ;

	forever
	{
		if ( read(fd,(char *)&sb,sizeof(sb)) != sizeof(sb) )
		{
			(void) close (fd) ;
			return (ERROR) ;
		}
		if ( sb.s_uid == uid )
			break ;
	}

	sb.s_uid = 0 ;
	(void) lseek (fd,-(sizeof(sb)),1) ;
	if ( write (fd,(char *)&sb,sizeof(sb)) != sizeof(sb) )
	{
		(void) close(fd) ;
		return (ERROR) ;
	}
	(void) close(fd) ;
	return (OK) ;
}
