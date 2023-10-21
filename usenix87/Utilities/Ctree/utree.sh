:
: utree - Unix conference tree
: This program is loosely based on the Conference Tree program by the
: CommuniTree Group.
:

UTREE=%R%
PATH=:/bin:/usr/bin:%P%
export UTREE PATH
QUIT=n

trap '' 15 3 2
umask 0
cat <<%%%

The Unix Tree - A Simple Conference-Oriented Communications Package
Programs (ut*) by Brandon Allbery, September 18, 1984.

%%%
while [ $QUIT = n ]
do	echo -n "Enter command (R, I, A, B, G, ?): "
	read cmd argA argB argC
	case "$cmd" in
	r | R | read | READ )
		utread $argA $argB $argC ;;
	i | I | index | INDEX )
		utindex $argA $argB $argC ;;
	a | A | addto | ADDTO )
		utaddto $argA $argB $argC ;;
	b | B | browse | BROWSE )
		utbrowse $argA $argB $argC ;;
	g | G | goodbye | GOODBYE )
		QUIT=y ;;
	'?' )
		cat <<%%%

Unix Tree Commands
------------------
(Each command may be abbreviated to its first letter.)

READ	Read a conference
INDEX	Index a conference
ADDTO	Add to a conference
BROWSE	Browse a conference
GOODBYE	Exit the Unix Tree
?	Print this help message

The READ, INDEX, and BROWSE commands may use one of the following modifiers:

COMPLETE	Read the specified conference and all subconferences
AFTER days	Read all subconferences less than <days> days old

These modifiers may be abbreviated to their first letters.  Only one may be
used in a single command.

Examples:

	READ CONFERENCES
	INDEX CONFERENCES AFTER 5
	R CONFERENCES A 15

The commands may run slowly, as shell scripts are limited in speed.  You may
exit a command by typing a rubout.

%%%
		echo ;;
	* )
		echo "No such command." ;;
	esac
done
echo
echo "Goodbye -- call again soon!"
exit
