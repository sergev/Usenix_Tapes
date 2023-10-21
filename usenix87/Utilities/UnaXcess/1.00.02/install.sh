:
: install.sh
: install UNaXcess 1.00.02
: Brandon S. Allbery
: This script is not copyrighted
:

PATH=:/bin:/usr/bin:/usr/ucb:/etc:/usr/plx
cat - << --EOF--

		    UNaXcess 1.00.02 Installation Script
		    ------------------------------------

	This script requires that (1) you have NOAUTOPATH defined, or
	(2) you be running as root.  If you are not running as root,
	the "mkconf" program cannot be installed.

	(On BSD systems, root permissions are required only if the
	UNaXcess owner does not exist.  Note that if you are not running
	as root, you will receive spurious error messages from chown.
	You may ignore these messages.)

--EOF--
grep NOAUTOPATH Makefile > /dev/null 2>&1
case "$?" in
1)      autopath=y
	;;
0)      autopath=n
	UAHOME="`grep NOAUTOPATH Makefile | sed -n 's/.*[       ]-DAUTOPATH=[^-A-Za-z\/.]*\([-A-Za-z\/.][-A-Za-z\/.]*\).*$/\1/p'`"
	if test ! -w "$UAHOME"; then
		echo "No write permission in $UAHOME.  Installation aborted."
		exit 1
	fi
	UAOWNER=${LOGNAME-${USER?'Who are you?'}}
	;;
2)      echo "Please re-run install.sh from the installation directory."
	exit 1
	;;
*)      echo "Your grep does funny things.  You'll have to install UNaXcess by hand."
	exit 1
	;;
esac
case $autopath in
y)      if test ! -f /vmunix -a ! -w /etc/passwd; then
		echo "Root permissions are required.  Installation aborted."
		exit 1
	fi
	echo "Please enter the username of the owner of this UNaXcess system."
	echo "(Press RETURN to abort installation.)"
	read UAOWNER
	case "$UAOWNER" in
	"")     echo "Installation aborted."
		exit 1
		;;
	esac
	if grep "^$UAOWNER:" /etc/passwd >/dev/null 2>&1; then
		echo "User $UAOWNER already exists.  Do you want to use it (Y/N)?"
		read yesno
		case "$yesno" in
		y*|Y*)  ;;
		*)      echo "Installation aborted."
			exit 1
			;;
		esac
	else
		if test -f /vmunix -a ! -w /etc/passwd; then
			echo "Please create the user and re-run install.sh.  Installation aborted."
			exit 1
		fi
		echo "Create $UAOWNER (Y/N)?"
		read yesno
		case "$yesno" in
		y*|Y*)  ;;
		*)      echo "Installation aborted."
			exit 1
			;;
		esac
		UAUID=`sed 's/^[^:]*:[^:]*:\([^:]*\):.*$/\1/' < /etc/passwd | sort -n | tail -1`
		UAUID=`expr $UAUID + 1`
		echo "The BBS directory will be /usr/$UAOWNER.  Press RETURN to accept, or"
		echo "enter a directory name."
		read home
		case "$home" in
		"")     home=/usr/$UAOWNER
			;;
		esac
		if test -r $home; then
			echo "$home exists.  Please choose a different directory.  Installation aborted."
			exit 1
		fi
		echo "$UAOWNER::$UAUID:1:UNaXcess Control Login:$home:" >> /etc/passwd
		mkdir $home
		chown $UAOWNER $home
		echo "Please give $UAOWNER a password.  DON'T LEAVE IT UNPROTECTED!!!"
		passwd $UAOWNER
	fi
	echo ""
	UAHOME="`sed -n '/^'$UAOWNER':/s=^[^:]*:[^:]*:[^:]*:[^:]*:[^:]*:\([^:]*\):.*\$=\1=p' < /etc/passwd`"
	;;
n)      ;;
esac
echo "UNaXcess will be installed in $UAHOME.  Is this okay (Y/N)?"
read yesno
case "$yesno" in
y*|Y*)  ;;
*)      echo "Please check /etc/passwd for $UAOWNER's home directory, and fix it."
	echo "Then type 'sh install.sh' to try installation again."
	exit 1
	;;
esac
#chmod 711 $UAHOME || echo "Please inspect the permissions on $UAHOME; they should be -rwx--x--x."
if test ! -d $UAHOME/msgdir; then
echo "Creating $UAHOME/msgdir..."
	rm -f $UAHOME/msgdir
	mkdir $UAHOME/msgdir
	chown $UAOWNER $UAHOME/msgdir
fi
if test ! -d $UAHOME/motd; then
	echo "Creating $UAHOME/motd..."
	rm -f $UAHOME/motd
	mkdir $UAHOME/motd
	chown $UAOWNER $UAHOME/motd
fi
if test ! -d $UAHOME/library; then
	echo "Creating $UAHOME/library..."
	rm -f $UAHOME/library
	mkdir $UAHOME/library
	chown $UAOWNER $UAHOME/library
fi
if test ! -d $UAHOME/uploads; then
	echo "Creating $UAHOME/uploads..."
	rm -f $UAHOME/uploads
	mkdir $UAHOME/uploads
	chown $UAOWNER $UAHOME/uploads
fi
echo "Creating $UAHOME/Logfile..."
rm -f $UAHOME/Logfile
chown $UAOWNER $UAHOME/Logfile > $UAHOME/Logfile
if test ! -f $UAHOME/userind; then
	echo "Creating $UAHOME/userind..."
	rm -rf $OAHOME/userind
	chown $UAOWNER $UAHOME/userind > $UAHOME/userind
fi
if test ! -d $UAHOME/lock; then
	echo "Creating $UAHOME/lock..."
	rm -f $UAHOME/lock
	mkdir $UAHOME/lock
	chown $UAOWNER $UAHOME/lock
fi
if test ! -d $UAHOME/members; then
	echo "Creating $UAHOME/members..."
	rm -f $UAHOME/members
	mkdir $UAHOME/members
	chown $UAOWNER $UAHOME/members
fi
if test ! -d $UAHOME/msgdir/general; then
	echo "Creating conference general..."
	rm -f $UAHOME/msgdir/general
	mkdir $UAHOME/msgdir/general
	chown $UAOWNER $UAHOME/msgdir/general
	echo 1 > $UAHOME/msgdir/general/himsg
	chown $UAOWNER $UAHOME/msgdir/general/himsg
	cat << --general/1-- > $UAHOME/msgdir/general/1
Date: `./udate`
From: SYSOP
To: All
Subject: This BBS

Welcome to UNaXcess V1.00.02 -- a computer bulletin board system designed for the UNIX(R) operating system.  You're using the ONLY BBS which is designed to USE Unix's unique features to their fullest potential.

This conference is the general-purpose conference.  Use it for discussions that don't fit in any of the existing conferences.  But if it's liable to be large, consider making a new conference for it.

The basic UNaXcess commands are:

M - Enter the Message Base menu.
G - Exit UNaXcess.
F - Enter the File UDL Section.  UNaXcess is capable of file uploading and downloading with ASCII, Xmodem and Kermit (not all may be available on all systems).
? - Help.  Press it once for one-line descriptions, press it again for the help file.

Press BREAK or CONTROL C to stop a command.
Press CONTROL-\ (CONTROL plus backslash) for a fast logout.

The UNaXcess User's Guide is available in the File UDL Area for those who have File UDL access.

--SYSOP
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
Date: `./udate`
From: SYSOP
To: All
Subject: This conference

This conference is for any and all users of UNaXcess.  ANYONE may post messages to the guest conference at any time.

--SYSOP
--guest/1--
fi
echo "The programs will be placed in /usr/bin.  Press RETURN to accept, or enter"
echo "a different directory."
read BIN
case "$BIN" in
"")	BIN=/usr/bin
	;;
*)	;;
esac
if test ! -d $BIN; then
	echo "$BIN does not exist.  Please create it.  Installation stopped."
	exit 1
fi
echo "The UNaXcess program will be called ua.  Press RETURN to accept, or enter"
echo "a different name."
read ua
case "$ua" in
"")	ua="ua"
esac
echo "The user lister will be called uwho.  Press ENTER to accept, or enter a"
echo "different name."
read uwho
case "$uwho" in
"")	uwho="uwho"
esac
echo "Creating $BIN/$ua..."
cp ua $BIN/$ua
chown $UAOWNER $BIN/$ua
chmod 711 $BIN/$ua
case $autopath in
y)	chmod 4711 $BIN/$ua
	;;
esac
echo "Creating $BIN/$uwho..."
cp uwho $BIN/$uwho
chmod 711 $BIN/$uwho
case $autopath in
y)      chown $UAOWNER $BIN/$uwho
	chmod 4711 $BIN/$uwho
	if test ! -f /vmunix; then
		echo "Creating $UAHOME/mkconf..."
		cp mkconf $UAHOME/mkconf
		chown root $UAHOME/mkconf
		chmod 4711 $UAHOME/mkconf
	fi
	;;
esac
echo "The conference maintenance utility will be called mvmsg.  Press RETURN to"
echo "accept, or enter a different name."
read mvmsg
case "$mvmsg" in
"")     mvmsg="mvmsg"
	;;
esac
echo "Creating $BIN/$mvmsg..."
cp mvmsg $BIN/$mvmsg
case $autopath in
y)	chown $UAOWNER $BIN/$mvmsg
	chmod 4700 $BIN/$mvmsg
	;;
esac
cat - << --CONF--

You will be asked a series of configuration questions.  If you have not read
the UNaXcess Installation Manual, please do so before answering them.  You
may hit DEL (or CONTROL-C or etc.) to exit this script if you need to.

--CONF--
echo "Do you want to allow READ-ONLY conferences (Y)?"
read yesno
case "$yesno" in
n*|N*)	readonly=NO
	;;
*)      readonly=YES
	;;
esac
echo "Do you want to allow restricted conferences (N)?"
read yesno
case "$yesno" in
y*|Y*)	xrated=YES
	;;
*)      xrated=NO
esac
echo "The default shell for the C command will be /bin/sh.  Press RETURN to accept,"
echo "or enter an alternative shell."
read shell
case "$shell" in
"")     shell=/bin/sh
	;;
esac
if test ! -f "$shell"; then
	echo "The shell $shell does not exist.  Installation stopped."
	exit 1
fi
echo "Do you want UNaXcess to check the user's environment for a preferred"
echo "shell (Y)?"
read env
case "$env" in
N*|n*)  env=NO
	;;
*)      env=YES
	;;
esac
echo "Enter the UNIX login names of all logins which will run UNaXcess as the"
echo "login shell.  Press RETURN if there will be none."
read bbsuser
while :; do
	echo "Enter the time limit for UNaXcess users, in minutes; 0 for no limit (30):"
	read tlimit
	case "$tlimit" in
	"")     tlimit=30
	esac
	check="`echo \"$tlimit\" | sed 's/[0-9]//gp'`"
	if test -n "$check" -o $tlimit -gt 32000; then
		echo "Please enter a number between 0 and 32000."
	else
		break
	fi
done
echo "The SYSOP's UNaXcess login name will be SYSOP.  Press RETURN to accept, or"
echo "enter a different name."
read sysop
case "$sysop" in
"")     sysop="sysop"
	;;
esac
sysop="`echo \"$sysop\" | sed 's/://g
			       s/^\(................................\).*$/\1/'`"
echo "Do you want to allow private messages (Y)?"
read private
case "$private" in
N*|n*)  private=NO
	;;
*)      private=YES
	;;
esac
echo "Do you want to enable logging (N)?"
read logging
case "$logging" in
Y*|y*)  logging=YES
	;;
*)      logging=NO
	;;
esac
while :; do
	echo "How many tries should a user get to log in -- 0 means no limit (3)?"
	read logtries
	case "$logtries" in
	"")     logtries=3
	esac
	check="`echo \"$logtries\" | sed 's/[0-9]//gp'`"
	if test -n "$check" -o "$logtries" -gt 32000; then
		echo "Please enter a number between 0 and 32000."
	else
		break
	fi
done
echo ""
echo "Creating $UAHOME/ua-config..."
cat << --CONFIG-- > $UAHOME/ua-config
readonly	$readonly
restricted	$xrated
shell		$shell
read-env	$env
bbs-user	$bbsuser
time-limit      $tlimit
sysop		"$sysop"
private-msgs    $private
logging		$logging
banner		""
login-msg	""
login-tries	$logtries
validation-level	1
--CONFIG--
chown $UAOWNER $UAHOME/ua-config
chmod 644 $UAHOME/ua-config
echo ""
echo "You can edit the config file yourself to change these and other options."
echo "Read the UNaXcess Administration manual for details."
echo ""
if test ! -f $UAHOME/userfile; then
	echo "Creating $UAHOME/userfile..."
	rm -rf $UAHOME/userfile
	cat << --USERFILE-- > $UAHOME/userfile
$sysop::5::80:0:general:24
new::6::2:0::0
guest::1::40:0:guest:16
--USERFILE--
	chown $UAOWNER $UAHOME/userfile
	chmod 644 $UAHOME/userfile
fi
echo ""
if test ! -r $UAHOME/directory; then
	rm -rf $UAHOME/directory
	today="at-install"	# OOPS! 2 lines deleted, this line changed
				# 2/3/87 ++bsa
	echo "Creating $UAHOME/directory..."
	cat << --LIBCATLG-- > $UAHOME/directory
GENERAL file branch; $today by $sysop: General up/downloading
GENERAL file GUIDE.UA; $today by $sysop: The UNaXcess User's Guide
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
if test ! -d $UAHOME/library/general; then
	echo "Creating File UDL branch general..."
	rm -f $UAHOME/library/general
	mkdir $UAHOME/library/general
	cp guide $UAHOME/library/general/guide.ua
fi
if test ! -f $UAHOME/NewMessage; then
	echo ""
	echo "Creating the new-user bulletin.  See the UNaXcess Administrator's Manual"
	echo "in order to change it."
	echo ""
	echo "Creating $UAHOME/NewMessage..."
cat << --NEWMESSAGE-- > $UAHOME/NewMessage

Welcome to UNaXcess 1.00.02, Copyright (C) 1984-1987 by Brandon S. Allbery.  This UNaXcess was installed `./udate`.

You are requested to register with your full name, and give your full address and phone number.

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
if test -d $UAHOME/help; then
	rm -rf $UAHOME/help
fi
echo "Creating $UAHOME/help..."
mkdir $UAHOME/help
cp mainmenu.help $UAHOME/help/mainmenu
cp msgbase.help $UAHOME/help/msgbase
cp adminmenu.help $UAHOME/help/adminmenu
cp readcmds.help $UAHOME/help/readcmds
echo ""
echo "UNaXcess Conferencing 1.00.02 Installation complete."
exit 0
