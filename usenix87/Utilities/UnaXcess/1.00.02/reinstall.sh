: reinstall script
: used to upgrade V1.00.00 to 1.00.01 or later
cd "$1" || {
	echo "usage: sh reinstall.sh bbs-directory" >&2
	exit 1
}
test -w ua-config || {
	echo "reinstall.sh: wrong directory or wrong user; must be able to write config files" >&2
	exit 1
}
bbsdir="`grep '^bbs-directory[ 	]' ua-config | sed 's/^bbs-directory[ 	][ 	]*\"*\([^\"][^\"]*\).*$/\1/`"
case "$bbsdir" in
"")	bbsdir="$1"
	;;
esac
ed - ua-config << \EOF:ua_config
g/^x-rated[ 	]/s//restricted	/
$a
validation-level	1
.
w
q
EOF:ua-config
cd "$bbsdir" || {
	echo "reinstall.sh: the BBS directory specified by ua-config is bad"
	exit 1
}
ed - userfile << \EOF:userfile
g/^\([^:][^:]*:[^:][^:]*:0*5\):[0-9][0-9]*:\([0-9][0-9]*:[^:][^:]*\):.*$/s//\1:40:\2:16/
w
q
EOF:userfile
cat << EOF:message
The system new-user validation level has been set to Guest (1); you may change
it by editing the "validation-level" parameter in the ua-config file.  (See the
UNaXcess Administration Guide for more information.)

All new-user logins have been set to 16 lines of 40 characters.  These may be
changed via the Alter command of UNaXcess; again, see the UNaXcess Adminis-
tration Guide for details.
EOF:message
exit 0
