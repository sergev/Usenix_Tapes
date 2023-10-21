h42197
s 00504/00000/00000
d D 1.1 86/07/18 21:10:41 brandon 1 0
c Converted to SCCS 07/18/86 21:10:41
e
u
U
f b 
f q 0.4.5
f t UNaXcess
t
T
I 1
echo ""
echo "UNaXcess V%Q% Installation Script"
echo "By Brandon S. Allbery"
echo ""
echo "You are free to copy, modify, etc. this installation script as much as you"
echo "wish; it's bound to come in handy if anyone on your system does programming."
echo "This script should run on most Unix-based systems.  If it doesn't, PLEASE let"
echo "me know at ...decvax!cwruecmp!ncoast!allbery."
echo ""
echo "I will be asking you for information as I install UNaXcess.  When I do, I'll"
echo "let you know by ending the question with two arrows, like this:  -> ->"
echo "When I do this, it's a signal for you to tell me something.  Usually, pressing"
echo "RETURN will do something reasonable."
echo ""
:
: Note to hackers -- the original idea for this script hails from lwall@sdcrdcf
: but this script is entirely mine.  To see the original, ask anyone with rn
: sources.
:
: Yes, the "-> ->" business is copped from Unify.  I sincerely doubt that it is
: copyrighted, and it is fairly obvious.
:

grep NOAUTOPATH Makefile > /dev/null 2>&1
case "$#" in
0)	autopath=y
	;;
1)	autopath=n
	UAHOME="`grep NOAUTOPATH Makefile | sed -n 's/.*[ 	]-DAUTOPATH=[^-A-Za-z\/.]*\([-A-Za-z\/.][-A-Za-z\/.]*\).*$/\1/p`"
	if test ! -w "$UAHOME"; then
		echo "I can't write in $UAHOME.  Please check your NOAUTOPATH definition"
		echo "in the Makefile and recompile, or create the directory."
		exit 1
	fi
	UAOWNER=${LOGNAME:-${USER:?'Who are you?'}}
	;;
2)	echo "I'm not being run from the UNaXcess distribution directory."
	echo "Please re-run install.sh from the installation directory."
	exit 1
	;;
*)	echo "Your grep does funny things.  You'll have to install UNaXcess by hand."
	exit 1
	;;
esac
case $autopath in
y)	if test ! -w /etc/passwd; then
		echo "I'm not running as root.  Please ask a system administrator to run this install"
		echo "script as root, as I need to create directories and possibly even a user name."
		exit 1
	fi
	: end of test for root.  This is about as portable as you get.
	echo "Please enter the username of the owner of this UNaXcess system.  -> ->"
	read UAOWNER
	case "$UAOWNER" in
	"")	echo "I hope you wanted to abort, because you just did."
		exit 1
	esac
	if grep "^$UAOWNER:" /etc/passwd >/dev/null 2>&1; then
		echo "I see $UAOWNER already exists.  Do you want to use him?  -> ->"
		read yesno
		case "$yesno" in
		y*|Y*)	echo "Okay, we use $UAOWNER as the owner of UNaXcess."
			;;
		*)	echo "Then we'll start again from scratch."
			exec sh Utilities/install.sh
		esac
	else
		echo "I don't see a user named $UAOWNER on this system.  Should I create one?  -> ->"
		read yesno
		case "$yesno" in
		y*|Y*)	echo "Okay, we make him."
			;;
		*)	echo "Then we'll start again from scratch."
			exec sh Utilities/install.sh
		esac
		UAUID=`sed 's/^[^:]*:[^:]*:\([^:]*\):.*$/\1/' < /etc/passwd | sort -n | tail -1`
		UAUID=`expr $UAUID + 1`
		echo "I'm going to call $UAOWNER's home directory /usr/$UAOWNER."
		echo "If you have a better idea, please tell me what it is.  -> ->"
		read home
		case "$home" in
		"")	echo "Ok, we use /usr/$UAOWNER."
			home=/usr/$UAOWNER
		esac
		if test -r $home; then
			echo "It seems that this particular name's in use.  Let's try again."
			exec sh Utilities/install.sh
		fi
		echo "$UAOWNER::$UAUID:1:UNaXcess Control Login:$home:" >> /etc/passwd
		mkdir $home
		chown $UAOWNER $home
		echo "Now give $UAOWNER a password.  DON'T LEAVE IT UNPROTECTED!!!"
		passwd $UAOWNER
	fi
	echo ""
	UAHOME="`sed -n '/^'$UAOWNER':/s=^[^:]*:[^:]*:[^:]*:[^:]*:[^:]*:\([^:]*\):.*\$=\1=p' < /etc/passwd`"
	;;
n)	;;
esac

echo "UNaXcess will be installed in $UAHOME.  Is this okay?  -> ->"
read yesno
case "$yesno" in
y*|Y*)	echo "Then we'll proceed with the installation."
	;;
*)	echo "Please check /etc/passwd for $UAOWNER's home directory, and fix it."
	echo "Then type 'sh Utilities/install.sh' to try installation again."
	exit 1
esac
chmod 711 $UAHOME || echo "Please inspect the permissions on $UAHOME; they should be -rwx--x--x."
: now we know the home and owner of UNaXcess

if test ! -d $UAHOME/msgdir; then
	echo "Creating $UAHOME/msgdir..."
	rm -rf $UAHOME/msgdir
	mkdir $UAHOME/msgdir
	chown $UAOWNER $UAHOME/msgdir
fi
if test ! -d $UAHOME/motd; then
	echo "Creating $UAHOME/motd..."
	rm -rf $UAHOME/motd
	mkdir $UAHOME/motd
	chown $UAOWNER $UAHOME/motd
fi
if test ! -d $UAHOME/library; then
	echo "Creating $UAHOME/library..."
	rm -rf $UAHOME/library
	mkdir $UAHOME/library
	chown $UAOWNER $UAHOME/library
fi
if test ! -d $UAHOME/uploads; then
	echo "Creating $UAHOME/uploads..."
	rm -rf $UAHOME/uploads
	mkdir $UAHOME/uploads
	chown $UAOWNER $UAHOME/uploads
fi
echo "Creating $UAHOME/Logfile..."
rm -f $UAHOME/Logfile
chown $UAOWNER $UAHOME/Logfile > $UAHOME/Logfile
if test ! -f $UAHOME/userind; then
	echo "Creating $UAHOME/userind..."
	rm -rf $UAHOME/userind
	chown $UAOWNER $UAHOME/userind > $UAHOME/userind
fi
if test ! -d $UAHOME/msgdir/general; then
	echo "Creating conference general..."
	rm -rf $UAHOME/msgdir/general
	mkdir $UAHOME/msgdir/general
	chown $UAOWNER $UAHOME/msgdir/general
	echo 1 > $UAHOME/msgdir/general/himsg
	chown $UAOWNER $UAHOME/msgdir/general/himsg
	cat << --general/1-- > $UAHOME/msgdir/general/1
Date: `Utilities/udate`
From: The UNaXcess System Operator
To: All
Subject: This BBS

Welcome to UNaXcess V0.04.04 -- a computer bulletin board system designed for
the UNIX(tm) operating system.  You are using the ONLY BBS which is designed
to USE Unix's unique features to their fullest potential.

This conference is the general-purpose conference.  Use it for discussions
that don't fit in any of the existing conferences.  But if it's liable to be
large, consider making a new conference for it.

The basic UNaXcess commands are:

	r	Read messages.  You may specify a message number on the
		command line, or you will be asked how to read messages.
	
	n	Read all new messages in all conferences.
	
	e	Enter a new message.  You can specify the recipient's
		name on the command line, like "e John Doe".
	
	j	Join a different conference.  You can also create a new
		conference with this command.  You can specify the new
		conference on the command line:  "j discussion".
	
	g	Exit UNaXcess.
	
	f	Enter the File Section.  UNaXcess is capable of file
		uploading and downloading with ASCII, Xmodem and Kermit
		(not all maybe available on all systems).

Press BREAK (or DEL or CONTROL C, depending on the system) to stop a command.
Press CONTROL-\ (CONTROL plus backslash) for a fast logout.

--general/1--
fi
if test ! -d $UAHOME/msgdir/guest; then
	echo "Creating conference guest..."
	rm -rf $UAHOME/msgdir/guest
	mkdir $UAHOME/msgdir/guest
	chown $UAOWNER $UAHOME/msgdir/guest
	echo 1 > $UAHOME/msgdir/guest/himsg
	chown $UAOWNER $UAHOME/msgdir/guest/himsg
	cat << --guest/1-- > $UAHOME/msgdir/guest/1
Date: `Utilities/udate`
From: The UNaXcess System Operator
To: All
Subject: This conference

This conference is for any and all users of UNaXcess.  ANYONE may post
messages to the guest conference at any time.

--guest/1--
fi
: end of system files build, for the most part

echo "Where do you want the programs to go?  ENTER defaults to /usr/bin.  -> ->"
read BIN
case "$BIN" in
"")	echo "I'll use /usr/bin, then."
	BIN=/bin
	;;
*)	echo "Ok, I'll put them in $BIN."
	;;
esac
if test ! -d $BIN; then
	echo "I can't find $BIN.  Please start over after you make it."
	exit 1
fi
echo "What do you want to call UNaXcess?  ENTER defaults to ua.  -> ->"
read ua
case "$ua" in
"")	ua="ua"
esac
echo "What do you want to call the user lister?  ENTER defaults to uwho.  -> ->"
read uwho
case "$uwho" in
"")	uwho="uwho"
esac
echo "What do you want to call the date printer?  Enter defaults to udate.  -> ->"
read udate
case "$udate" in
"")	udate="udate"
esac
echo "Creating $BIN/$ua..."
cp ua $BIN/$ua
case $autopath in
y)	chown $UAOWNER $BIN/$ua
	chmod 4711 $BIN/$ua
	;;
esac
echo "Creating $BIN/$udate..."
cp Utilities/udate $BIN/$udate
chown $UAOWNER $BIN/$udate
chmod 711 $BIN/$udate
echo "Creating $BIN/$uwho..."
cp Utilities/uwho $BIN/$uwho
case $autopath in
y)	chown $UAOWNER $BIN/$uwho
	chmod 4711 $BIN/$uwho
	echo "Creating $UAHOME/mkconf..."
	cp Utilities/mkconf $UAHOME/mkconf
	chown root $UAHOME/mkconf
	chmod 4711 $UAHOME/mkconf
	;;
esac
: end of binary installation

echo "What do you want to call the shell message poster?  ENTER defaults to upost."
echo "  -> ->"
read upost
case "$upost" in
"")	upost="upost"
esac
echo "Creating $BIN/$upost..."
sed -e "s;@UAOWNER@;${UAOWNER};" -e "s;@udate@;BIN/${udate};" < Utilities/upost.sh > $BIN/$upost
chown $UAOWNER $BIN/$upost
chmod 755 $BIN/$upost
echo "What do you want to call the shell bulletin poster?  ENTER defaults to umotd."
echo "  -> ->"
read umotd
case "$umotd" in
"")	umotd="umotd"
esac
echo "Creating $BIN/$umotd..."
sed -e "s;@UAOWNER@;${UAOWNER};" -e "s;@udate@;$BIN/${udate};" < Utilities/umotd.sh > $BIN/$umotd
chown $UAOWNER $BIN/$umotd
chmod 755 $BIN/$umotd
echo "What do you want to call the message mover?  ENTER defaults to mvmsg."
echo "  -> ->"
read mvmsg
case "$mvmsg" in
"")	mvmsg="mvmsg"
esac
echo "Creating $BIN/$mvmsg..."
cp Utilities/mvmsg $BIN/$mvmsg
chown $UAOWNER $BIN/$mvmsg
chmod 4700 $BIN/$mvmsg
echo "Do you want to install the USENET News Transfer Facility?  ENTER defaults"
echo " to NO.  -> ->"
read instnews
case "$instnews" in
y*|Y*)	echo "What do you want to call the transfer program?  ENTER defaults to"
	echo "uanews.  -> ->"
	read uanews
	case "$uanews" in
	"")	uanews="uanews"
	esac
	echo "Creating $BIN/$uanews..."
	cp Utilities/uanews $BIN/$uanews
	case $autopath in
	y)	chown $UAOWNER $BIN/$uanews
		chmod 4700 $BIN/$uanews
		;;
	esac
esac
: end of shell utilities installation

echo ""
echo "Now I'm going to create the runtime configuration file for UNaXcess.  This"
echo "file allows you to decide what features of UNaXcess are allowed in your BBS"
echo "system."
echo ""
echo "I suggest you read the UNaXcess README file before doing this, so you'll"
echo "know what I'm talking about."
echo ""
echo "Do you want to allow READ-ONLY conferences?  Press RETURN for YES.  -> ->"
read yesno
case "$yesno" in
""|y*|Y*)
	readonly=YES
	;;
*)	readonly=NO
esac
echo "Do you want to allow RESTRICTED (X-Rated) conferences?  Press RETURN for NO."
echo "  -> ->"
read yesno
case "$yesno" in
y*|Y*)
	xrated=YES
	;;
*)	xrated=NO
esac
echo "What is the editor to be run?  The default is the internal editor.  -> ->"
read editor
case "$editor" in
"")	editor=ua-edit
esac
echo "What is the shell to be run by the COMMAND option?  The default is /bin/sh."
echo "  -> ->"
read shell
case "$shell" in
"")	shell=/bin/sh
esac
echo "Do you want to have UNaXcess check the user's shell and editor preferences"
echo "via the EDITOR and SHELL environment variables?  The default is YES.  -> ->"
read env
case "$env" in
N*|n*)	env=NO
	;;
*)	env=YES
esac
echo "What is the login name of the user who can't log directly into UNaXcess?"
echo "RETURN if all users can log in directly  -> ->"
read bbsuser
echo "What is the time limit on UnaXcess, in minutes?  -> ->"
read tlimit
case "$tlimit" in
"")	tlimit=0
esac
echo "What is the system operator's UNaXcess login to be?  RETURN for SYSOP  -> ->"
read sysop
case "$sysop" in
"")	sysop="sysop"
esac
echo "Do you want to allow private messages?  RETURN for YES  -> ->"
read private
case "$private" in
N*|n*)	private=NO
	;;
*)	private=YES
esac
echo "Do you want to enable logging?  RETURN for YES  -> ->"
read logging
case "$logging" in
N*|n*)	logging=NO
	;;
*)	logging=YES
esac
echo "Please specify one of the following choices:"
echo "    nopause     Don't pause after 16 lines of output"
echo "    ask         Ask each user whether he wants puses or not"
echo "    pause       Pause after 16 lines of output always"
echo "Please select one, or press RETURN for ASK:  -> ->"
read pauses
case "$pauses" in
N*|n*)	pauses=2
	;;
P*|p*)	pauses=0
	;;
*)	pauses=1
esac
echo "How many tries should a user get to log in?  RETURN for 3  -> ->"
read logtries
case "$logtries" in
"")	logtries=3
esac
echo ""
echo "Creating $UAHOME/ua-config..."
cat << --CONFIG-- > $UAHOME/ua-config
readonly	$readonly
x-rated		$xrated
editor		$editor
shell		$shell
read-env	$env
bbs-user	$bbsuser
time-limit	$tlimit
sysop		"$sysop"
private-msgs	$private
logging		$logging
banner		""
login-msg	""
pauses		$pauses
login-tries	$logtries
--CONFIG--
chown $UAOWNER $UAHOME/ua-config
chmod 644 $UAHOME/ua-config
echo ""
echo "You can edit the config file yourself to change these and other options."
: end of config file generation

echo ""
if test ! -f $UAHOME/userfile; then
	echo "Creating $UAHOME/userfile..."
	rm -rf $UAHOME/userfile
	cat << --USERFILE-- > $UAHOME/userfile
$sysop::5::80:0:
new::6::2:0:
guest::1::40:0:
--USERFILE--
	chown $UAOWNER $UAHOME/userfile
	chmod 644 $UAHOME/userfile
fi
: end of userfile installation

echo ""
if test ! -d $UAHOME/lock; then
	echo "Making lock directory..."
	rm -rf $UAHOME/lock
	mkdir $UAHOME/lock
	chown $UAOWNER $UAHOME/lock
	chmod 777 $UAHOME/lock
fi
if test ! -r $UAHOME/directory; then
	rm -rf $UAHOME/directory
	today="`date | sed -e 's/^... \(...\)  *\([0-9][0-9]*\) [^ ][^ ]* 19\(..\)$/\1\/\2\/\3/'`"
	today="`echo $today | sed -e s/Jan/1/ -e s/Feb/2/ -e s/Mar/3/ -e s/Apr/4/ -e s/May/5/ -e s/Jun/6/`"
	today="`echo $today | sed -e s/Jul/7/ -e s/Aug/8/ -e s/Sep/9/ -e s/Oct/10/ -e s/Nov/11/ -e s/Dec/12/`"
	echo "Creating $UAHOME/directory..."
	cat << --LIBCATLG-- > $UAHOME/directory
GENERAL file branch; $today by $sysop: General up/downloading
--LIBCATLG--
	chown $UAOWNER $UAHOME/directory
	chmod 644 $UAHOME/directory
fi
if test ! -f $UAHOME/upload-log; then
	rm -rf $UAHOME/upload-log
	echo "Creating $UAHOME/upload-log..."
	cp /dev/null $UAHOME/upload-log
	chown $UAOWNER $UAHOME/upload-log
	chmod 644 $UAHOME/upload-log
fi
: end of directory installation

if test ! -f $UAHOME/NewMessage; then
	echo ""
	echo "I am going to create a simple new-user bulletin.  It is your job to change it"
	echo "to say what you want it to say."
	echo ""
	echo "Creating $UAHOME/NewMessage..."
cat << --NEWMESSAGE-- > $UAHOME/NewMessage

Welcome to the UNaXcess System, Copyright (C) 1984, 1985 by Brandon S. Allbery.
This UNaXcess was installed `Utilities/udate`.

--NEWMESSAGE--
	chown $UAOWNER $UAHOME/NewMessage
	chmod 644 $UAHOME/NewMessage
fi
if test ! -f $UAHOME/motd/himotd; then
	echo "Creating $UAHOME/motd/himotd..."
	echo 0 > $UAHOME/motd/himotd
	chown $UAOWNER $UAHOME/motd/himotd
	chmod 644 $UAHOME/motd/himotd
fi
echo ""
echo "Now you must create the main bulletin.  I will run the $umotd program"
echo "for you -- you will simply enter the bulletin.  I hope you have EDITOR set"
echo "up correctly!"
echo ""
$BIN/$umotd
: end of bulletin installation

echo ""
echo "Congratulations!  You have just successfully installed the UNaXcess System."
echo "Before you use it, I suggest you put a password on the '$sysop' login,"
echo "so nobody else can make trouble with it."
echo ""
$BIN/$udate
echo "UNaXcess Version 0.04.04 has been installed."
E 1
