/*
 *	@(#)udl.c	1.1 (TDI) 7/18/86 21:02:01
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:02:01 by brandon
 *     Converted to SCCS 07/18/86 21:02:01
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:02:01 by brandon
 *     Converted to SCCS 07/18/86 21:02:01
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:02:01 by brandon
 *     Converted to SCCS 07/18/86 21:02:01
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)udl.c	1.1 (TDI) 7/18/86 21:02:01";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

#include "ua.h"
#ifdef BSD
#include <sys/time.h>
#else  SYS3
#include <time.h>
#endif BSD
#include <varargs.h>

#define A_UPLOAD	parms.ua_auc
#define A_DOWNLOAD	parms.ua_adc

#define X_UPLOAD	parms.ua_xuc
#define X_DOWNLOAD	parms.ua_xdc

#define K_UPLOAD	parms.ua_kuc
#define K_DOWNLOAD	parms.ua_kdc

#define LIBRARY		"library"
#define DIRECTORY	"directory"
#define STORAGE		"uploads"
#define UPLOG		"upload-log"

#define off		0
#define on		1

#define pager		__col = 0, __line = 0, __Page =

#define DIRFORMAT	"%[^ ] %*1[Ff]%*1[Ii]%*1[Ll]%*1[Ee] %[^;]; %[^ ] %*1[Bb]%*1[Yy] %[^:]: %[^\n]"

jmp_buf brchloop, fileloop;
static char thisbrch[80] = "";

int __Page = on;
int __line = 0;
int __col = 0;
int __paws = off;

char *whatis(), *pgetin(), *cpmform(), *upstr(), *today();

extern struct tm *localtime();

udl(branch)
char *branch; {
	int again();
	int (*oldint)();
	char *cp;

	if (user.u_access < A_FILES) {
		puts("\nYou will not be able to upload or download files.\nYou may, however, download File Lists.");
		log("File Section entered; access restricted");
	}
	thisbrch[0] == '\0';
	for (cp = branch; *cp != '\0'; cp++)
		if (*cp == ' ') {
			strcpy(thisbrch, ++cp);
			break;
		}
	oldint = signal(SIGINT, again);
	setjmp(brchloop);
	while (libmenu())
		;
	msg("\n");
	signal(SIGINT, oldint);
}

again() {
	signal(SIGINT, again);
	pager on;
	msg("\nInterrupt\n");
	fflush(stdout);
	log("Interrupt");
	if (__paws == on) {
		talk();
		__paws = off;
	}
	longjmp(brchloop, 1);
}

libmenu() {
	struct direct *branch;
	DIR *library;
	char cmd[512], bname[512];
	int (*oldsig)();
	int brch_cmd();
	char *desc;
	
	if ((library = opendir(LIBRARY)) == NULL) {
		msg("The Library is closed\n");
		return 0;
	}
	if (thisbrch[0] != '\0') {
		strcpy(cmd, thisbrch);
		thisbrch[0] == '\0';
		goto gotbrch;
	}
	msg("\nThe UNaXcess File Section.  Please select one of the following\nbranches, or EXIT to leave the Library.\n\n");
	while ((branch = readdir(library)) != NULL) {
		if ((desc = whatis(branch->d_name, NULL)) == NULL)
			continue;
		msg("    %-8.8s   %s\n", upstr(branch->d_name), desc);
	}
	closedir(library);
	msg("\nBranch: ");
	if (pgetin(cmd, NULL) == NULL)
		return 0;
	log("Branch: %s", cmd);
	if (s_cmp(cmd, "EXIT") == 0)
		return 0;

gotbrch:
	if (whatis(cmd, NULL) != NULL) {
		library = opendir(LIBRARY);
		while ((branch = readdir(library)) != NULL)
			if (s_cmp(branch->d_name, cmd) == 0) {
				closedir(library);
				strcpy(bname, branch->d_name);
				oldsig = signal(SIGINT, brch_cmd);
				setjmp(fileloop);
				while (visit(bname))
					;
				signal(SIGINT, oldsig);
				return 1;
			}
	}
	closedir(library);
	msg("There is no such branch.  If you wish to open a new branch,\nleave a message to %s.\n", parms.ua_sysop);
	return 1;
}

visit(branch)
char *branch; {
	char cmd[512];
	DIR *directory;
	
	sprintf(cmd, "%s/%s", LIBRARY, branch);
	if ((directory = opendir(cmd)) == NULL) {
		msg("The %s branch is closed.\n", upstr(branch));
		return 0;
	}
	closedir(directory);
	msg("\n%s Branch\nUpload, Download, List of Files, Get File List, Exit: ", upstr(branch));
	if (pgetin(cmd, NULL) == NULL)
		return 0;
	log("Branch cmd: %s", cmd);
	switch (cmd[0]) {
		case 'e':
		case 'E':
			return 0;
		case 'U':
		case 'u':
			upload(branch);
			break;
		case 'D':
		case 'd':
			download(branch);
			break;
		case 'L':
		case 'l':
			filelist(branch);
			break;
		case 'G':
		case 'g':
			getlist(branch);
			break;
		default:
			msg("Unrecognized command.\n");
	}
	return 1;
}

brch_cmd() {
	pager on;
	msg("\nInterrupt\n");
	log("Interrupt");
	fflush(stdout);
	if (__paws == on) {
		talk();
		__paws = off;
	}
	signal(SIGINT, brch_cmd);
	longjmp(fileloop, 1);
}

filelist(branch)
char *branch; {
	char path[512];
	DIR *directory;
	struct direct *file;
	char *desc;

	sprintf(path, "%s/%s", LIBRARY, branch);
	directory = opendir(path);
	msg("\nFile Directory for the %s Branch:\n\n", upstr(branch));
	while ((file = readdir(directory)) != NULL) {
		if ((desc = whatis(branch, file->d_name)) == NULL)
			continue;
		msg("    %-12.12s   %s\n", cpmform(file->d_name), desc);
	}
	msg("\n");
	closedir(directory);
}

getlist(branch)
char *branch; {
	char path[512], listfile[30], cmd[512];
	DIR *directory;
	struct direct *file;
	int (*oldsig)();
	char *desc;
	FILE *list;

	sprintf(listfile, "/tmp/cli%05d", getpid());
	if ((list = fopen(listfile, "w")) == NULL) {
		msg("Can't open temporary list file???\n");
		exit(5);
	}
	msg("\nDownload file listing from the %s branch\n\nSupported transfer protocols are: Ascii, Xmodem, and Kermit.\n\nEnter File Transfer Protocol (XMODEM default): ", upstr(branch));
	if (pgetin(cmd, NULL) == NULL)
		return;
	log("List dnld mode: %s", cmd);
	switch (cmd[0]) {
		case 'A':
		case 'a':
			if (!validudl(A_DOWNLOAD)) {
				msg("\nAscii Download is not supported.\n");
				log("No Ascii");
				return;
			}
			sprintf(cmd, A_DOWNLOAD, listfile);
			break;
		case 'K':
		case 'k':
			if (!validudl(K_DOWNLOAD)) {
				msg("\nKermit Download is not supported.\n");
				log("No Kermit");
				return;
			}
			sprintf(cmd, K_DOWNLOAD, listfile);
			break;
		case 'X':
		case 'x':
			if (!validudl(X_DOWNLOAD)) {
				msg("\nXModem Download is not supported.\n");
				log("No Xmodem");
				return;
			}
			sprintf(cmd, X_DOWNLOAD, listfile);
			break;
		case '\0':
			cmd[0] = 'X';
			if (!validudl(X_DOWNLOAD)) {
				msg("\nXModem Download is not supported.\n");
				log("No Xmodem");
				return;
			}
			sprintf(cmd, X_DOWNLOAD, listfile);
			break;
		default:
			msg("Invalid protocol designation.\n");
			return;
	}
	sprintf(path, "%s/%s", LIBRARY, branch);
	directory = opendir(path);
	fprintf(list, "File Directory for the %s Branch:\r\n\r\n", upstr(branch));
	while ((file = readdir(directory)) != NULL) {
		if ((desc = whatis(branch, file->d_name)) == NULL)
			continue;
		fprintf(list, "    %-12.12s   %s\r\n", cpmform(file->d_name), desc);
	}
	fclose(list);
	closedir(directory);
	msg("You have 30 seconds to prepare for file transmission.\nPress BREAK to abort transmission.\n\n");
	fflush(stdout);
	sleep(30);
	oldsig = signal(SIGINT, SIG_IGN);
	system(cmd);
#ifdef SYS3
	system("stty echoe");
#endif SYS3
	signal(SIGINT, oldsig);
	unlink(listfile);
}

download(branch)
char *branch; {
	char path[512], filename[512], cmd[512];
	DIR *directory;
	struct direct *file;
	int (*oldsig)();
	
	if (user.u_access < A_FILES) {
		log("Attempted download, access denied.");
		puts("You may not download files.");
		return;
	}
	msg("\nDownload from the %s branch\n\nEnter file to download: ");
	if (pgetin(filename, NULL) == NULL)
		return;
	log("Dnld file: %s", filename);
	if (filename[0] == '.' || Index(filename, '/') != NULL) {
		msg("No such file: \"%s\"\n", upstr(filename));
		return;
	}
	if (whatis(branch, filename) != NULL) {
		sprintf(path, "%s/%s", LIBRARY, branch);
		directory = opendir(path);
		while ((file = readdir(directory)) != NULL) {
			if (s_cmp(file->d_name, filename) == 0) {
				closedir(directory);
				sprintf(path, "%s/%s/%s", LIBRARY, branch, file->d_name);
				msg("Supported transfer protocols are: Ascii, Xmodem, and Kermit.\n\nEnter File Transfer Protocol (XMODEM default): ", upstr(branch));
				if (pgetin(cmd, NULL) == NULL)
					return;
				switch (cmd[0]) {
					case 'A':
					case 'a':
						if (!validudl(A_DOWNLOAD)) {
							msg("\nAscii Download is not supported.\n");
							log("No Ascii");
							return;
						}
						sprintf(cmd, A_DOWNLOAD, path);
						break;
					case 'K':
					case 'k':
						if (!validudl(K_DOWNLOAD)) {
							msg("\nKermit Download is not supported.\n");
							log("No Kermit");
							return;
						}
						sprintf(cmd, K_DOWNLOAD, path);
						break;
					case 'X':
					case 'x':
						if (!validudl(X_DOWNLOAD)) {
							msg("\nXModem Download is not supported.\n");
							log("No Xmodem");
							return;
						}
						sprintf(cmd, X_DOWNLOAD, path);
						break;
					case '\0':
						cmd[0] = 'X';
						if (!validudl(X_DOWNLOAD)) {
							msg("\nXModem Download is not supported.\n");
							log("No Xmodem");
							return;
						}
						sprintf(cmd, X_DOWNLOAD, path);
						break;
					default:
						msg("Invalid protocol designation.\n");
						return;
				}
				msg("You have 30 seconds to prepare for file transmission.\nPress BREAK to abort transmission.\n\n");
				fflush(stdout);
				sleep(30);
				oldsig = signal(SIGINT, SIG_IGN);
				system(cmd);
#ifdef SYS3
				system("stty echoe");
#endif SYS3
				signal(SIGINT, oldsig);
				return;
			}
		}
		closedir(directory);
	}
	msg("No such file: \"%s\"\n", upstr(filename));
	log("No such file");
}

upload(branch)
char *branch; {
	char path[512], filename[512], cmd[512], desc[512];
	DIR *directory;
	struct direct *file;
	int (*oldsig)();
	FILE *logf;
	
	if (user.u_access < A_FILES) {
		log("Attempted upload, access denied.");
		puts("You may not upload files.");
		return;
	}
	msg("\nUpload to the %s branch\n\nEnter the name to give the new file: ");
	if (pgetin(filename, NULL) == NULL)
		return;
	log("Upld file: %s", filename);
	if (filename[0] == '.' || Index(filename, '/') != NULL || Index(filename, ';') != NULL) {
		msg("Invalid filename: \"%s\"\n", upstr(filename));
		log("Invalid filename");
		return;
	}
	sprintf(path, "%s/%s", STORAGE, branch);
	if ((directory = opendir(path)) == NULL) {
		msg("The %s has denied upload ability for this branch.\n", parms.ua_sysop);
		return;
	}
	while ((file = readdir(directory)) != NULL) {
		if (s_cmp(file->d_name, filename) == 0) {
			closedir(directory);
			msg("That file name is used.  Please try again with a different filename.\n");
			log("File exists");
			return;
		}
	}
	closedir(directory);
	msg("Enter a description for the file: ");
	if (pgetin(desc, NULL) == NULL)
		return;
	log("Description: %s", desc);
	if ((logf = fopen(UPLOG, "a")) == NULL) {
		log("Error %d opening %s", errno, UPLOG);
		msg("Can't log the new file.\n");
		return;
	}
	fprintf(logf, "%s file %s; %s by %s: %s\n", branch, filename, today(), upstr(user.u_name), desc);
	fclose(logf);
	sprintf(path, "%s/%s/%s", STORAGE, branch, filename);
	msg("Supported transfer protocols are: Ascii, Xmodem, and Kermit.\nXmodem protocol uses checksums; CCITT CRC is not supported.\n\nEnter File Transfer Protocol (XMODEM default): ", upstr(branch));
	if (pgetin(cmd, NULL) == NULL)
		return;
	log("Upld protocol: %s", cmd);
	switch (cmd[0]) {
		case 'A':
		case 'a':
			if (!validudl(A_UPLOAD)) {
				msg("\nAscii Upload is not supported.\n");
				log("No Ascii");
				return;
			}
			sprintf(cmd, A_UPLOAD, path);
			break;
		case 'K':
		case 'k':
			if (!validudl(K_UPLOAD)) {
				msg("\nKermit Upload is not supported.\n");
				log("No Kermit");
				return;
			}
			sprintf(cmd, K_UPLOAD, path);
			break;
		case 'X':
		case 'x':
			if (!validudl(X_UPLOAD)) {
				msg("\nXModem Upload is not supported.\n");
				log("No Xmodem");
				return;
			}
			sprintf(cmd, X_UPLOAD, path);
			break;
		case '\0':
			cmd[0] = 'X';
			if (!validudl(X_UPLOAD)) {
				msg("\nXModem Upload is not supported.\n");
				log("No Xmodem");
				return;
			}
			sprintf(cmd, X_UPLOAD, path);
			break;
		default:
			msg("Invalid protocol designation.\n");
			return;
	}
	msg("You have 30 seconds to prepare for file transmission.\nPress BREAK to abort transmission.\n\n");
	fflush(stdout);
	sleep(30);
	oldsig = signal(SIGINT, SIG_IGN);
	system(cmd);
#ifdef SYS3
	system("stty echoe");
#endif SYS3
	signal(SIGINT, oldsig);
}

/*VARARGS2*/
msg(fmt, va_alist)
char *fmt;
va_dcl {
	va_list va_ptr;
	register char *cp;
	int lzflag, width, ext, longflag, ljflag;
	
	va_start(va_ptr);
	for (cp = fmt; *cp != '\0'; cp++)
		if (*cp != '%')
			say(*cp);
		else {
			width = 0;
			ext = 0;
			lzflag = 0;
			longflag = 0;
			ljflag = 0;
			if (*++cp == '-')
				ljflag = 1;
			else
				cp--;
			if (isdigit(*++cp)) {
				if (*cp == '0') {
					lzflag = 1;
					cp++;
				}
				while (isdigit(*cp))
					width *= 10 + (*cp++ - '0');
			}
			if (*cp == '.') {
				while (isdigit(*++cp))
					ext *= 10 + (*cp - '0');
			}
_longagain:
			switch (*cp) {
				case 's':
					_fmtstr(va_arg(va_ptr, char *), width, ext, ljflag);
					break;
				case 'D':
					_fmtint(va_arg(va_ptr, long), width, lzflag, 1, ljflag);
					break;
				case 'd':
					_fmtint((long) va_arg(va_ptr, int), width, lzflag, longflag, ljflag);
					break;
				case 'c':
					_fmtchr(va_arg(va_ptr, char), width, lzflag);
					break;
				case 'l':
					longflag = 1;
					goto _longagain;
				default:
					say(*cp);
			}
		}
}

_fmtchr(c, width, ljflag)
char c; {
	register int w;
	
	if (!ljflag)
		for (w = 0; w < width - 1; w++)
			say(' ');
	say(c);
	if (ljflag)
		for (w = 0; w < width - 1; w++)
			say(' ');
}

_fmtstr(sp, width, ext, ljflag)
char *sp; {
	register char *cp;
	register int w;
	
	cp = sp;
	if (ext == 0)
		ext = strlen(cp);
	if (!ljflag)
		for (w = 0; w < width - ext; w++)
			say(' ');
	for (w = 0; w < ext; w++)
		say(*cp++);
	if (ljflag)
		for (w = 0; w < width - ext; w++)
			say(' ');
}

_fmtint(i, width, lzflag, longflag, ljflag)
long i; {
	long lnum;
	register int w;
	char *cp;
	char ibuf[20];
	int nospflag, nwidth;
	
	lnum = i;
	nospflag = (width == 0);
	if (width > sizeof ibuf - 2)
		width = sizeof ibuf - 2;
	cp = ibuf + sizeof ibuf - 1;
	*cp-- = '\0';
	if (lnum == 0)
		*cp-- = '0';
	else
		while (lnum != 0) {
			*cp-- = (lnum % 10) + '0';
			lnum /= 10;
		}
	if (!nospflag || (!lzflag && ljflag))
		while (cp >= ibuf + (sizeof ibuf - width - 1))
			*cp-- = (lzflag? '0': ' ');
	nwidth = ((long) cp) - ((long) ibuf);
	while (*++cp != '\0')
		say(*cp);
	if (ljflag)
		for (w = 0; w < width - nwidth; w++)
			say(' ');
}

say(ch)
char ch; {
	ch &= 0x7f;
	switch (ch) {
		case '\t':
			do {
				say(' ');
			} while (__col % 8);
			break;
		case '\n':
			putchar('\n');
			__col = 0;
			if (__Page == on && ++__line == 23) {
				paws();
				__line = 0;
			}
			break;
		case '\177':
			msg("^?");
			break;
		default:
			if (ch < ' ') {
				say('^');
				say(ch + '@');
				return;
			}
			if (__col == user.u_llen - 1)
				say('\n');
			putchar(ch);
			__col++;
	}
}

paws() {
	fputs("Type any key to continue. . .", stdout);
	__paws = on;
	silent();
	getchar();
	talk();
	__paws = off;
	fputs("\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b", stdout);
}

s_cmp(s1, s2)
char *s1, *s2; {
	for (; *s1 != '\0' && ToLower(*s1) == ToLower(*s2); s1++, s2++)
		;
	return (!(*s1 == '\0' && *s2 == '\0'));
}

char *whatis(branch, file)
char *branch, *file; {
	static FILE *directory = NULL;
	static char dent[512];
	char tbr[512], tfi[512], date[512], who[512], desc[512];
	
	if (directory != NULL || (directory = fopen(DIRECTORY, "r")) != NULL) {
		fseek(directory, 0L, 0);
		while (fgets(dent, sizeof dent, directory) != NULL) {
			if (dent[0] == '%' || dent[0] == '\n')
				continue;
			tbr[0] = '\0';
			tfi[0] = '\0';
			date[0] = '\0';
			who[0] = '\0';
			desc[0] = '\0';
			if (sscanf(dent, DIRFORMAT, tbr, tfi, date, who, desc) != 5)
				continue;
			if (s_cmp(tbr, branch) == 0) {
				if (s_cmp(tfi, (file == NULL? "branch": file)) != 0)
					continue;
				sprintf(dent, "%s [Created %s by %s]", desc, date, who);
				return dent;
			}
		}
	}
	if (directory == NULL)
		log("No download directory");
	return NULL;
}

/* this is a stub-ified version of v1.0 COMND input module */

char *pgetin(str, tab)
char *str;
char tab[]; {
	char *ch;

	fflush(stdout);
	pager off;
	ch = gets(str);
	pager on;
	if (ch == NULL) {
		msg("^D\n");
		log("EOF");
		fflush(stdout);
	}
	return ch;
}

char *cpmform(fn)
char *fn; {
	static char buf[13];
	register int cnt, scnt;
	
	for (scnt = 0, cnt = 0; cnt < 8 && fn[cnt] != '.' && fn[cnt] != '\0'; cnt++, scnt++)
		buf[scnt] = ToUpper(fn[cnt]);
	while (scnt < 8)
		buf[scnt++] = ' ';
	buf[scnt++] = '.';
	while (fn[cnt] != '.' && fn[cnt] != '\0')
		cnt++;
	if (fn[cnt] == '.')
		cnt++;
	while (scnt < 12 && fn[cnt] != '\0') {
		buf[scnt++] = ToUpper(fn[cnt]);
		cnt++;
	}
	buf[scnt] = '\0';
	return buf;
}

char *upstr(s)
char *s; {
	static char sbuf[512];
	register char *cp, *dp;
	
	for (cp = s, dp = sbuf; *cp != '\0'; cp++, dp++)
		*dp = ToUpper(*cp);
	*dp = '\0';
	return sbuf;
}

char *today() {
	long now;
	struct tm *date;
	static char buf[11];
	
	time(&now);
	date = localtime(&now);
	sprintf(buf, "%d/%d/%d", date->tm_mon + 1, date->tm_mday, date->tm_year);
	return buf;
}

validudl(cmd)
char *cmd; {
	if (cmd[0] == '\0')
		return 0;
	if (Index(cmd, '%') != RIndex(cmd, '%'))
		return 0;
	if (Index(cmd, '%') == (char *) 0) {
		strcat(cmd, " %s");
		return 1;
	}
	if (*(Index(cmd, '%') + 1) != 's')
		return 0;
	return 1;
}
