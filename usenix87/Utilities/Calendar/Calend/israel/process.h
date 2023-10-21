/* Copyright 1983 - University of Maryland */

int level;		/* level of recursion on included files '<' */
char tmplate[BUFSIZ];	/* temp file name template "/tmp/cal-<pid>-A,B ..." */
bool clean;		/* was '-c' (clean up) option given? */
bool i_opt;		/* was '-i' (ignore old messages) given? */
bool remonly;		/* '-r' (remind only) */
int cdate,cwday;	/* current date and weekday */
