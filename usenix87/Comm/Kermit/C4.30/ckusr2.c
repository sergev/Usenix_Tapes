/*  C K U S R 2  --  "User Interface" STRINGS module for Unix Kermit  */

/*  This module separates long strings from the body of the ckuser module. */  

#include "ckermi.h"
#include "ckcmd.h"
#include "ckuser.h"

extern char cmdbuf[];
extern int nrmt, nprm, dfloc;
extern char *dftty;
extern struct keytab prmtab[];
extern struct keytab remcmd[];

static
char *hlp1[] = {
"\n",
"  Usage: kermit [-x arg [-x arg]...[-yyy]..]]\n",
"   x is an option that requires an argument, y an option with no argument:\n",
"     actions (* options also require -l and -b) --\n",
"       -s file(s)   send (use '-s -' to send from stdin)\n",
"       -r           receive\n",
"       -k           receive to stdout\n",
"     * -g file(s)   get remote file(s) from server (quote wildcards)\n",
"       -a name      alternate name, used with -s, -r, -g\n",
"       -x           enter server mode\n",
"     * -f           finish remote server\n",
"     * -c           connect before transaction\n",
"     * -n           connect after transaction\n",
"       -h           help - print this message\n",
"     settings --\n",
"       -l line      communication line device\n",
"       -b baud      line speed, e.g. 1200\n",
"       -i           binary file or Unix-to-Unix\n",
"       -p x         parity, x is one of e,o,m,s,n\n",
"       -t           line turnaround handshake = xon, half duplex\n",
"       -w           don't write over preexisting files\n",
"       -q    	     be quiet during file transfer\n",
"       -d   	     log debugging info to debug.log\n",
" If no action command is included, enter interactive dialog.\n",
""
};

/*  U S A G E */

usage() {
    conola(hlp1);
}

/*  Help string definitions  */

static char *tophlp[] = { "\n\
Type ? for a list of commands, type 'help x' for any command x.\n\
While typing commands, use the following special characters:\n\n\
 DEL, RUBOUT, BACKSPACE, CTRL-H: Delete the most recent character typed.\n\
 CTRL-W: Delete the most recent word typed.\n",

"\
 CTRL-U: Delete the current line.\n\
 CTRL-R: Redisplay the current line.\n\
 ?       (question mark) display help on the current command or field.\n\
 ESC     (Escape or Altmode) Attempt to complete the current field.\n",

"\
 \\       (backslash) include the following character literally.\n\n\
From Unix command level, type 'kermit -h' to get help about command line args.\
\n",
"" };

static char *hmxxbye = "\
Shut down and log out a remote Kermit server";

static char *hmxxclo = "\
Close one of the following logs:\n\
 session, transaction, packet, debugging -- 'help log' for further info.";

static char *hmxxcon = "\
Connect to a remote system via the tty device given in the\n\
most recent 'set line' command";

static char *hmxxget = "\
Format: 'get filespec'.  Tell the remote Kermit server to send the named\n\
files.  If filespec is omitted, then you are prompted for the remote and\n\
local filenames separately.";

static char *hmxxlg[] = { "\
Record information in a log file:\n\n\
 debugging             Debugging information, to help track down\n\
  (default debug.log)  bugs in the C-Kermit program.\n\n\
 packets               Kermit packets, to help track down protocol problems.\n\
  (packet.log)\n\n",

" session               Terminal session, during CONNECT command.\n\
  (session.log)\n\n\
 transactions          Names and statistics about files transferred.\n\
  (transaction.log)\n",
"" } ;

static char *hmxxlogi[] = { "\
Syntax: script text\n\n",
"Login to a remote system using the text provided.  The login script\n",
"is intended to operate similarly to uucp \"L.sys\" entries.\n",
"A login script is a sequence of the form:\n\n",
"	expect send [expect send] . . .\n\n",
"where 'expect' is a prompt or message to be issued by the remote site, and\n",
"'send' is the names, numbers, etc, to return.  The send may also be the\n",
"keyword EOT, to send control-d, or BREAK, to send a break.  Letters in\n",
"send may be prefixed by ~ to send special characters.  These are:\n",
"~b backspace, ~s space, ~q '?', ~n linefeed, ~r return, ~c don\'t\n",
"append a return, and ~o[o[o]] for octal of a character.  As with some \n",
"uucp systems, sent strings are followed by ~r unless they end with ~c.\n\n",
"Only the last 7 characters in each expect are matched.  A null expect,\n",
"e.g. ~0 or two adjacent dashes, causes a short delay.  If you expect\n",
"that a sequence might not arrive, as with uucp, conditional sequences\n",
"may be expressed in the form:\n\n",
"	-send-expect[-send-expect[...]]\n\n",
"where dashed sequences are followed as long as previous expects fail.\n",
"" };

static char *hmxxrc[] = { "\
Format: 'receive [filespec]'.  Wait for a file to arrive from the other\n\
Kermit, which must be given a 'send' command.  If the optional filespec is\n",

"given, the (first) incoming file will be stored under that name, otherwise\n\
it will be stored under the name it arrives with.",
"" } ;

static char *hmxxsen = "\
Format: 'send file1 [file2]'.  File1 may contain wildcard characters '*' or\n\
'?'.  If no wildcards, then file2 may be used to specify the name file1 is\n\
sent under; if file2 is omitted, file1 is sent under its own name.";

static char *hmxxser = "\
Enter server mode on the currently selected line.  All further commands\n\
will be taken in packet form from the other Kermit program.";

static char *hmhset = "\
The 'set' command is used to establish various communication or file\n\
parameters.  The 'show' command can be used to display the values of\n\
'set' parameters.  Help is available for each individual parameter;\n\
type 'help set ?' to see what's available.\n";

static char *hmxychkt = "\
Type of packet block check to be used for error detection, 1, 2, or 3.\n\
Type 1 is standard, and catches most errors.  Types 2 and 3 specify more\n\
rigorous checking, at the cost of higher overhead.  Not all Kermit programs\n\
support types 2 and 3.\n";

static char *hmxyf[] = { "\
set file: names, type, warning, display.\n\n\
'names' are normally 'converted', which means file names are converted\n\
to 'common form' during transmission; 'literal' means use filenames\n\
literally (useful between like Unix systems).\n",

"\n\
'type' is normally 'text', in which conversion is done between Unix newlines\n\
and CRLF line delimiters; 'binary' means to do no conversion.  Use 'binary'\n\
for executable programs or binary data.\n",

"\n\
'warning' is 'on' or 'off', normally off.  When off, incoming files will\n\
overwrite existing files of the same name.  When on, new names will be\n\
given to incoming files whose names are the same as existing files.\n" ,

"\n\
'display' is normally 'on', causing file transfer progress to be displayed\n\
on your screen when in local mode.  'set display off' is useful for allowing\n\
file transfers to proceed in the background.\n\n",
"" } ;

static char *hmhrmt[] = { "\
The 'remote' command is used to send file management instructions to a\n\
remote Kermit server.  There should already be a Kermit running in server\n\
mode on the other end of the currently selected line.  Type 'remote ?' to\n",

"\
see a list of available remote commands.  Type 'help remote x' to get\n\
further information about a particular remote command 'x'.\n" ,
""} ;

/*  D O H L P  --  Give a help message  */

dohlp(xx) int xx; {
    int x,y;

    if (xx < 0) return(xx);
    switch (xx) {

case XXBYE:
    return(hmsg(hmxxbye));

case XXCLO:
    return(hmsg(hmxxclo));

case XXCON:
    return(hmsg(hmxxcon));

case XXCWD:
    return(hmsg("Change Working Directory, equivalent to Unix 'cd' command"));

case XXDEL:
    return(hmsg("Delete a local file or files"));

case XXDIAL:
    return(hmsg("Dial a number using modem autodialer"));

case XXDIR:
    return(hmsg("Display a directory of local files"));

case XXECH:
    return(hmsg("Display the rest of the command on the terminal,\n\
useful in command files."));

case XXEXI:
case XXQUI:
    return(hmsg("Exit from the Kermit program, closing any open logs."));

case XXFIN:
    return(hmsg("\
Tell the remote Kermit server to shut down without logging out."));

case XXGET:
    return(hmsg(hmxxget));

case XXHLP:
    return(hmsga(tophlp));

case XXLOG:
    return(hmsga(hmxxlg));

case XXLOGI:
    return(hmsga(hmxxlogi));

case XXREC:
    return(hmsga(hmxxrc));

case XXREM:
    if ((y = cmkey(remcmd,nrmt,"Remote command","")) == -2) return(y);
    if (y == -1) return(y);
    if (x = (cmcfm()) < 0) return(x);
    return(dohrmt(y));

case XXSEN:
    return(hmsg(hmxxsen));

case XXSER:
    return(hmsg(hmxxser));

case XXSET:
    if ((y = cmkey(prmtab,nprm,"Parameter","")) == -2) return(y);
    if (y == -2) return(y);
    if (x = (cmcfm()) < 0) return(x);
    return(dohset(y));

case XXSHE:
    return(hmsg("\
Issue a command to the Unix shell (space required after '!')"));

case XXSHO:
    return(hmsg("\
Display current values of 'set' parameters; 'show version' will display\n\
program version information for each of the C-Kermit modules."));

case XXSPA:
    return(hmsg("Display disk usage in current device, directory"));

case XXSTA:
    return(hmsg("Display statistics about most recent file transfer"));

case XXTAK:
    return(hmsg("\
Take Kermit commands from the named file.  Kermit command files may\n\
themselves contain 'take' commands, up to a reasonable depth of nesting."));

default:
    if (x = (cmcfm()) < 0) return(x);
    printf("Not available yet - %s\n",cmdbuf);
    break;
    }
    return(0);
}

/*  H M S G  --  Get confirmation, then print the given message  */

hmsg(s) char *s; {
    int x;
    if (x = (cmcfm()) < 0) return(x);
    printf("%s\n",s);
    return(0);
}

hmsga(s) char *s[]; {			/* Same function, but for arrays */
    int x, i;
    if ( x = (cmcfm()) < 0) return(x);
    for ( i = 0; *s[i] ; i++ ) fputs(s[i], stdout);
    fputc( '\n', stdout);
    return(0);
}

/*  D O H S E T  --  Give help for SET command  */

dohset(xx) int xx; {
    
    if (xx == -3) {
        printf("%s",hmhset);
    	return(0);
    }
    if (xx < 0) return(xx);
    switch (xx) {

case XYCHKT:
    printf("%s",hmxychkt);
    return(0);

case XYDELA: 
    printf("%s","\
Number of seconds to wait before sending first packet after 'send' command\n");
    return(0);

case XYDUPL:
    printf("%s","\
During 'connect': 'full' means remote host echoes, 'half' means this program\n\
does its own echoing.\n");
    return(0);

case XYEOL:
    printf("%s","\
Decimal ASCII value for character to terminate outbound packets, normally\n\
13 (CR, carriage return).  Inbound packets are assumed to end with CR.\n");
    return(0);

case XYESC:
    printf("%s","\
Decimal ASCII value for escape character during 'connect', normally 28\n\
(Control-\\)\n");
    return(0);

case XYFILE:
    printf("%s",hmxyf[0]);
    printf("%s",hmxyf[1]);
    printf("%s",hmxyf[2]);
    printf("%s",hmxyf[3]);
    return(0);

case XYFLOW:
    printf("%s","\
Type of flow control to be used.  Choices are 'xon/xoff' and 'none'.\n\
normally xon/xoff.\n");
    return(0);

case XYHAND:
    printf("%s","\
Decimal ASCII value for character to use for half duplex line turnaround\n\
handshake.  Normally, handshaking is not done.\n");
    return(0);

case XYLEN:
    printf("%s","\
Packet length to use.  90 by default.  94 maximum.\n");
    return(0);

case XYLINE:
    printf("Device name of communication line to use.  Normally %s.\n",dftty);
    if (!dfloc) {
	printf("If you set the line to other than %s, then Kermit\n",dftty);
	printf("%s","\
will be in 'local' mode; 'set line' will reset Kermit to remote mode.\n");
    printf("%s","\
If the line has a modem, and if the modem-dialer is set to direct, this \n\
command causes waiting for a carrier detect (e.g., on a hayes type modem). \n\
This can be used to wait for incomming calls.  \n");
    printf("%s","\
To use the modem to dial out, first set modem-dialer (e.g., to hayes), then \n\
set line, next issue the dial command, and finally connect \n");
    }
    return(0);

case XYMARK:
    printf("%s","\
Decimal ASCII value of character that marks the beginning of packets sent by\n\
this program (normally 1 = Control-A)\n");
    return(0);

case XYMODM:
    printf("%s","\
Type of modem for dialing remote connections.  Needed to indicate modem can\n\
be commanded to dial without \"carrier detect\" from modem.  Many recently\n\
manufactured modems use \"hayes\" protocol.\n");
    return(0);

case XYNPAD:
    printf("%s","\
Number of padding characters to request for inbound packets, normally 0.\n");
    return(0);

case XYPADC:
    printf("%s","Decimal ASCII value of inbound padding character, normally 0.\n");
    return(0);

case XYPARI:
    printf("%s","\
Parity to use during terminal connection and file transfer:\n\
even, odd, mark, space, or none.  Normally none.\n");
    return(0);

case XYPROM:
    printf("%s","Prompt string for this program, normally 'C-Kermit>'.\n");
    return(0);

case XYSPEE:
    printf("%s","\
Communication line speed for external tty line specified in most recent\n\
'set line'.  Any of the common baud rates: 0, 110, 150, 300, 600, 1200,\n\
1800, 2400, 4800, 9600.\n");
    return(0);

case XYTIMO:
    printf("%s","\
Timeout interval for this program to use during file transfer, seconds.\n");
    return(0);

default:
    printf("Not available yet - %s\n",cmdbuf);
    return(0);
    }
}

/*  D O H R M T  --  Give help about REMOTE command  */

dohrmt(xx) int xx; {
    int x;
    if (xx == -3) {
	printf("%s",hmhrmt[0]);
	printf("%s",hmhrmt[1]);
    	return(0);
    }
    if (xx < 0) return(xx);
    switch (xx) {

case XZCWD:
    return(hmsg("\
Ask remote Kermit server to change its working directory."));

case XZDEL:
    return(hmsg("\
Ask remote Kermit server to delete the named file(s)."));

case XZDIR:
    return(hmsg("\
Ask remote Kermit server to provide directory listing of the named file(s)."));

case XZHLP:
    return(hmsg("\
Ask remote Kermit server to tell you what services it provides."));

case XZHOS:
    return(hmsg("\
Send a command to the remote system in its own command language\n\
through the remote Kermit server."));

case XZSPA:
    return(hmsg("\
Ask the remote Kermit server to tell you about its disk space."));

case XZTYP:
    return(hmsg("\
Ask the remote Kermit server to type the named file(s) on your screen."));

case XZWHO:
    return(hmsg("\
Ask the remote Kermit server to list who's logged in, or to give information\n\
about the specified user."));

default:
    if (x = (cmcfm()) < 0) return(x);
    printf("not working yet - %s\n",cmdbuf);
    return(-2);
    }
}
