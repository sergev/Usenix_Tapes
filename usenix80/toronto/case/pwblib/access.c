/*
 *      testaccess -- routines to determine of a given user is
 *              should be given access to the tty specified.
 */

/*
 *      ttybits -- get tty access bits
 *      assumes a file  ttyfile of 4 bytes per entry (see tlist for
 *      format); The bits specified are the same as in those of
 *      react;  The subroutine reads the file until it finds
 *      a tty name that matches and returns the bits
 */
ttybits(tty)
char tty; {
	int mode;
	int fd;
	struct tlist{
		int bits;
		char ttyname;
		char nl;} ttylst;

	fd = open(ttyfile,0);
	if( fd < 0 ) return(-1);
	while( read(fd,&ttylst, sizeof ttylst) > 0)
	if (ttylst.ttyname == tty ) goto out;
	close(fd);
	return(-1);    /* not aknown tty */
out:    ;
	close(fd);
	return(ttylst.bits);
}

/*
 *      userbits -- read access list for user bits
 *      assumes a file access-list of 1 word per entry
 *      The bits specified are the same as in those of
 *      react;  The subroutine reads the access list
 *      file the nth word of that file is the permissions for the
 *      user specified.
 */
userbits(uid){
	int mode;
	int fd;

	if( (fd = open(aclist,0) ) <0 ) return(-1);
	seek(fd,(uid&0377)*2,0);
	if( read(fd,&mode,2) < 0 ) { close(fd); return(-1);}
	close(fd);
	return(mode);
}
