/* Copyright (c) 1984 Chris Torek / University of Maryland */

/*
 * $Header: /usr/enee/chris/sys/spool/RCS/spool.h,v 1.1 83/12/10 01:06:09 chris Exp $
 *
 * Definitions for spooler
 *
 * $Log:	spool.h,v $
 * Revision 1.1  83/12/10  01:06:09  chris
 * Initial revision
 * 
 */

#define	DeSpoolDaemon	"/usr/lib/despool"	/* Name of despooler daemon.
						   It is run with the device
						   name as an argument. */
#define	DevFile		"/etc/spool_devices"	/* File which describes the
						   available devices */
#define	SpoolDirectory	"/usr/spool/%s"		/* Used with printf to make
						   directory name for a given
						   device */

/* The format for DevFile is a set of lines beginning with one of "device" or
   "exec".  "Device" specifies a list of device names; the first of these is
   the spool device, the second is the official name, and the remainder are
   aliases.  The official name is used to generate the spool directory name.
   The device name is passed to the despooler to be used as the standard
   output of the programs executed.  "Exec" specifies a program to execute,
   or the user may explicitly name a program.  (Security is assured by having
   the daemon carefully run all files as the spooling user, and all control
   files owned by root.  This will not work across machines, but....) */

/* This contains the in-core version of the stuff in DevFile */
struct SpoolDevice {
	char	*sd_dev;	/* "/dev/foo" */
	struct namelist {
		char		*nl_name;
		struct namelist	*nl_next;
	} sd_namelist;		/* official name, and aliases */
	struct proglist {
		char		*pr_shortname;
		char		*pr_longname;
		struct proglist	*pr_next;
	} *sd_proglist;		/* List of -<xyz> programs */
	int	sd_restricted;	/* True => disallow -p */
	int	sd_pagelen;	/* page length (lines) */
	int	sd_linelen;	/* line length (chars) */
	char	*sd_ff;		/* form feed sequence (if any) */
	char	*sd_header;	/* header-printer program */
	struct SpoolDevice *sd_next;
} *DevList;

struct SpoolDevice *FindDevice ();
