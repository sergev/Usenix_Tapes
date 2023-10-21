/*
 *	@(#)fileudl.c	1.1 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)fileudl.c	1.1 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include "ua.h"
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

#define DIRFORMAT	"%[^ ] %*1[Ff]%*1[Ii]%*1[Ll]%*1[Ee] %[^;]; %[^ ] %*1[Bb]%*1[Yy] %[^:]: %[^\n]"

jmp_buf brchloop, fileloop;

char *whatis(), *cpmform(), *upstr(), *today();

extern struct tm *localtime();

udl() {
	int again();
	int (*oldint)();
	char *cp;

	if (user.u_access < A_FILES) {
		writes("\nYou will not be able to upload or download files.  You may, however, download File Lists.");
		log("File Section entered; access restricted");
	}
	oldint = signal(SIGINT, again);
	setjmp(brchloop);
	while (libmenu())
		;
	writef("\n");
	signal(SIGINT, oldint);
}

again() {
	signal(SIGINT, again);
	writef("\nInterrupt\n");
	log("Interrupt");
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
		writef("The Library is closed\n");
		return 0;
	}
	writef("\nThe UNaXcess File Section.  Please select one of the following\nbranches, or EXIT to leave the Library.\n\n");
	while ((branch = readdir(library)) != NULL) {
		if ((desc = whatis(branch->d_name, NULL)) == NULL)
			continue;
		writef("    %-8.8s   %s\n", upstr(branch->d_name), desc);
	}
	closedir(library);
	writef("\nBranch: ");
	reads(cmd);
	log("Branch: %s", cmd);
	if (cmd[0] == '\0' || s_cmp(cmd, "EXIT") == 0)
		return 0;
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
	writef("There is no such branch.  If you wish to open a new branch,\nleave a message to %s.\n", parms.ua_sysop);
	return 1;
}

visit(branch)
char *branch; {
	char cmd[512];
	DIR *directory;
	
	sprintf(cmd, "%s/%s", LIBRARY, branch);
	if ((directory = opendir(cmd)) == NULL) {
		writef("The %s branch is closed.\n", upstr(branch));
		return 0;
	}
	closedir(directory);
	writef("\n%s Branch\nUpload, Download, List of Files, Get File List, Exit: ", upstr(branch));
	cmd[0] = readc();
	log("Branch cmd: %c", cmd[0]);
	switch (cmd[0]) {
		case 'E':
			return 0;
		case 'U':
			upload(branch);
			break;
		case 'D':
			download(branch);
			break;
		case 'L':
			filelist(branch);
			break;
		case 'G':
			getlist(branch);
			break;
		default:
			writef("Unrecognized command.\n");
	}
	return 1;
}

brch_cmd() {
	writef("\nInterrupt\n");
	log("Interrupt");
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
	writef("\nFile Directory for the %s Branch:\n\n", upstr(branch));
	while ((file = readdir(directory)) != NULL) {
		if ((desc = whatis(branch, file->d_name)) == NULL)
			continue;
		writef("    %-12.12s   %s\n", cpmform(file->d_name), desc);
	}
	writef("\n");
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
		writef("Can't open temporary list file???\n");
		log("Error %d opening %s", errno,listfile);
		panic("gfl_temp");
	}
	writef("\nDownload file listing from the %s branch\n\nSupported transfer protocols are: Ascii, Xmodem, and Kermit.\n\nEnter File Transfer Protocol (XMODEM default): ", upstr(branch));
	reads(cmd);
	log("List dnld mode: %s", cmd);
	switch (cmd[0]) {
		case 'A':
		case 'a':
			if (!validudl(A_DOWNLOAD)) {
				writef("\nAscii Download is not supported.\n");
				log("No Ascii");
				return;
			}
			sprintf(cmd, A_DOWNLOAD, listfile);
			break;
		case 'K':
		case 'k':
			if (!validudl(K_DOWNLOAD)) {
				writef("\nKermit Download is not supported.\n");
				log("No Kermit");
				return;
			}
			sprintf(cmd, K_DOWNLOAD, listfile);
			break;
		case 'X':
		case 'x':
			if (!validudl(X_DOWNLOAD)) {
				writef("\nXModem Download is not supported.\n");
				log("No Xmodem");
				return;
			}
			sprintf(cmd, X_DOWNLOAD, listfile);
			break;
		case '\0':
			cmd[0] = 'X';
			if (!validudl(X_DOWNLOAD)) {
				writef("\nXModem Download is not supported.\n");
				log("No Xmodem");
				return;
			}
			sprintf(cmd, X_DOWNLOAD, listfile);
			break;
		default:
			writef("Invalid protocol designation.\n");
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
	writef("You have 30 seconds to prepare for file transmission.\nPress BREAK to abort transmission.\n\n");
	sleep(30);
	oldsig = signal(SIGINT, SIG_IGN);
	udlrun(cmd, (char *) 0);
#ifdef SYS3
	udlrun("stty", "echoe");
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
		writes("You may not download files.");
		return;
	}
	writef("\nDownload from the %s branch\n\nEnter file to download: ", branch);
	reads(filename);
	log("Dnld file: %s", filename);
	if (filename[0] == '.' || Index(filename, '/') != NULL) {
		writef("No such file: \"%s\"\n", upstr(filename));
		return;
	}
	if (whatis(branch, filename) != NULL) {
		sprintf(path, "%s/%s", LIBRARY, branch);
		directory = opendir(path);
		while ((file = readdir(directory)) != NULL) {
			if (s_cmp(file->d_name, filename) == 0) {
				closedir(directory);
				sprintf(path, "%s/%s/%s", LIBRARY, branch, file->d_name);
				writef("Supported transfer protocols are: Ascii, Xmodem, and Kermit.\n\nEnter File Transfer Protocol (XMODEM default): ", upstr(branch));
				reads(cmd);
				switch (cmd[0]) {
					case 'A':
					case 'a':
						if (!validudl(A_DOWNLOAD)) {
							writef("\nAscii Download is not supported.\n");
							log("No Ascii");
							return;
						}
						sprintf(cmd, A_DOWNLOAD, path);
						break;
					case 'K':
					case 'k':
						if (!validudl(K_DOWNLOAD)) {
							writef("\nKermit Download is not supported.\n");
							log("No Kermit");
							return;
						}
						sprintf(cmd, K_DOWNLOAD, path);
						break;
					case 'X':
					case 'x':
						if (!validudl(X_DOWNLOAD)) {
							writef("\nXModem Download is not supported.\n");
							log("No Xmodem");
							return;
						}
						sprintf(cmd, X_DOWNLOAD, path);
						break;
					case '\0':
						cmd[0] = 'X';
						if (!validudl(X_DOWNLOAD)) {
							writef("\nXModem Download is not supported.\n");
							log("No Xmodem");
							return;
						}
						sprintf(cmd, X_DOWNLOAD, path);
						break;
					default:
						writef("Invalid protocol designation.\n");
						return;
				}
				writef("You have 30 seconds to prepare for file transmission.\nPress BREAK to abort transmission.\n\n");
				sleep(30);
				oldsig = signal(SIGINT, SIG_IGN);
				udlrun(cmd, (char *) 0);
#ifdef SYS3
				udlrun("stty", "echoe");
#endif SYS3
				signal(SIGINT, oldsig);
				return;
			}
		}
		closedir(directory);
	}
	writef("No such file: \"%s\"\n", upstr(filename));
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
		writes("You may not upload files.");
		return;
	}
	writef("\nUpload to the %s branch\n\nEnter the name to give the new file: ", branch);
	reads(filename);
	log("Upld file: %s", filename);
	if (filename[0] == '.' || Index(filename, '/') != NULL || Index(filename, ';') != NULL) {
		writef("Invalid filename: \"%s\"\n", upstr(filename));
		log("Invalid filename");
		return;
	}
	sprintf(path, "%s/%s", STORAGE, branch);
	if ((directory = opendir(path)) == NULL) {
		writef("The %s has denied upload ability for this branch.\n", parms.ua_sysop);
		return;
	}
	while ((file = readdir(directory)) != NULL) {
		if (s_cmp(file->d_name, filename) == 0) {
			closedir(directory);
			writef("That file name is used.  Please try again with a different filename.\n");
			log("File exists");
			return;
		}
	}
	closedir(directory);
	writef("Enter a description for the file: ");
	reads(desc);
	log("Description: %s", desc);
	if ((logf = fopen(UPLOG, "a")) == NULL) {
		log("Error %d opening %s", errno, UPLOG);
		writef("Can't log the new file.\n");
		return;
	}
	fprintf(logf, "%s file %s; %s by %s: %s\n", branch, filename, today(), upstr(user.u_name), desc);
	fclose(logf);
	sprintf(path, "%s/%s/%s", STORAGE, branch, filename);
	writef("Supported transfer protocols are: Ascii, Xmodem, and Kermit.\nXmodem protocol uses checksums; CCITT CRC is not supported.\n\nEnter File Transfer Protocol (XMODEM default): ", upstr(branch));
	reads(cmd);
	log("Upld protocol: %s", cmd);
	switch (cmd[0]) {
		case 'A':
		case 'a':
			if (!validudl(A_UPLOAD)) {
				writef("\nAscii Upload is not supported.\n");
				log("No Ascii");
				return;
			}
			sprintf(cmd, A_UPLOAD, path);
			break;
		case 'K':
		case 'k':
			if (!validudl(K_UPLOAD)) {
				writef("\nKermit Upload is not supported.\n");
				log("No Kermit");
				return;
			}
			sprintf(cmd, K_UPLOAD, path);
			break;
		case 'X':
		case 'x':
			if (!validudl(X_UPLOAD)) {
				writef("\nXModem Upload is not supported.\n");
				log("No Xmodem");
				return;
			}
			sprintf(cmd, X_UPLOAD, path);
			break;
		case '\0':
			cmd[0] = 'X';
			if (!validudl(X_UPLOAD)) {
				writef("\nXModem Upload is not supported.\n");
				log("No Xmodem");
				return;
			}
			sprintf(cmd, X_UPLOAD, path);
			break;
		default:
			writef("Invalid protocol designation.\n");
			return;
	}
	writef("You have 30 seconds to prepare for file transmission.\nPress BREAK to abort transmission.\n\n");
	sleep(30);
	oldsig = signal(SIGINT, SIG_IGN);
	udlrun(cmd, (char *) 0);
#ifdef SYS3
	udlrun("stty", "echoe");
#endif SYS3
	signal(SIGINT, oldsig);
}

char *whatis(branch, file)
char *branch, *file; {
	static FILE *directory = NULL;
	static char dent[512];
	char tbr[512], tfi[512], fdate[512], who[512], desc[512];
	
	if (directory != NULL || (directory = fopen(DIRECTORY, "r")) != NULL) {
		fseek(directory, 0L, 0);
		while (fgets(dent, sizeof dent, directory) != NULL) {
			if (dent[0] == '%' || dent[0] == '\n')
				continue;
			tbr[0] = '\0';
			tfi[0] = '\0';
			fdate[0] = '\0';
			who[0] = '\0';
			desc[0] = '\0';
			if (sscanf(dent, DIRFORMAT, tbr, tfi, fdate, who, desc) != 5)
				continue;
			if (s_cmp(tbr, branch) == 0) {
				if (s_cmp(tfi, (file == NULL? "branch": file)) != 0)
					continue;
				sprintf(dent, "%s [Created %s by %s]", desc, fdate, who);
				return dent;
			}
		}
	}
	if (directory == NULL)
		log("No download directory");
	return NULL;
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
	while (scnt < 12)
		buf[scnt++] = ' ';
	buf[scnt] = '\0';
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

udlrun(cmd, arg)
char *cmd, *arg; {
	int pid, sig, status;

	switch (pid = fork()) {
	case -1:
		log("Error %d on fork for udlrun", errno);
		writes("The system is too busy; try again later.");
		return -1;
	case 0:
		for (sig = SIGINT; sig <= SIGTERM; sig++)
			signal(sig, SIG_DFL);
		setuid(getuid());
		run(cmd, arg);
		exit(-1);
	default:
		CRIT();
		io_off();
		for (sig = SIGIOT; sig <= SIGTERM; sig++)
			signal(sig, SIG_IGN);
		while (wait(&status) != pid)
			;
		if (status != 0) {
			log("Status from \"%s %s\": %d", cmd, arg, status);
			writes("Error executing UDL program");
		}
		for (sig = SIGIOT; sig <= SIGTERM; sig++)
			signal(sig, logsig);
		signal(SIGALRM, thatsall);
		io_on(0);
		NOCRIT();
	}
	return 1;
}
