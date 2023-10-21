
From $HOME/.profile:
==============================================================
export CDHISTFILE CDHIST PWD
CDHIST=$HOME
# CDHISTFILE=$HOME/.cdhistory	# set this for directory history file.
CDHISTFILE=
ENV=$HOME/.kshrc
==============================================================

From $HOME/.kshrc (Korn shell ENV file):
==============================================================
alias set_prompt="PS1='$ '"	# of course, my real prompt is different.
alias cd='. $HOME/.functions; _cd'
alias md='. $HOME/.functions; md'
==============================================================

From $HOME/.functions (Where my function definitions reside):
==============================================================
alias cd=_cd
function _cd
{
	typeset -i cdlen i
	typeset t

	if [ $# -eq 0 ]
	then
		set -- $HOME
	fi

	if [ "$CDHISTFILE" -a -r "$CDHISTFILE" ] # if directory history exists
	then
		typeset CDHIST
		i=-1
		while read -r t			# read directory history file
		do
			CDHIST[i=i+1]=$t
		done <$CDHISTFILE
	fi

	if [ "${CDHIST[0]}" != "$PWD" -a "$PWD" != "" ]
	then
		_cdins				# insert $PWD into cd history
	fi

	cdlen=${#CDHIST[*]}			# number of elements in history

	case "$@" in
	-)					# cd to new dir
		if [ "$OLDPWD" = "" ] && ((cdlen>1))
		then
			print ${CDHIST[1]}
			'cd' ${CDHIST[1]}
		else
			'cd' $@
		fi
		;;
	-l)					# print directory list
		typeset -R3 num
		((i=cdlen))
		while (((i=i-1)>=0))
		do
			num=$i
			print "$num ${CDHIST[i]}"
		done
		return
		;;
	-[0-9]|-[0-9][0-9])			# cd to dir in list
		if (((i=${1#-})<cdlen))
		then
			print ${CDHIST[i]}
			'cd' ${CDHIST[i]}
		else
			'cd' $@
		fi
		;;
	-*)					# cd to matched dir in list
		t=${1#-}
		i=1
		while ((i<cdlen))
		do
			case ${CDHIST[i]} in
			*$t*)
				print ${CDHIST[i]}
				'cd' ${CDHIST[i]}
				break
				;;
			esac
			((i=i+1))
		done
		if ((i>=cdlen))
		then
			'cd' $@
		fi
		;;
	*)					# cd to new dir
		'cd' $@
		;;
	esac

	_cdins					# insert $PWD into cd history

	if [ "$CDHISTFILE" ]
	then
		cdlen=${#CDHIST[*]}		# number of elements in history

		i=0
		while ((i<cdlen))
		do
			print -r ${CDHIST[i]}	# update directory history
			((i=i+1))
		done >$CDHISTFILE
	fi
	set_prompt
}

function _cdins					# insert $PWD into cd history
{						# meant to be called only by _cd
	typeset -i i

	((i=0))
	while ((i<${#CDHIST[*]}))		# see if dir is already in list
	do
		if [ "${CDHIST[$i]}" = "$PWD" ]
		then
			break
		fi
		((i=i+1))
	done

	if ((i>22))				# limit max size of list
	then
		i=22
	fi

	while (((i=i-1)>=0))			# bump old dirs in list
	do
		CDHIST[i+1]=${CDHIST[i]}
	done

	CDHIST[0]=$PWD				# insert new directory in list
}
==============================================================

Cd is a sizable alias, but actually executes quite quickly.
I hope it is useful to someone else; I've been using it for several months.

-Dale

-- 
	seismo!noao!mcdsun!nud!df	602/438-5739	ihnp4!mot!nud!df
