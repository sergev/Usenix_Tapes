#
#	An implementation of pushd and popd for ksh.
#	Written by Fletcher Mattox, sally!fletcher
#
#	The directory stack grows upward from 0.  This is different from
#	the csh version where the top of the stack is always 0.
#	However, the direction of our stack is secretly reversed
#	when necessary to preserve the illusion of a stack with top
#	at 0.  The idea is to make this program appear identical
#	to its counterpart in csh.
#
#	invariant: dirstack[top] == $PWD
#
#	From the csh man page...
#
#     dirs
#          Prints the directory stack; the top of the stack is at
#          the left, the first directory in the stack being the
#          current directory.
#
#     popd
#     popd +n
#          Pops the directory stack, returning to the new top
#          directory.  With a argument `+n' discards the nth entry
#          in the stack.  The elements of the directory stack are
#          numbered from 0 starting at the top.
#
#     pushd
#     pushd name
#     pushd +n
#          With no arguments, pushd exchanges the top two elements
#          of the directory stack.  Given a name argument, pushd
#          changes to the new directory (ala cd) and pushes the
#          old current working directory (as in csw) onto the
#          directory stack.  With a numeric argument, rotates the
#          nth argument of the directory stack around to be the
#          top element and changes to it.  The members of the
#          directory stack are numbered from the top starting at
#          0.
#
let top=0
dirstack[top]=$PWD

function pushd
{
	dirstack[top]=$PWD	# kludge to force . to top of stack
	case $# in
	0)
		if (( top > 0 )) ; then
			let n=top-1
			swapdir $n $top
		else
			echo No other directory.
		fi
		;;
	1)
		case $1 in
		+[1-9]*)
			n=$1
			n=${n#+}
			if (( n > top )) ; then
				echo Directory stack not that deep.
				break
			else
				let n=top-n	# "reverse" the stack
				swapdir $n $top
			fi
			unset n
			;;
		*)
			if  cd $1 ; then
				dirstack[ ((top=top+1)) ]=$PWD
				dirs
			fi
			;;
		esac
		;;
	*)
		echo Too many arguments.
		;;
	esac
}

function popd
{
	dirstack[top]=$PWD	# kludge to force . to top of stack
	case $# in
	0)
		case $top in
		0)
			echo Directory stack empty.
			;;
		*)
			unset dirstack[top]
			let top=top-1
			cd ${dirstack[top]}
			dirs
			;;
		esac
		;;
	1)
		case $1 in
		+[1-9]*)
			n=$1
			n=${n#+}
			if (( n > top )) ; then
				echo Directory stack not that deep.
				unset n
				break
			else
				let n=top-n	# "reverse" the stack, and
				# shift the stack down one for all entries >= n
				while (( n < top ))
				do
					let m=n+1
					dirstack[n]=${dirstack[m]}
					let n=n+1
				done
				unset dirstack[top] m n
				let top=top-1
				dirs
			fi
			;;
		*)
			echo Bad directory.
			;;
		esac
 		;;
	*)
		echo Too many arguments.
		;;
	esac
}

function dirs
{
	dirstack[top]=$PWD	# kludge to force . to top of stack
	let n=top
	while (( n >= 0 ))
	do
		echo "${dirstack[n]} "\\c
		let n=n-1
	done
	echo ""
	unset n
}

function swapdir
{
	t=${dirstack[$1]}
	dirstack[$1]=${dirstack[$2]}
	dirstack[$2]=$t
	cd ${dirstack[top]}
	dirs
	unset t
}
