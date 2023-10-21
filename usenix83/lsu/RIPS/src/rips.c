#
#include <stdio.h>
#include <math.h>
#include "/usr/sys/statbuffer.h"
#define MAXAUTHS 	8  	/*  Max number of authors	*/
#define MAXKEYS  	100	/* Max number of keywords	*/
#define SCRN_WIDTH  	70	/*  Width of CRT less 10	*/
#define SCRN_HT		24	/*  Height of CRT in lines	*/
#define CARD_WIDTH  	45	/*  Print width of a file card	*/
#define CARD_HT		18	/* Max print lines on a file card*/
#define DLEN  		128	/*  Max numb chars in file name	*/
#define YLEN		8	/* Len of year field		*/
#define KLEN  		64	/* Len of keyword		*/
#define TLEN 		512	/* Len of title			*/
#define JLEN  		256	/* Len of journal		*/
#define RLEN 		1024	/* Len of reference		*/
#define ALEN  		32	/* Len of author		*/
#define PLEN  		16	/* Len of pagination		*/
#define VLEN  		256	/* Len of vol, city, pub	*/
#define CLEN		128	/* Max length of sh command line */
#define SLEN		1024  	/* Maximum # chars in a search command line */
#define MAX_PAREN	20  	/* Maximum # of open or close parens */
			    	/* in a command line		 */
#define MAX_CONJ	100  	/* Maximum # of conjunctions in a	 */
 			    	/* command line.			 */
#define DIRSIZ		14	/* Max. chars in a directory field	*/
#define NUM_CTYS	27	/* Number of recognized countries	*/
#define CARD 		0	/* Output is a card		*/
#define PAGE		1	/*  Output is on a page		*/
#define KWS		2	/* Output keywords		*/
#define NOKWS		3	/* Don't output keywords	*/
#define LPR		4	/* Send out to line printer	*/
#define	TTY		5	/* Send out to tty		*/
#define RRFILE		6	/* Output to rep. req. file	*/
#define FATAL		7	/* Bomb out			*/
#define NONFATAL	8	/* Give it another go		*/
#define NORMAL		9	/* No pathology			*/
#define RR		10	/* Yes we're doing rep. req.	*/

/************************************************************************
 *									*
 *			R I P S  V 1.0					*
 *									*
 *	Reference Information Program for Scientists			*
 *									*
 *	S. Klyce                   April 5, 1981			*
 *									*
 *		REVISION	HISTORY					*
 *									*
 *	S.D.K.								*
 *	May 10, 1981.  First working version implemented.		*
 *									*
 *	S.D.K.								*
 *	May 14, 1981.  Added prefix capability and author search.       *
 *									*
 *	S.D.K.								*
 *	May 15, 1981.  Added automatic incorporation of keywords in   	*
 *			title to .kwl and auto search of title for	*
 *			presence of the keyword(s).			*
 *									*
 *	S.D.K.								*
 *	May 22, 1981.  Added routine to bombout after ed_append() to	*
 *			prevent scrambling file while system does its	*
 *			refsort & kwsort routines.			*
 *									*
 *			Added pr_refs() for a current listing of data   *
 *			base.						*
 *									*
 *	S.D.K.								*
 *	July 12, 1981.  Moved backup files to /tmp/dbbak/{} to free *
 *			up space on /dev/rl1.  				*
 *									*
 *			Modified kwsort and refsort to wait for the	*
 *			system sort routine to finish.  Look for	*
 *			/tmp/stmlock while these are running, and	*
 *			in general, when calling the sort program	*
 *			each process must set and unset /tmp/stmlock    *
 *			to prevent botched jobs.			*
 *									*
 *	S.D.K.								*
 *	July 22, 1981.  Added a reference numbering generator for the   *
 *			saved reference file.				*
 *									*
 *	S.D.K.								*
 *	Sept  3, 1981.	Added gen_kwl() to automatically regenerate	*
 *			a reference file kew word list.  This	*
 *			will obviate the need to permanently store	*
 *			*.kwl.						*
 *									*
 *	S.D.K.								*
 *	Oct  20, 1981.	Added facilities to print references on file	*
 *			cards.  Options are whole list or just new	*
 *			data entries in xxx.new.			*
 *			   Note that user is responsible for removing   *
 *			xxx.new after printing cards for new references.*
 *									*
 *			***************					*
 *			R I P S   V 2.0					*
 *			***************					*
 *									*
 *	S.D.K.								*
 *	Jun   7, 1982.  Added facility to store book info in addition 	*
 *			to journal articles.  Call this feature by	*
 *			responding to "journal =? " with book.		*
 *									*
 *	S.D.K.								*
 *	Jun  25, 1982.  After some unhappiness over the restraints in	*
 *			the original RIPS search and retrieval sub-	*
 *			routines originally designed after features	*
 *			found in packages driving MEDLINE and MEDLAR,	*
 *			for example, a totally new retrieval syntax and	*
 *			philosophy was developed.  The entire reference *
 *			and its associated keyword list (if there) is	*
 *			now searched in a pattern matching scheme to	*
 *			pull out root words whether prefixed and/or 	*
 *			suffixed.  Of more general relevance, a non-	*
 *			restrictive boolean command parser recognizing	*
 *			nested phrases was implemented.  A search	*
 *			command line consists of a string of root key-	*
 *			words separated by (at present) by one of three *
 *			conjunctions: '&' (AND), '|' (OR), or '!' (NOT).*
 *			An example command line might look like:	*
 *									*
 *				(kw1 | kw2) & (kw3 | kw4) ! kw5		*
 *									*
 *			For compactness, exclusive or (XOR) is not	*
 *			implemented explicitly since XOR can be derived *
 *			by existing conjunctions ((a|b)!(a&b)).		*
 *			    A major advantage with the scheme is that	*
 *			a command string may contain any (reasonable)   *
 *			number of keywords, and each reference is	*
 *			searched completely and saved or not depending	*
 *			upon content.  Other schemes (e.g., MEDLAR) do	*
 *			tricks with keyed dictionaries (very fast, but	*
 *			disk space expensive) a word at a time.		*
 *			    Benchmarks made from a retrieval based on	*
 *			3 keywords searching a file of about 1800	*
 *			references showed about a 20% decrease in CPU	*
 *			time for the new retrieval method.  Cost was 2	*
 *			minutes of CPU time (3 min. real on unloaded	*
 *			system) for above search.			*
 *									*
 *	S.D.K.								*
 *	Jul   1, 1982.	Revamped the handling of books and book		*
 *			chapters throughout.  Liberalized the search	*
 *			syntax so that AND, OR, and NOT are acceptable	*
 *			substitutes for &, |, and !, respectively.	*
 *			Added documentation to output retrieval files.	*
 *									*
 *									*
 *	S. Klyce							*
 *	Jul  21, 1982	Added a file structure checker (check_db())	*
 *			to help novices out of tight places when they	*
 *			edit a reference file and mess up the strict	*
 *			structural design.  Commonly, missing or extra  *
 *			fields are the culprits.  This check is more	*
 *			powerful than the auto-checker in gen_kwl().	*
 *			Using both should allow users to spot all the	*
 *			corrupt entries.				*
 *									*
 *	S.D.K.								*
 *	Nov  29, 1982	Eliminated backup of keyword list to save disk	*
 *			space.  Lost keyword lists can now be retrieved	*
 *			only by regeneration or from backup tapes.	*
 *			  Moved /tmp/rtm --> rtm to avoid multiuser	*
 *			clashes.					*
 *			  Our first new file (about 30 refs) was	*
 *			lost !  Was unable to reconstruct the series	*
 *			of events to discover whether the culprit was	*
 *			a multiuser clash, user trickiness, or system	*
 *			bug.  Will be watching this ....		*
 *									*
 *	Nov  30, 1982	Gotcha !  Above bug was found to be due to a	*
 *			user logging out while the sort job was in	*
 *			progress.  Added a nohup to the system call and	*
 *			changed refsort so that a minimum time exists	*
 *			when the original file is zero length.	*
 *			We still lose if we crash during a file mv, but	*
 *			that's why we do the reference count on entry.	*
 *			  Added a comment when an appended reference is	*
 *			not written on the file.			*
 *									*
 *	Dec   2, 1982	Added better protection to user files.	*
 *			Now calls to stat() and getuid() are made	*
 *			and users are kicked out of the editor if	*
 *			the current file is not owned by them	*
 *									*
 *	Dec   3, 1982	After some badgering by enthusiastic users, a	*
 *			routine to permit retrievals by publication	*
 *			date was added.  We now have two preposi-	*
 *			tions: BEFORE (<) and AFTER (>).  Now we can	*
 *			write:						*
 *				a AND b AND BEFORE 1980 AND AFTER 1970	*
 *									*
 *	Dec   4, 1982	Any changes in rips are now anounced to the	*
 *			users after entering rips.  The messages live	*
 *			in /usr/lib/rips/doc/rmotd.nr.			*
 *									*
 *	Dec   10, 1982	The task of making reprint request cards is an	*
 *			odious, tedious and frustrating.  When (if) the *
 *			reprints arrive, the process begins anew to 	*
 *			enter the new items to a rips file.  There- *
 *			fore, we added routines to print post cards	*
 *			(continuous form) while adding the references	*
 *			to the file.				*
 *			  It is assumed that the user is working from	*
 *			ISI's Current Contents.				*
 *									*
 *	Dec   24, 1982	Spun off gen_kwl() to an external program in an	*
 *			effort to make more space for other goodies.	*
 *			The keyword list is still regenerated from	*
 *			within rips, and since it's not used routinely	*
 *			the slight slowdown in execution is acceptible.	*
 *									*
 ************************************************************************/

FILE *fp, *fopen();
FILE *fq, *fopen();
FILE *fr, *fopen();
FILE *fs, *fopen();
FILE *ft, *fopen();
FILE *fu, *fopen();
FILE *fl, *fopen();

char db_name[DLEN];
char dbn[DIRSIZ];
char year[YLEN];
char title[TLEN];
char journal[JLEN];
char auth[MAXAUTHS][ALEN];
char keytab[MAXKEYS][KLEN];
char ktsearch[MAXKEYS][KLEN];
char volume[VLEN];
char pp[PLEN];
char *db_direct = { "/usr/lib/rips/" };
char *kw_name = {"                                      "};
char *new_entries = {"                                      "};
char *rr_auths = {"/usr/lib/rips/rr/rr_auths"}; /*  file storing 1st auth names */
char *rrequests = {"/usr/lib/rips/rr/rrequests"}; /* file storing printed requests */
char *rr_labels = {"/usr/lib/rips/rr/rr_labels"}; /*  author label file for rr's */
char *rr_cards = {"/usr/lib/rips/rr/cards"};  /*  final rr card file	*/
char *sys_guru = {"Dr Klyce"};
char sdbbakup[CLEN];
char ref[RLEN];
int s_cnt = 0;
int debug = 0;
char *rtine[] = {

	"getmode",
	"edit",
	"refpr",
	"pr_ref",
	"pr_card",
	"get_rrad",
	"mk_rrlabel",
	"mk_rrequest",
	"get_who",
	"search",
	"get_db"
};
char *mesg[] = {

	"Huh ?",
	"file busy",
	"Can't open input file",
	"format error",
	"Can't create output file",
	"system error",
	"Permission to alter file denied",
	"syntax error",
	"Correct ? (yes) ",
	"working ...\b",
	"Instructions ? (no) ",
	"******************************\n",
	"*** REPRINT REQUEST ",
	" ***\n",
	"*** LSU/LIONS EYE RESEARCH",
	"Entry not saved",
	"\n\nEnter option - \n\n"
};

struct request_mesg {
	char dear[35], leadin[90], signoff[80];
} p[] = {
/* English */	"Dear Colleague,",
		 "I would greatly appreciate receiving a\n\treprint of your article:",
		  "Thank you for your help in this matter.\n",
/* German */	"(KSehr geehrter Kollege!",
		 "F}r die ]bersendung eines Sonderdruckes\n\tIhrer interessanten Arbeit:",
		  "w{re ich Ihnen sehr zu Danke verplichtet.\n\tMit freundlichen Gr}ssen,(B",
/* French */	"(RMon cher et distingu} Confr}re,",
		 "Je vous serai bien oblige{ pour l\'envoi\n\td\'un tirage @ part de votre publication:",
		  "Agr{e, Monsieur, mes remerciements tr}s\n\trespectueux.(B",
/* Italian */	"Egregio Collega,",
		 "Le saro molto riconoscente se vorra\n\tinviarmi una copia del Suo lavoro:",
	 	 "RingraziandoLa anticipamente Le mando i\n\tmiei saluti.",
/* Spanish */	"(RApreciado Colega,",
		 "Tengo inter{s en recibir una copia de su\n\tarticulo:",
		  "Agradeciendo de antemano su atencion\n\tatentamente,(B",
/* Dutch */	"Hooggeachte Collega,",
		 "Gaarne ontving ik, indien mogelijk, eem\n\toverdruckje van uw artikel:",
		  "U bij voorbaat dankend, teken ik met\n\tvriendlijke goeten en hoogachting,"
};
struct countries  {
	char country[10]; int lang;
} c[] = {
	"deutch",	1,
	"german",	1,
	"ddr",		1,
	"switz",	1,
	"austri",	1,
	"oester",	1,
	"france",	2,
	"belg"	,	2,
	"itali",	3,
	"italy",	3,
	"spain",	4,
	"mexico",	4,
	"argentin",	4,
	"chile",	4,
	"brazil",	4,
	"venezu",	4,
	"puerto",	4,
	"costa ri",	4,
	"boliv",	4,
	"peru",		4,
	"paragua",	4,
	"uragua",	4,
	"columbia",	4,
	"ecuador",	4,
	"cuba",		4,
	"holland",	5,
	"nether",	5,
	"",		0
};
main()
{
	lf(SCRN_HT,stdout);
	printf("\t\t\t* * * R I P S * * *\n\n");
	printf("             Reference Information Program for Scientists\n\n");
	system ("cat /usr/lib/rips/doc/rmotd.nr");  /* Useful to alert users of
						  recent changes, etc.     */
	rips();
}
rips()
{
	getdb();
	getmode();
}
getdb()
{
	auto char dummy[16];
	auto char dbak[CLEN];
	auto char s[50];
	if ((fl=fopen("/tmp/busy_rr","r"))>0)
		error(7,1,NONFATAL);
	fclose(fl);
	printf("Current files:\n\n");
	system("lss /usr/lib/rips/files");  /* lss is multicolumn version of UNIX ls */
	*dbn='|';
	while(*dbn=='|')
		strinp("\nEnter file name, help, or quit =? ",dbn,DLEN);
	if (index(dbn,"help")>=0) {
		system("cat /usr/lib/rips/doc/README.nr | pg");
		getdb();
	}
	else if (index(dbn,"quit")>=0)
		exit(0);
	else if (1)		/*  Here we put together sh command lines  */
				/*  and file path names for devious        */
				/*  purposes which include backing up the  */
				/*  user's file prior to permitting	   */
				/*  any new scribbling.  Users are not	   */
				/*  given clues as to where the backup     */
				/*  copies are stored to prevent novices   */
				/*  from ruining both copies.  This method */
				/*  is far from foolproof, however, since  */
				/*  if the user re-edits a blitzed data-   */
				/*  base the blitzed file will replace */
				/*  the virgin backup.  All is not lost if */
				/*  a regular system backup is faithfully  */
				/*  performed.				   */

	{
		squeeze(dbn,'|');
		sprintf(db_name,"%sfiles/%s",db_direct,dbn);
		sprintf(new_entries,"%snew/%s.new",db_direct,dbn);
		sprintf(kw_name,"%skwl/%s.kwl",db_direct,dbn);
		sprintf(dbak,"%sbkup/%s.bak",db_direct,dbn);
		sprintf(sdbbakup,"cp %s %s",db_name,dbak);
		sprintf(s,"wc %s",db_name);
		*dummy='y';
		if ((fp=fopen(db_name,"r"))<=0) {
			strinp("Confirm that this is a new file (answer yes)...",dummy,6);
			if (*dummy=='y') {
				if ((fp = fopen(db_name,"w"))<=0)
					error(10,4,FATAL);
				fclose(fp);
				if ((fp = fopen(kw_name,"w"))<=0)
					error(10,4,FATAL);
				fclose(fp);
				if ((fp = fopen(new_entries,"w"))<=0)
					error(10,4,FATAL);
				fclose(fp);
			}
		}

		if (*dummy=='y') {
			printf("File size (# refs  # words) =\b");
			system(s);
			printf("If this too small a number, exit rips and contact %s\n",sys_guru);
		}
		else getdb();
	}
	else getdb();
}
getmode()
{
	auto char ans[4];
	auto char dummy[CLEN];
	s_cnt=0;
	printf("%s",*(mesg+16));
	printf("\te - editor\n");
	printf("\tr - retrieval\n");
	printf("\tg - regenerate keyword list\n");
	printf("\td - change to new data base\n");
	printf("\tp - print cards, refs, keywords, or request reprints\n");
	printf("\tw - temporary exit to shell\n\tq - exit to shell\n\n");
	strinp("=? ",ans,4);
	if (*(ans+1)=='d') debug=1;
	else debug=0;
	switch(*ans) {
	case 'e':
		edit();
		break;
	case 'd':
		rips();
		break;
	case 'r':
		retrieve();
		break;
	case 'w':
		printf("Type a <cntrl> c to return to rips\n");
		system("/bin/sh");
		break;
	case 'q':
	case 'x':
		exit();
		break;
	case 'p':
		if (*(ans+1)=='k')
			pr_mode(1);
		else pr_mode(0);
		break;
	case 'g':
		gen_kwl();
		break;
	default:
		error(0,0,NONFATAL);
		getmode();  /*  Huh?  Try again. */
	}
	getmode();
}

pr_mode(kflag)
int kflag;
{
	auto char ans[4];
	printf("%s",*(mesg+16));
	printf("\tt - reference list on terminal\n");
	printf("\tp - reference list on line printer\n");
	printf("\tk - keyword list on terminal\n");
	printf("\tl - keyword list on line printer\n");

	printf("\tc - print file or reprint request cards\n\n");
	strinp("=? ",ans,4);
	switch (*ans) {

		case 'k':
			if (stat_kwl(db_name,kw_name))
				kwlist(TTY);
			break;
		case 'l':
			if (stat_kwl(db_name,kw_name))
				kwlist(LPR);
			break;
		case 'p':
			pr_refs(LPR,kflag);
			break;
		case 't':
			pr_refs(TTY,kflag);
			break;
		case 'c':
			pr_cards();
			break;
		default:
			error(0,0,NONFATAL);
			pr_mode();
			break;
	}
}

edit()
{
	char ans[4];
	char s[SLEN];
	int i;
	if (get_perm(db_name));
	else if ((i=lock())<0) error(1,1,NONFATAL);
			/*  Prevent all from editing if the file
			    is currently being processed by rips in
				the background				*/
	else {
		printf("%s",*(mesg+16));
		printf("\ta - append\n");
		printf("\tc - correct\n");
		printf("\ts - check file structure for errors\n");
		printf("\tr - return to RIPS\n\tq - return to shell\n\n");
		strinp("=? ",ans,4);
		sprintf(s,"nohup sh /usr/bin/refsort %s %s&",db_name,dbn);

		/* The nohup above is extremely important !  Now the user can
		 log out whilst the references are being sorted.  Refsort was
		 changed as well so that the refs are sorted into a tmp file,
		 then moved rather than directly onto the old ref file.
		 This decreases the length of time the original file is
		 susceptable to system crash.				*/

		switch(*ans) {
		case 'a':
			backup();
			ed_append(NORMAL);
			system(s);
			printf("%s until sorted\n",*(mesg+1));
			break;
		case 'c':
			backup();
			ed_correct();
			ulock();
			break;
		case 's':
			check_db();
			ulock();
			break;
		case 'r':
			ulock();
			rips();
			break;
		case 'q':
		case 'x':
			ulock();
			exit();
			break;
		default:
			ulock();
			edit();
		}
	}
	return(0);
}
ed_append(rrflag)
int rrflag;
{
	static char dummy[TLEN],c,*ppflag;
	auto char lname[ALEN];
	auto char inits[10];
	auto char temp[RLEN];
	auto char rref[RLEN];
	int i,j,auth_cnt,key_cnt,runflag=1;
	printf("Respond to ref. type with:\n\n\tj - for journal article\n");
	printf("\tc - for chapter in book\n\tb - for entire book\n");
	printf("\tq - to end session\n\n");
	while (runflag)
	{
		do {

			strinp("ref. type (j, c, b, q) =? ",ppflag,6);
			switch (*ppflag) {
				case 'j':
				case 'c':
				case 'b':
					break;
				case 'q':
					runflag=0;
					break;
				default:
					*ppflag='|';
			}
		} while (*ppflag=='|');
		if (runflag) {
			for (i=0;;)
			{
				printf("author %1d",1+i++);
				strinp("=? ",auth[i-1],ALEN);
				if (*auth[i-1]=='|') break;
			}
			auth_cnt=i-1;
			strinp("year =? ",year,YLEN);
			switch (*ppflag) {
				case 'j':
					strinp("title =? ",title,TLEN);
					strinp("journal =? ",journal,JLEN);
					strinp("vol.# =? ",volume,VLEN);
					break;

				case 'b':
					strinp("Book title =? ",title,TLEN);
					strinp("Publisher =? ",journal,JLEN);
					strinp("City      =? ",volume,VLEN);
					replace(journal,"|",", |",1);
					break;
				case 'c':
					strinp("Chapter title =? ",title,TLEN);
					strinp("Book title =? ",dummy,TLEN);
					squeeze(title,'|');
					strcat(title,".  In, ");
					strcat(title,dummy);
					strinp("Ed. by =? ",dummy,JLEN);
					replace(dummy,"|",". |",1);
					sprintf(journal,"Ed. by %s",dummy);
					strinp("Publisher =? ",volume,VLEN);
					replace(volume,"|",", ",1);
					strinp("City      =? ",dummy,VLEN);
					strcat(volume,dummy);
/*
					if (debug) printf("TI: %s\nJO: %s\nVO: %s\n",title,journal,volume);
*/
			}
			strinp("pages =? ",dummy,PLEN);
			check(title);
			check(journal);
			check(volume);
			check(dummy);
			check(year);
			switch (*ppflag) {
				case 'c':
					sprintf(pp,"pp.%s",dummy);
					break;
				case 'b':
					sprintf(pp,"%s",dummy);
					squeeze(pp,'|');
					strcat(pp," pp|");
					break;
				default:
					sprintf(pp,"%s",dummy);
			}
			for(i=0;;)
			{
				printf("keyword %1d",1+i++);
				strinp("=? ",keytab[i-1],KLEN);
				if (*keytab[i-1]=='|') {
					key_cnt=i-1;
					break;
				}
			}
			sprintf(ref,"%s",*auth);
			if (rrflag==RR) {
				sprintf(rref,"%s",*auth);
			}
			for (i=0;i<auth_cnt-1;i++)
			{
				if (i>0)
				{
					strcat(ref,", ");
					strcat(ref,auth[i]);
				}
			}
			if (auth_cnt>1)
			{
				strcat(ref,", ");
				if (rrflag==RR) strcat(rref,", et al.");
				strcat(ref,auth[i]);
			}
			if (rrflag==RR) {
				sprintf(dummy,"%s",title);
				if (slen(title)>40) {
					for (i=35;*(dummy+i) != ' ';i++) {
						if (*(dummy+i)=='\0'|| *(dummy+i) == '-')
							break;
					}
					*(dummy+i)='\0';
					strcat(dummy,"...");
				}
				sprintf(temp,"  %s. %s.  %s %s:%s.",year,dummy,journal,volume,pp);
				strcat(rref,temp);
			}
			sprintf(temp,"  %s. %s.  %s %s:%s.",year,title,journal,volume,pp);
			strcat(ref,temp);
			refpr(ref,PAGE,TTY);
			printf("key words:");
			for (i=0;i<key_cnt;i++)
			{
				printf("  ");
				putline(keytab[i],KLEN);
			}
			printf("\n\n");
			strinp(*(mesg+8),dummy,5);
			if (*dummy=='|' || *dummy=='y')
			{
				if ((fp=fopen(db_name,"a"))<=0)
					error(1,4,FATAL);
				if ((fr=fopen(new_entries,"a"))<=0)
					error(1,4,FATAL);
				for (j=fp;j<=fr;j += ((int)fr-(int)fp)) {
					fprintf(j,"%2d|",auth_cnt);
					for (i=0;i<auth_cnt;i++)
						fprintf(j,"%s",auth[i]);
					fprintf(j,"%s%s%s%s%s",year,title,journal,volume,pp);
					fprintf(j,"%2d|",key_cnt);
					for(i=0;i<key_cnt;i++)
						fprintf(j,"%s",keytab[i]);
					fprintf(j,"\n");
					fclose(j);
				}
				if (runflag && rrflag==RR) {
					if ((fr = fopen(rrequests,"a"))<=0)
						error(1,4,FATAL);
					refpr(rref,CARD,RRFILE);
					fclose(fr);
					if ((fs=fopen(rr_auths,"a"))<=0)
						error(1,4,FATAL);
					squeeze(*auth,'|');
					for (i=0;*(*auth+i)!='\0';i++)
						*(lname+i)= *(*auth+i);
					while (*(lname+ --i) != ' ' && i>0);
					*(lname+i)='\0';
					for (j=0;*(*auth+i)!='\0';j++,i++)
						*(inits+j)= *(*auth+i);
					*(inits+j)='\0';
					fprintf(fs,"Dr.%s %s\n",inits,lname);
					fclose(fs);
				}
			}
			else {
				*title='\0';
				printf("%s\n",*(mesg+15));
			}
		}
	}
}
ed_correct()
{
	char s[CLEN];
	system("cat /usr/lib/rips/doc/CORRECT.ins");
	sprintf(s,"ed %s",db_name);
	system(s);
}

retrieve()
{
	char ans[32], stype[SLEN], syntax[MAX_CONJ], c, s;
	int op_paren[MAX_PAREN], cl_paren[MAX_PAREN];
	int i,j,k,l,m,n,o,temp,c_cnt=0;
	strinp(*(mesg+10),ans,8);
	if (*ans != '|') {

		lf(SCRN_HT,stdout);
		system("cat /usr/lib/rips/doc/RETR.INS | pg");
	}
	for (*ans='\0';*ans!='|' && *ans!='y';) {
		strinp("Search command line =? ",stype,SLEN);
		strinp(*(mesg+8),ans,8);
		*(stype+slen(stype)-1) = ' ';  /* Get rid of the | from strinp */
		if (*(stype)=='\"') {  /* get command from file */
			squeeze(stype,'\"');
			squeeze(stype,' ');
			if ((fl=fopen(stype,"r"))<=0)
				error(9,2,FATAL);
			fgets(stype,SLEN,fl);
			squeeze(stype,'\n');
			printf("File command line: %s\n",stype);
			fclose(fl);
		}
	}
	squeeze(stype,' ');		/* and remove all the spaces.	  */
	replace(stype,"BY","AND",0);	/* Here we fake an author cue	*/
	replace(stype,"AND","&",0);	/* Get rid of alternate		*/
	replace(stype,"NOT","!",0);	/* conjunctions & preps		*/
	replace(stype,"BEFORE","<",0);
	replace(stype,"OR","|",0);	/* Must be done after befORe	*/
	replace(stype,"AFTER",">",0);
	replace(stype,"&!","-",0);
	replace(stype,"|!","X",0);
	replace(stype,"IN","=",0);

	for (i=j=k=l=m=0,*(op_paren+m++)=0,n=1;*(stype+i) != '\0';i++) {

		/* Here we parse the command line by building a list of	*/
		/* keywords in ktsearch, a string composed of conjunc-	*/
		/* tions in order of appearance, and two arrays which	*/
		/* store the number and positions of open and close	*/
		/* parentheses.  In the above line we add a leading	*/
		/* open paren and at the end of the loop, we add a	*/
		/* closing paren to the outermost nest (@ n=0).		*/

		switch (*(stype+i)) {
			case '(':
				*(op_paren+m++)=k;
				break;
			case ')':
				if (l) {  /* We are done building a word */
					*(ktsearch[k++]+l) = '\0';
					l=0;
				}
				*(cl_paren+n++)=k-1;
				break;
			case '&':
			case '|':
			case '-':
			case 'X':
				*(syntax+j++) = *(stype+i);
				++c_cnt;
				if (l) {  /* Done with a word, again */
					*(ktsearch[k++]+l) = '\0';
					l=0;
				}
				break;
			default:
				*(ktsearch[k]+l++) = lower(*(stype+i));
					/* Build a keyword in the search    */
					/* table; lower case only.	    */
		}
	}
	if (l) *(ktsearch[k++]+l) = '\0';	/*  Terminate last keyword */
	*(syntax+j) = '\0';			/* and syntax string.	   */
	*(cl_paren) = k-1;  /* Add closing paren after keyword[k-1] */

	for (i=1;i<m-1;i += j) {	/*  Here we reverse the order of   */
					/*  the closing parens for all     */
					/*  equal open parens so that nests*/
					/*  will be parsed innermost to	   */
					/*  outermost as promised.	   */

		for (j=i;*(op_paren+i) == *(op_paren+j);j++);
		for (l= --j,o=i;o<l;o++,l--) {
			if (debug) printf("j=%d o=%d l=%d\n",j,o,l);
			temp = *(cl_paren+o);
			*(cl_paren+o) = *(cl_paren+l);
			*(cl_paren+l) = temp;
		}
	}

	if (m != n || k != c_cnt+1) {
			/*  Preliminary routines to check for syntactical  */
			/* errors in the command line.  Should be beefed  */
			/* up in the future to report other sorts of      */
			/* illegal syntax by creative (?) users.	  */
		error(9,3,NONFATAL);
		retrieve();
	}
	else {
		s_cnt=0;
		if ((fr = fopen("rtm","w"))<=0)
			error(9,4,FATAL);
		date(ans);
		fprintf(fr,"\t\tRIPS search command:\n\t\t\t");
		for (i=j=0;*(stype+i++)!='\0';) {
			if (j++ >SCRN_WIDTH) {
				j=0;
				fprintf(fr,"\n\t\t\t%c",*(stype+i-1));
			}
			else putc(*(stype+i-1),fr);
		}
		fprintf(fr,"\n\t\tperformed on %s\n",db_name);
		fprintf(fr,"\t\t%s\n\n",ans);
		if (debug) printf("k=%d\nop=%d\ncl=%d\nm=%d\nsyntax=%s\n",k,op_paren[m-1],cl_paren[n-1],m,syntax);
		printf("\nLooking for [%s]\n\tin %s...\b",stype,db_name);
		search(ktsearch,k,op_paren,cl_paren,m,syntax);
		putchar('\n');
		if (s_cnt>1) printf("%3d references",s_cnt);
		else if (s_cnt) printf("%2d reference",s_cnt);
		else printf("No references");
		printf(" retrieved in file.\n");
		fclose(fr);
		if (s_cnt>0)
		{
			strinp("Would you like to list the result on the terminal =? ",&ans,6);
			if (*ans=='y') system("cat rtm|pg");
			printf("Results saved in ");
			system("echo $s/refs");
			system("mv rtm $s/refs$$");
		}
		strinp("Another search =? ",ans,8);
		if (*ans=='y') retrieve();
	}
}

search(keywords,nkwds,op,cp,depth,syn)
char keywords[MAXKEYS][KLEN],syn[MAX_CONJ];
int nkwds, op[MAX_PAREN], cp[MAX_PAREN], depth;
{
	register i,j;
	int k,d;
	int dates = -1;
	char code[MAXKEYS], sref[RLEN], s[MAX_CONJ];
	int auth_cnt;
	char kdate[2][5],r_date[5];
	for (k=0;i<nkwds;i++) {
		switch (**(keywords+i)) {
			case '<':
			case '>':
			case '=':
				for (j=0;j<4;j++) {
					*(kdate[k]+j) = *(keywords[i]+j+1);
				}
				*(kdate[k++]+j)= '\0';
				if (k>2)
					error(9,3,NONFATAL);
/*
				if (debug) for (j=0;j<k;j++) printf("kdate=%s\n",kdate[j]);
*/
				dates=i;
				break;
			default:
				break;
		}
	}
	if (debug) {
		putc('\n',stdout);
		for (i=0;i<nkwds;i++) printf("%s\n",*(keywords+i));
		for (i=0;i<depth;i++) printf("op[%1d]=%d cp[%1d]=%d\n",i,op[i],i,cp[i]);
	}
	if ((fp = fopen(db_name,"r"))<=0)
		error(9,2,FATAL);
	while (fgets(ref,RLEN,fp) != NULL) {  /* Read references 'til end */
		sscanf(ref,"%d|",&auth_cnt);  /*  Grab the # auths for later use by save() */
		slowcpy(sref,ref);	     /*  Make all chars lower case. */
		for (d=i=0;i<nkwds;i++) {	/*  Search the reference  */
						/*  for the presence of a */
						/*  keyword or date range */
						/*  building the code word*/
						/*  for parsing below.	  */
			*(code+i)='0';
			if (debug) printf("**kwds+i= %c\n",**(keywords+i));
			if (dates==i) switch (**(keywords+i)) { /*  Check for date	*/
				case '<':
				case '>':
				case '=':
					for (j=k=0;j<=auth_cnt;j++) {
						while (*(sref+k++)!='|');
					}
					for (j=0;j<4;j++)
						*(r_date+j)= *(sref+k++);
					*(r_date+j)='\0';
					if (debug) printf("r_date= %s\n",r_date);
					switch (**(keywords+i)) {
						case '<':
							if (atoi(*(kdate+d++))>atoi(r_date))
								*(code+i)='1';
								break;
						case '>':
							if (atoi(*(kdate+d++))<atoi(r_date))
								*(code+i)='1';
								break;
						case '=':
							if (atoi(*(kdate+d++))==atoi(r_date))
								*(code+i)='1';
								break;
						default:
							break;
					}
			}
			else if (index(sref,*(keywords+i))>=0)
				*(code+i) = '1';
		}
		*(code+i) = '\0';		/* Terminate code word    */
		if (debug) printf("code=%s\n",code);
/*
printf("\ncode=%s\n",code);
*/
		sprintf(s,"%s",syn);

		for (d=depth;--d>=0;) { /* Analyze innermost to outermost */
					/* nests and "when you get to the */
					/* end, stop." { L. Carroll }     */

			for (i = *(op+d);i <= *(cp+d);i++) {
					/* Don't get in over your head,   */
				 	/* just examine one nest at a time*/
					/* analyzing each nest from left  */
					/* to right.			  */

				switch (*(s+i)) {
					case '-':
					case 'X':
						*(code+i+1)=not(*(code+i+1));
					/*  Below we test for the presence   */
					/*  of a parenthetical expression    */
					/*  after the NOT.  If there, we NOT */
					/*  all the codes in that expression */

						if (d<depth && *(op+d+1)==i+1)
							for (k = *(op+d+1)+1;k<=*(cp+d+1);k++)
								*(code+k) = *(code+i+1);
						break;
				}
				switch (*(s+i)) {

					case '-':
					case '&':	/* AND */
						if (*(code+i) != '1' || *(code+i+1) != '1')
							*(code+i) = *(code+i+1) = '0';
						/* else both are 1's, leave alone */
						break;

					case 'X':
					case '|':	/*  OR */
						if (*(code+i) != '1' && *(code+i+1) != '1')
							*(code+i) = *(code+i+1) = '0';
						else for (j = *(op+d);j <= i+1;j++)
							*(code+j)='1';
						/* Above we make sure all    */
						/* code members to the left  */
						/* of this test are made true*/
						/* back to the open paren.   */

						if (debug) {
							printf("OR: %c | %c\n",*(code+i),*(code+i+1));
							printf("CODE: %s\n",code);
						}
						break;

/*
					case '!':	
						if (debug) printf("NOT: %c ",*(code+i+1));
						if (*(code+i+1)=='1')
							*(code+i+1)='0';
						else *(code+i+1)='1';
						if (debug) printf("!= %c\n",*(code+i+1));

						if (debug) printf("code=%s\n",code);
						break;
*/

					case '@':	/* Been here already */
					case '\0':	/* Kludge of the year */
						break;

					default:
						error(9,5,FATAL);
							/* Should NEVER get here */
				}
				*(s+i) = '@';  /* parse once only */
			}
		}
		if (index(code,"0")<0) {
			if (debug) printf("HIT\n");
			save(ref,auth_cnt,PAGE,NOKWS,LPR);
		}
		else if (debug) printf("MISS\n");
/*
printf("code=%s\n",code);
*/
	}
	fclose(fp);
}


save(s,a,mode,kws,dev)
char s[RLEN];
int a, dev;
int mode,kws;
{
	char sref[RLEN];
	register i,b,k;
	s_cnt++;
	b=k=0;
/*
	if (debug) printf("save:\n%s\n",s);
*/
	while (s[b++]!='|');
	if (mode==PAGE)
	{
		k=8;
		sprintf(sref,"%5d.  ",s_cnt);
	}
	while (s[b++]!='|') sref[k++]=s[b-1];
	for (i=0;i<a-1;i++)
	{
		if (i>0)
		{
			sref[k++]=',';
			sref[k++]=' ';
			sref[k++]=' ';
			while(s[b++]!='|') sref[k++]=s[b-1];
		}
	}
	if (a>1)
	{
		sref[k++]=' ';
		sref[k++]=' ';
		sref[k++]='&';
		sref[k++]=' ';
		while (s[b++]!='|') sref[k++]=s[b-1];
	}
	sref[k++]=' ';
	sref[k++]=' ';
	sref[k++]='(';
	while (s[b++] != '|') sref[k++]=s[b-1];
	sref[k++]=')';
	sref[k++]='.';
	sref[k++]=' ';
	sref[k++]=' ';
	while (s[b++] != '|') sref[k++]=s[b-1];
	sref[k++]='.';
	sref[k++]=' ';
	sref[k++]=' ';
	while (s[b++] != '|') sref[k++]=s[b-1];
	sref[k++]=' ';
	while (s[b++] != '|') sref[k++]=s[b-1];
	sref[k++]=':';
	while (s[b++] != '|') sref[k++]=s[b-1];
	sref[k++]='.';
	sref[k++]='^';
	if (kws==KWS) {
		sref[k++]='^';
		while (s[b++]!='|');
		if (s[b-1]!='0')
			while (s[b++]!='\0') sref[k++]=s[b-1];
		sref[k++]='^';
	}
	sref[k++]='\0';
	if (kws==KWS) replace(sref,"|"," ",0);
	refpr(sref,mode,dev);
}

kwlist(mode)
int mode;
{
	char s[CLEN];
	if (mode==LPR) {

		sprintf(s,"pr -4 -w132 -h \"RIPS Keyword List from %s\" %s|dlpr -f&",dbn,kw_name);
		printf("Keywords being put out to lpr\n");
	}
	else if (mode==TTY)
		sprintf(s,"cat %s|acol|pg",kw_name);
			/*  Acol & pg do automatic columnation & and 	*/
			/*  give a screenful at a time.  These are on	*/
			/* the USENIX distribution tapes (acol is from	*/
			/* Purdue; pg lives at delaware/geotronics/s6/  */
			/* pg.c).  These are used here without apology  */
			/* to save core.  Besides every good UNIX site  */
			/* should have local substitutes; else just 	*/
			/* "cat kw_name" here.	 			*/
	system(s);
}

check(s)	/* This routine checks for missing info in reference	*/
		/* words and replaces them with '?'.			*/
char s[TLEN];
{
	if (*s=='|')
		sprintf(s,"?|");
}

putline(r,limit)  /*  Write a string to the standard output terminating */
		  /* on | rather than \0. 				*/
char r[];
int limit;
{
	int i = 0;
	while (--limit>0 && *(r+i) != '|') putchar(*(r+i++));
}
squeeze(s,c)
char s[RLEN];
int c;
{
	int i,j;
	for (i=j=0;*(s+i)!='\0';i++)
		if (*(s+i)!=c)
			*(s+j++) = *(s+i);
	*(s+j)='\0';
}
refpr(s,mode,d)	/*  Prints a reference formatted according to mode  */
			/*  in a place specified by d.		    */
int mode, d;
char s[RLEN];
{
	register i,j,k;
	int l,fildes,firstline;
	int width;
	int len;
	char frs[RLEN];
	squeeze(s,'|');
	if (d==TTY) fildes = stdout;
	else fildes=fr;
	if (mode==PAGE)
		width = SCRN_WIDTH;
	else
		width = CARD_WIDTH;
	replace(s,"^^","  Keywords: ",0);
	squeeze(s,'^');
	squeeze(s,'\n');
	len=slen(s);
	firstline=j=l=0;
	while(len>width)
	{
		k=0;
		*frs='\0';
		i=width+j;
/*
		while(*(s+--i)!=' ' && i>=0);
*/
		while(*(s+--i)!=' ');
		for (;j<i;frs[k++]=s[j++]);
		frs[k]='\0';
		while (s[++j]==' ') k++;
		len -= k;
		if (mode==PAGE) fprintf(fildes,"%s\n\t  ",frs);
		else fprintf(fildes,"%s\n  ",frs);
		if (l > 16 && mode==CARD) error(2,4,FATAL);
		++l;
/*
		if (debug) printf("%s\n",frs);
*/
		if (mode==PAGE && !firstline++) width -= 10;
	}
	*frs='\0';
	for (i=j,k=0;s[j]!='\0';frs[k++]=s[j++]);
	frs[k]='\0';
	fprintf(fildes,"%s",frs);
/*
	if (debug) printf("%s\n",frs);
*/
	l = CARD_HT - l;
	if (mode==CARD && d != RRFILE) lf(--l,fildes);
	else lf(2,fildes);
}
lower(c)	/* Local version to convert to lower case.  Could be	*/
		/* site dependent!					*/
int c;
{
	if (c>=65 && c<=90)
		return(c+32);
	else return (c);
}
backup()	/*  Save files in a semi protected place */
{
	printf("Backup files being created...");
	system(sdbbakup);
	printf("\n");
}
error(s,m,type)	/* Print the subroutine, error message and bombout or return */
int s, m, type;
{
	printf("RIPS-%s: %s\n",*(rtine+s),*(mesg+m));
	if (type==FATAL) exit(0);
}

pr_refs(device,kflag)
int device, kflag;
{
	int auth_cnt,kp;
	char ans[5];
	char s[CLEN];
	if (device==LPR) {
		if ((fr=fopen("/tmp/rips_db","w"))<=0)
			error(3,4,FATAL);
	}
	if ((fp=fopen(db_name,"r"))<=0)
		error(3,3,FATAL);
	while (fgets(ref,RLEN,fp) != NULL)
	{
		sscanf(ref,"%d|",&auth_cnt);
		if (kflag) kp=KWS;
		else kp = NOKWS;
		save(ref,auth_cnt,PAGE,kp,device,fr);
	}
	fclose(fp);
	if (device==LPR) {
		fclose(fr);
		do strinp("Are you ready to print (y or n) ? ",&ans,4);
		while (*ans!='y' && *ans != 'n' && *ans != '|');
		if (*ans=='y') {
			sprintf(s,"pr -h \"Reference List from %s\" /tmp/rips_db|dlpr -if&",dbn);
			system(s);
		}
		else printf("Result stored in /tmp/rips_db\n");
	}
}
pr_cards()
{
	auto char ans[6],anss[6];
	auto int i = 0;
	auto int ra_cnt,rl_cnt;
	auto int auth_cnt;
	auto char s[CLEN];
	do {
		printf("%s",*(mesg+16));
		strinp("\tf - file cards\n\tr - reprint request cards\n=? ",anss,5);
	} while (*anss != 'f' && *anss != 'r');
	switch (*anss) {

		case 'f':
			do {
				printf("%s",*(mesg+16));
				strinp("\tw - whole list\n\tn - new entries\n=? ",ans,5);
			} while (*ans != 'n' && *ans != 'w');
			switch (*ans) {
		
				case 'n':
					if ((ft = fopen(new_entries,"r"))<=0)
						error(4,2,FATAL);
					break;
				case 'w':
					if ((ft = fopen(db_name,"r"))<=0)
						error(4,2,FATAL);
					break;
			}
			if ((fr = fopen("/tmp/rips_cards","w"))<=0)
				error(4,5,NONFATAL);
			else {
				printf("%s",*(mesg+9));
				sprintf(s,"\n\n\n\n\n\n\nCARD SOURCE: %s\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",dbn);
				fprintf(fr,"%s",s);
				while (fgets(ref,RLEN,ft) != NULL)
				{
					sscanf(ref,"%d|",&auth_cnt);
					save(ref,auth_cnt,CARD,KWS,LPR);
				}
				fprintf(fr,"%s",s);
				fclose(fr);
				fclose(ft);
				if ((fr = fopen("/tmp/rips_cards","r"))<=0)
					error(4,2,FATAL);
				while (fgets(ref,RLEN,fr) != NULL) i++;
				if ((i-12)%CARD_HT != 0)
					error(4,3,NONFATAL);
				else printf("Your cards are stored in /tmp/rips_cards.\nRemember to remove /usr/lib/rips/new/{filename}.new !\n");
				fclose(fr);
			}
			break;
		case 'r':
			i=1;
			printf("%s",*(mesg+10));
			strinp("",ans,5);
			if (*ans=='y')
				system("cat /usr/lib/rips/doc/RR.nr|pg");
			if (!get_perm(db_name)) {
				if ((fr=fopen("/tmp/busy_rr","r"))<=0) {
					if ((fr=fopen("/tmp/busy_rr","w"))<=0)
						error(4,4,FATAL);
					fprintf(fr,"busy");
					fclose(fr);
				}
				else error(4,1,FATAL);
				if ((fr=fopen(rrequests,"r"))>0) {
/*
					printf("Reprint request file already exists!\n");
					do {
						printf("%s",*(mesg+16));
						strinp("\tc - continue the file\n\tz - zero file, start new one\n",ans,5);
					} while (*ans!='c' && *ans!='z');
					if (*ans=='z') {
						strinp("Confirm you want to zero request file (y) ",ans,5);
						if (!get_perm(rrequests) && *ans=='y')
							system("rm -f /usr/lib/rips/rr/*");
						else {
							i=0;
							system("rm -f /tmp/busy_rr");
						}
					}
					else {
*/
					if ((fr = fopen(rr_auths,"r"))<=0)
						error(4,4,FATAL);
					for (ra_cnt=0;fgets(s,CLEN,fr)!=NULL;ra_cnt++);
					fclose(fr);
					if ((fr = fopen(rr_labels,"r"))<=0)
						error(4,4,FATAL);
					for (rl_cnt=0;fgets(s,CLEN,fr)!=NULL;rl_cnt++);
					fclose(fr);
				}
				fclose(fr);
				if (i) {
					if (ra_cnt>rl_cnt) {
						printf("\007Note: Author addresses needed for a prior run!\nThese must be entered prior to making new requests.\n");
						get_rradds(rl_cnt); 
					}
					ed_append(RR);
					get_rradds(ra_cnt);
					printf("%s",*(mesg+9));
					if (!(getuid()&0377)) {
						/* Assume only the su will be
						generating the labels	*/
						mk_rrlabels();
						mk_rreqsts();
					}
					system("rm -f /tmp/busy_rr");
				}
			}
			break;

	}
}

gen_kwl()
{
	char s[128];
	sprintf(s,"/usr/bin/gen_kwl %s %s %d",db_name,kw_name,debug);
	system(s);
}
index(s,t)
char s[RLEN], t[RLEN];
{
	int i,j,k;
	for (i=0;*(s+i) != '\0';i++) {
		for (j=i,k=0;*(t+k)!='\0' && (*(s+j) == *(t+k) || *(t+j) == '*');j++,k++) ;
		if(*(t+k) == '\0') return(i);
	}
	return(-1);
}

/************************************************************************
 *									*
 *		REPLACE.C     03/11/82      S. Klyce			*
 *									*
 *	This is a function that will replace occurrences of an		*
 *  "old" phrase with a "new" phrase in a "s"tring.  Replacement	*
 *  continues until all "old" are substituted (n=0) or until		*
 *  n occurrences have been replaced.  NOTE that passing a HUGE 	*
 *  n will have the same effect as passing 0.				*
 *									*
 *  USE:  replace(string,old_phrase,new_phrase,how_many);		*
 *									*
 ************************************************************************/

replace(s,old,new,n)
char old[CLEN], new[CLEN];
char s[RLEN];
int n;
{
	register i,j,k;
	char t[RLEN];
	int ind,cnt=0;
	while ((ind=index(s,old)) >= 0 && (!n || (cnt++ < n))) {

		for (i=j=k=0;i<ind;i++,j++) *(t+i) = *(s+j);
		for (j += slen(old);k < slen(new); i++,k++) *(t+i) = *(new+k);
		while ((*(t+i++) = *(s+j++)) != '\0');
		for (i=0;((*(s+i) = *(t+i)) != '\0');i++);
	}
}
/*******************************************************/
/***   STRINP.C     Feb 18, 1982    S. KLYCE         ***/
/*******************************************************/

/* #include <stdio.h> */
#define MINASCII	'\037'   /**  These two lines           **/
#define MAXASCII	'\176'   /** system dependent           **/

strinp(s,svar,lim)
char *s, *svar;
int lim;
{
	int c,i;
	int error = 1;
	while (error) {
		printf("%s",s);
		error=i=0;
		while (--lim > 0 && (c=getchar()) != EOF && c != '\n')
		{
			if ((c < MINASCII || c > MAXASCII) && c != '\n') error=1;
			svar[i++] = c;
		}
		if (c == '\n')
			svar[i++] = '|';
				/* Above is a variation to strinp.c for */
				/* RIPS for field separation in the	*/
				/* reference file.			*/
		svar[i] = '\0';
	}
}
slen(s)
char s[RLEN];
{
	char *p = s;
	while(*p != '\0') p++;
	return(p-s);
}


lock()  /* Here we check to see if the file is currently busy */
	/* If not, we busy it out.				   */
{
	char lockfile[CLEN];
	sprintf(lockfile,"/tmp/busy_%s",dbn);
	if ((fl=fopen(lockfile,"r"))<=0) {
		fl = fopen(lockfile,"w");
		fprintf(fl,"BUSY");
		fclose(fl);
		return(1);
	}
	return(-1);
}

ulock()	/*  Remove the lock file when done editing a file	*/
	/* except when ed_appending as unlocking will be taken	*/
	/* care of by kwsort.					*/
{
	char unlock[CLEN];
	sprintf(unlock,"rm -f /tmp/busy_%s",dbn);
	system(unlock);
}

date(c)	/*  Get the time and date	*/
char *c;
{
	register i;
	char *s;
	int tbuf[2];
	time(tbuf);
	s = ctime(tbuf);
	while ((*(c+i) = *(s+i++))!='\0');
}

check_db()
{
	char s[CLEN];
	sprintf(s,"/usr/bin/rips_chk %s",db_name);
	printf("%s",*(mesg+9));
	system(s);
}

#define LLEN		32
#define MAXL		5

get_rradds(rcnt)
int rcnt;
{
	auto char addr[MAXL][LLEN];
	auto char zip[YLEN];
	auto char item[TLEN];
	auto char who[ALEN];
	auto char ans[6];
	register i,j,k;
	int zerr;
	int id;
	if ((fp=fopen(rr_labels,"a"))<=0)
		error(5,5,FATAL);
	if ((fl=fopen(rr_auths,"r"))<=0)
		error(5,3,FATAL);
	else {
		for (i=0;i<rcnt;i++)
			fgets(who,ALEN,fl);
		while (fgets(who,ALEN,fl)!=NULL) {
			zerr=1;
			while (zerr) {
				printf("Enter address for:\n");
				for (i=0;i<LLEN+6;i++) printf("-");
				printf("|\n");
				printf("l1\t%s",who);
				for (zerr=i=0;i<MAXL-1;i++) {
					printf("l%1d",i+2);
					strinp("\t",addr[i],LLEN);
					replace(*(addr+i),"|","\n",1);
					if (*(*(addr+i))=='\n')
						break;
				}
				printf("%s",who);
				for (j=0;j<i;j++)
					printf("%s",addr[j]);
				lf((MAXL-j),stdout);
				strinp(*(mesg+8),ans,8);
				switch (*ans) {

					case 'y':
					case '|':
						sprintf(item,"%s",who);
						for (j=0;j<i;j++) {
							strcat(item,addr[j]);
						}
						replace(item,"\n",":",0,0);
						id=getuid()&0377;
						fprintf(fp,"%3d   %s\n",id,item);
						break;
					default:
						printf("%s",*(mesg+15));
						zerr=1;
						break;
				}
			}
		}
	}
	fclose(fp);
	fclose(fl);
}
mk_rrlabels()
{
	register i,j;
	int cnt = 0;
	char who[128];
	char ans[8];
	auto char item[TLEN];
	if ((fs=fopen(rr_labels,"r"))<=0)
		error(6,3,NONFATAL);
	else if ((fu=fopen("/usr/lib/rips/rr/labels","w"))<=0)
			error(6,5,NONFATAL);
	else {
		putdoc(fu,0);
		while (fgets(item,TLEN,fs) != NULL) {
			replace(item,":","\n",0,0);
			for (i=0;i<6;i++)
				*(item+i) = 'x';
			replace(item,"xxxxxx","",1,0);
			for (i=cnt=0;*(item+i)!='\0';i++)
				if (*(item+i)=='\n') cnt++;
			fprintf(fu,"%s",item);
			lf((MAXL-cnt),fu);
		}
		putdoc(fu,0);
		fclose(fs);
		fclose(fu);
	}
}
putdoc(fildes,type)
int fildes,type;
{
	register i;
	fprintf(fildes,"\n%s%s",*(mesg+11),*(mesg+12));
	if (type==RRFILE)
		fprintf(fildes,"CARDS ");
	else fprintf(fildes,"LABELS");
	fprintf(fildes,"%s%s%s%s\n",*(mesg+13),*(mesg+14),*(mesg+13),*(mesg+11));
	if (type==RRFILE)
		lf(16,fildes);
}
get_perm(file)
char file[128];
{
	int uid,own;
	char who_name[48];
	if (stat(file,&statbuf)<0)
		error(1,5,FATAL);
			/*  Pick off the stats from the file file	*/
	uid=getuid()&0377;
	own=statbuf.uid&0377;
	if (uid != own && uid) {
		get_who(own,who_name);
		error(1,6,NONFATAL);
		printf("Only %s can alter this file.\nYou can, however, continue\n",who_name);
		printf("by working in a file owned by you.\n");
			/*  Let no one but the owner or the s.u. edit	*/
		return(1);
	}
	return(0);
}
mk_rreqsts()
{
	register i,j;
	int uid, cnt = 0;
	char who_name[48];

	char addr[TLEN], taddr[TLEN];
	if ((fp=fopen(rrequests,"r"))<=0)
		error(7,3,FATAL);
	if ((fr=fopen(rr_auths,"r"))<=0)
		error(7,3,FATAL);
	if ((fu=fopen(rr_labels,"r"))<=0)
		error(7,3,FATAL);
	if ((fs=fopen(rr_cards,"w"))<=0)
		error(7,5,FATAL);
	putdoc(fs,RRFILE);
	while (fgets(addr,TLEN,fu) != NULL) {
		sscanf(addr,"%d",&uid);
		for (cnt=j=0,i=slen(addr)-4;*(addr+i)!=':';i--);
		while ((*(taddr+j++) = lower(*(addr+i++))) != '\0');
		for (*addr='\n',*(taddr+j)='\0',i=0;i<NUM_CTYS;i++)
			if (index(taddr,c[i].country)>0) break;
/*
		c[i].lang=0;*/  /* remove this for multilanguage printing */
		fprintf(fs,"\t%s\n\n\t   %s\n\n",p[c[i].lang].dear,p[c[i].lang].leadin);
		while (*addr=='\n') fgets(addr,CARD_WIDTH+10,fp);
		fprintf(fs,"\t%s",addr);
		while (*addr!='\n') {
			fgets(addr,CARD_WIDTH+10,fp);
			fprintf(fs,"\t%s",addr);
			cnt++;
		}
		fprintf(fs,"\t   %s",p[c[i].lang].signoff);
		lf(8-cnt,fs);
		get_who(uid,who_name);
		fprintf(fs,"\t\t      ________________________________________\n");
		fprintf(fs,"\t\t     |  %s\n\t\t     |  LSU School of Medicine",who_name);
		fprintf(fs,"\n\t\t     |  Ophthalmology/LSU Eye Center");
		fprintf(fs,"\n\t\t     |  136 South Roman Street");
		fprintf(fs,"\n\t\t     |  New Orleans, LA  70112  U.S.A.\n\n\n");
	}
	putdoc(fs,RRFILE);
	fclose(fp);
	fclose(fr);
	fclose(fs);
	fclose(fu);
}
get_who(id,name)
int id;
char *name;
{
	auto char line[DLEN];
	auto char user[48];
	register i,j;
	if ((fl=fopen("/etc/passwd","r"))<=0)
		error(8,3,FATAL);
	while (fgets(line,DLEN,fl)!=NULL) {
		for (j=i=0;i<2;i++) while (*(line+j++) != ':');
		for (i=0;(*(user+i)= *(line+j++))!=':';i++);
		*(user+i)='\0';
		if (id==atoi(user)) {
			while (*(line+j++)!=':');
			i=0;
			while((*(name+i++)= *(line+j++)) != ':');
			*(name+i-1) = '\0';
			fclose(fl);
			return(1);
		}
	}
	error(8,6,FATAL);
}

slowcpy(s,t)  /* copy lower case t to s  */
char *s, *t;
{
	while(*s++ = lower(*t++));
}
stat_kwl(dfile,kfile)
char dfile[128], kfile[128];
{
	register i;
	int dmt[2];
	char *s;
	if (stat(dfile,&statbuf)<0)
		error(0,2,NONFATAL);
	dmt[0]=statbuf.modtime[0];
	dmt[1]=statbuf.modtime[1];
	if (stat(kfile,&statbuf)<0)
		error(0,2,NONFATAL);
/*
printf("dfile:%d %d\tkfile:%d %d\n",dmt[0],dmt[1],statbuf.modtime[0],statbuf.modtime[1]);
*/
	if ((statbuf.modtime[0]<dmt[0])||(statbuf.modtime[0]==dmt[0] && statbuf.modtime[1]<dmt[1]))
	{
		s=ctime(statbuf.modtime);
		printf("Keyword list not current.  Last update %s\n",s);
		do switch (*s) {
			case 'y':
				return(1);
				break;
			case 'n':
			case '|':
				return(0);
				break;
			default:
				strinp("Still want to print keywords ? ",s,6);
				break;
		} while (*s!='y' && *s!='n' && *s!='|');
	}
	return(1);
}
not(c)
char c;
{
	if (c=='1') return('0');
	return('1');
}
lf(n,f)
int n,f;
{
	while(n-- >= 0) putc('\n',f);
}
