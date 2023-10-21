#! /bin/csh -f
#
#-	shdir.make - Directory stack installation script
#-
#-	To try out the function of shdir, use csh  "source" command to
#-	invoke this program since some aliases will need to be defined
#-	in your current environment.
#-
#-	Change the mode to execute and  then execute it  will cause it
#-	to be installed.
#-
#	Author:		Paul Lew, General Systems Group, Salem, NH
#	Last update:	10/14/86  02:26 PM
#
#if ( "${user}" == "lew" ) goto end
#---------------------------------------------------------------#
#	If invoked from csh "source" command, install locally	#
#	else install globally and update .login, .cshrc, and	#
#	.logout file.						#
#---------------------------------------------------------------#
if ( $?0 ) goto install
#---------------------------------------------------------------#
#		   Compile shdir.c first			#
#---------------------------------------------------------------#
if ( ! -e "shdir" ) then
	if ( ! -e shdir.c ) then
		echo "...File shdir.c missing, aborted..."
		goto end
		endif
	echo -n "...Compiling shdir.c "
	cc -s -o shdir shdir.c -ltermcap
	echo "done..."
	endif
#---------------------------------------------------------------#
#	Make sure all the required files are there		#
#---------------------------------------------------------------#
foreach fname (restdir lsdir shdir useshdir)
	if ( ! -e ${fname} ) then
		echo "...${fname} not in directory ${cwd}, aborted..."
		goto end
		endif
	end
#---------------------------------------------------------------#
#	    Define aliases in the current shell			#
#---------------------------------------------------------------#
foreach aname (lsdir po s to)
	@ achar = `alias ${aname} | wc -c`
	if ( ${achar} > 0 ) echo "...alias ${aname} will be redefined..."
	end
unset aname achar
alias	lsdir	source ${cwd}/lsdir
alias	po	'popd +\!* > /dev/null; shdir `dirs`'
alias	s	${cwd}'/shdir -s\!* `dirs` ;if ( ${status} ) pushd +${status} > /dev/null'
alias	to	'pushd \!* > /dev/null ; '"${cwd}"'/shdir `dirs`'
#---------------------------------------------------------------#
#	  Make a set of directory stack to demo			#
#---------------------------------------------------------------#
set dirs = (`dirs`)
if ( ${#dirs} < 3 ) then
	cd /usr/lib/uucp
	foreach x (/usr/spool/uucp /usr/spool/uucppublic /etc ${cwd})
		pushd $x > /dev/null
		end
	unset x
	endif
#---------------------------------------------------------------#
#	Modify TERMCAP so he/she will see the result		#
#---------------------------------------------------------------#
switch ( "${TERM}" )
	case vt100:
	case vt102:
	case vt125:
	case vt220:
	case vt240:
	case wy75:
	    set noglob
	    eval `tset -Q -s`
	    unset noglob
	    set tc = ('jjkkllmmqqxx' '\E(B' '\E(0')
	    foreach te (ac ae as)
		echo "${TERMCAP}" | fgrep -s ":${te}"
		if ( ${status} == 1 ) then
			echo "...${te} added to TERMCAP..."
			setenv TERMCAP "${TERMCAP}${te}=${tc[1]}:"
			endif
		shift tc
		end
	    breaksw
	default:
	endsw
#---------------------------------------------------------------#
#		     Give a little hint				#
#---------------------------------------------------------------#
cat << cat_eof
...Now type 's' and move to the directory you want by pressing space bar....
cat_eof
goto end

#---------------------------------------------------------------#
#		Final installation starts here			#
#---------------------------------------------------------------#
install:
cat << cat_eof
       **************** GENERAL INFORMATION ****************

This is the final installation procedure.  It will move all the
executables to proper directory and also modify your .login, and
.logout file so that directory stack will preserve across logins.
Your .cshrc will also be modified to define aliases.

The defualt place to store the executables is: /usr/local/bin so that
it can be shared among users.  Press return on to the question below
if you like to use the default.

cat_eof
echo -n "Where do you want to place executables in your system? "
set dir = "$<"
if ( "${dir}" == "" ) set dir = '/usr/local/bin'
if ( ! -d "${dir}" ) then
	mkdir "${dir}"
	if ( ! -d "${dir}" ) then
		echo "...${dir} is not a valid directory, aborted..."
		goto end
		endif
	endif
#---------------------------------------------------------------#
#	 Check if dir specified is in the search path		#
#---------------------------------------------------------------#
unset inpath
foreach cpath (${path})
	if ( "${cpath}" == "${dir}" ) then
		set inpath
		break
		endif
	end
if ( ! ${?inpath} ) then
	echo "...Warning: ${dir} not defined in your search path..."
	endif
unset inpath cpath
#---------------------------------------------------------------#
#	  Move script to the executables directory		#
#---------------------------------------------------------------#
foreach fname (restdir lsdir shdir useshdir)
	if ( -e "${dir}/${fname}" ) then
		echo "...${dir}/${fname} already exist..."
		continue
		endif
	if ( ! -e ${fname} ) then
		echo "...${fname} not in directory ${cwd}, aborted..."
		goto end
		endif
	if ( ${cwd} == "${dir}" ) continue
	if ( -e "${dir}/${fname}" ) then
		echo "...${dir}/${fname} already exist..."
	else
		/bin/mv ${fname} "${dir}"
		endif
	end
sed -e "s:source /usr/local/bin:source ${dir}:" < ${dir}/lsdir > /tmp/lsdir$$
/bin/mv -f /tmp/lsdir$$ ${dir}/lsdir
/bin/chmod a+rx "${dir}/useshdir"
/bin/chmod a+r-x "${dir}"/{restdir,lsdir}
#---------------------------------------------------------------#
#      Modify .cshrc, .logout to install for my account		#
#---------------------------------------------------------------#
${dir}/useshdir ${dir}
cat << cat_eof
--------------------------------------------------------------------
If anyone else in your system would like to use shdir, he/she only
need to execute script: useshdir.

If you like to have VT100 graphic characters to draw the box, add the
following to your termcap entry after you verify it does support these
features:

	:ac=jjkkllmmqqxx:	#alternate graphic characters
	:as=\E(0:		#alternate set start
	:ae=\E(B:		#alternate set end

   ******** The installation is now complete, have fun!! ********
cat_eof
#---------------------------------------------------------------#
#			  Exit here				#
#---------------------------------------------------------------#
unset added msg
end:
