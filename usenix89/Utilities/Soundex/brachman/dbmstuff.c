/* dbmstuff.c */

/*
 * Interface to old and new dbm routines
 */

#include <stdio.h>

#ifndef NEWDBM

#include <dbm.h>

/*ARGSUSED*/
DBMINIT(path, flags)
char *path;
int flags;
{

	return(dbminit(path));
}

DBMCLOSE()
{

#ifdef HAS_CLOSE
	dbmclose();
#else
	close(3);		/* free up the file descriptors */
	close(4);
#endif
}

datum
FETCH(key)
datum key;
{
	datum fetch();

	return(fetch(key));
}

datum
FIRSTKEY()
{

	return(firstkey());
}

datum
NEXTKEY(key)
datum key;
{
	return(nextkey(key));
}

STORE(key, content)
datum key, content;
{

	return(store(key, content));
}

REPLACE(key, content)
datum key, content;
{

	if (delete(key) == -1)
		return(-1);
	return(store(key, content));
}

DELETE(key)
datum key;
{

	return(delete(key));
}

#endif !NEWDBM

#ifdef NEWDBM

#include <ndbm.h>

static DBM *current_db = (DBM *) NULL;

DBMINIT(path, flags)
char *path;
int flags;
{

	current_db = dbm_open(path, flags, 0);
	if (current_db == (DBM *) NULL)
		return(-1);
	return(0);
}

DBMCLOSE()
{

	if (current_db != (DBM *) NULL) {
		dbm_close(current_db);
		current_db = (DBM *) NULL;
	}
}

datum
FETCH(key)
datum key;
{

	return(dbm_fetch(current_db, key));
}

datum
FIRSTKEY()
{

	return(dbm_firstkey(current_db));
}

/*ARGSUSED*/
datum
NEXTKEY(key)
datum key;
{

	return(dbm_nextkey(current_db));
}

REPLACE(key, content)
datum key, content;
{

	return(dbm_store(current_db, key, content, DBM_REPLACE)); 
}

STORE(key, content)
datum key, content;
{

	return(dbm_store(current_db, key, content, DBM_INSERT));
}

DELETE(key)
datum key;
{

	return(dbm_delete(current_db, key));
}

#endif NEWDBM
