: Use /bin/sh
#
# Configure.sh, a shell script for configuring the Elm mail system for
# 	your site and desires.  This script uses some ideas ripped out
#	of the 'rn' install script.  Thanks Larry!
#

export PATH || (sh $0 ; kill $$)

SHELL=/bin/sh

SED1=/tmp/Elm.sed1
SED2=/tmp/Elm.sed2

if [ -f /bin/rm ]
then
  rm=/bin/rm
else
  rm=rm
fi

$rm -f $SED1 $SED2
touch $SED1 $SED2
chmod 777 $SED1 $SED2

# first test - is stdin still free for answering questions??
if [ ! -t 0 ]
then
  echo "Please use 'sh Configure.sh' rather than 'sh < Configure.sh'"
  exit 1
fi

# next let's see what sorta echo flag we have here...

if [ "`echo -n`" = "-n" ]
then
  nflag=""
  cflag="\\c" 
else
  nflag="-n"
  cflag=""
fi

# now the intro blurb

cat << END_OF_INTRO

       		    Elm Configuration Script, v4
       
      
    This is the configuration script for the Elm mail system.  By using
    it rather than editing the "hdrs/sysdefs.h" file, it is hoped that
    the installation process will be considerably easier.

    On all questions, the value in [square brackets] is the default that 
    will be used if you just press RETURN...


END_OF_INTRO

echo "Trying to figure out what sort of OS you're on..."

# next interesting part - figure out what OS we're on

os_name=""

$rm -f .osname
touch .osname

cat << 'EOF' > .get_osname

uname

if [ $? != 0 ] 
then
  if [ -f /vmunix ]
  then
    echo "bsd" > .osname
  else
    exit 0
  fi
fi

if [ "`uname -s`" != "" ]
then
   uname -s | tr '[A-Z]' '[a-z]' > .osname
fi

exit 0
 
EOF

sh .get_osname > /dev/null 2>&1

if [ -f .osname ] 
then
  os_name="`cat .osname`"
  $rm -f .osname
fi

$rm -f .get_osname

if [ "$os_name" = "" ] 
then
  cat << THE_END

I cannot figure out what sort of operating system you're running here.  Please
type in the NAME of the OS you're running or the name of the machine you're
THE_END

  echo $nflag "running on.  I'll check the name in a minute : " $cflag

  read junk morejunk
  
  os_name=`echo $junk | tr '[A-Z]' '[a-z]'`
fi

OS_FLAG="HUH??"

while [ "$OS_FLAG" = "HUH??" ]
do

  case $os_name in
    hp)		NAME="HP-UX"; OS_FLAG=""		;;
    hp-ux) 	NAME="HP-UX"; OS_FLAG="";   		;;
    vax)	NAME="BSD"; OS_FLAG="BSD";  	;;
    vaxen)	NAME="BSD"; OS_FLAG="BSD";  	;;
    bsd)	NAME="BSD"; OS_FLAG="BSD";  	;;
    uts)	NAME="UTS"; OS_FLAG="UTS";  	;;
    sun)	NAME="BSD"; OS_FLAG="";		;;
    pyramid)	NAME="Pyramid BSD"; OS_FLAG="PYRAMID";	;;
    amdahl)	NAME="UTS"; OS_FLAG="UTS";  	;;
    sv_*)	NAME="System V"; OS_FLAG="";		;;
    svii_*)	NAME="System VII"; OS_FLAG="";		;;
    v5)		NAME="System V"; OS_FLAG="";		;;
    v7)		NAME="System VII"; OS_FLAG="";		;;
    eunice)	NAME="Eunice"; OS_FLAG="";		;;
    convergent)	NAME="Convergent BSD"; OS_FLAG="BSD";		;;
    berkeley)	NAME="BSD"; OS_FLAG="BSD";		;;
    ultrix)	cat << THE_END

I know that this is a breed of Unix, but I don't know what TYPE of OS it
is like.  Please enter the NAME of the OS that this is most like (e.g. "BSD")
and we'll go from there.

THE_END
					;;
    xenix)	cat << THE_END

You're outta luck.  Xenix (dumb OS that it is only allows 6 character 
identifier names.  You'll need to go in and LABORIOUSLY translate all the
VERY LONG identifier names to the right length.   The correct response to 
seeing this message is to call your vendor and get ANGRY!!!!

Meanwhile, we might as well just quit here.  Sorry.
THE_END
exit 1
					;;
    *)		cat << THE_END

I don't know what OS you're specifying!  The only one's I know of are;

  HP-UX, BSD, UTS, Eunice, Xenix, Ultrix, V5, and V7

I also know the machines

  HP, Amdahl, Sun, Vaxen, Convergent and Pyramid

If you're not among this list, you'll need to pick the closest OS name.  
THE_END

    echo " "
    echo $nflag "Please choose again: " $cflag
    read os_name
    ;;
  esac

done

echo " "
echo " "
echo you\'re on the following version of Unix: $NAME

echo " "
echo "Now we can get down to those questions..."
echo " "

cat << THE_END

First off, should the program use "uuname" rather than trying to read 
THE_END

not_using_lsys_file=0

answer="yes"
echo $nflag "the L.sys file (y/n) [yes] ? "  $cflag
read answer

if [ "$answer" = "n" -o "$answer" = "no" ]
then
  not_using_lsys_file=1
  echo "s/#define USE_UUNAME/\/** #define USE_UUNAME **\//" >> $SED1
fi

cat << THE_END

Next, are you running smail, a "gateway" configuration of sendmail, or some 
other program that means the program shouldn't touch any of the addresses 
THE_END

answer="no"
dont_touch_addresses=0
echo $nflag "that users type in (y/n) [no] ? " $cflag
read answer

if [ "$answer" = "y" -o "$answer" = "yes" ]
then
  dont_touch_addresses=1
  echo \
"s/\/\*\* #define DONT_TOUCH_ADDRESSES \*\*\//#define DONT_TOUCH_ADDRESSES /" \
  >> $SED1
  echo \
"s/\/\*\* #define DONT_OPTIMIZE_RETURN \*\*\//#define DONT_OPTIMIZE_RETURN /" \
  >> $SED1
fi

cat << THE_END

Does your site receive mail with valid "Reply-To:" and "From:" fields in
THE_END

answer="no"
echo $nflag "the headers (y/n) [no] ? " $cflag
read answer

if [ "$answer" != "y" -a "$answer" != "yes" ]
then
  echo \
"s/#define USE_EMBEDDED_ADDRESSES/\/** #define USE_EMBEDDED_ADDRESSES **\//" \
  >> $SED1
fi

cat << THE_END

-------------------------------------------------------------------------------

How about memory?  If you have a lot, you can enter a fairly large number
for the next few questions...if not, you'll probably want to enter the
suggested small-system values.  (This applies to the speed of the swapping
on your system too - if you're on a FAST system, use the big values!)

First, how many aliases should be allowed for an individual user? The suggested
values are 503 for blindly fast systems, 251 for average systems and 127 for
THE_END

max_ualiases="NONE"

while [ "$max_ualiases" = "NONE" ] 
do 

  echo $nflag "slow systems.  Number of aliases [251] ? " $cflag 

  read junk

  if [ "$junk" = "" ] 
  then
    junk=251
  fi

  if [ $junk -lt 50 -o $junk -gt 1000 ] 
  then
    echo \
"Pretty strange answer!  I think you should reconsider and try this question "\
    echo "again..."
    echo " "
  else
    max_ualiases=$junk
    echo "s/>251</$max_ualiases/" >> $SED1
    case $junk in
	127) default=223	;;
	503) default=739	;;
	*  ) default=503	;;
    esac
  fi
done

max_saliases="NONE"

while [ "$max_saliases" = "NONE" ] 
do 

  echo $nflag "Max number of system aliases available  [$default] ? " $cflag

  read junk

  if [ "$junk" = "" ] 
  then
   junk=$default
  fi

  if [ $junk -lt 50 -o $junk -gt 1000 ] 
  then
    echo "Pretty far out value for this question!  I think you should reconsider"
    echo "your answer and try this question again..."
    echo " "
  else
    max_saliases=$junk
    echo "s/>503</$max_saliases/" >> $SED1
  fi
done

cat << THE_END

The next pair of questions have to do with what to do when another program has 
locked a mailbox...

First, how many times should the program check for the removal of the lock
THE_END

default=6

max_attempts="NONE"

while [ "$max_attempts" = "NONE" ] 
do 

  echo $nflag "file before giving up? [6] " $cflag

  read junk

  if [ "$junk" = "" ] 
  then
   junk=$default
  fi

  if [ $junk -lt 3 -o $junk -gt 10 ] 
  then
    echo \
"The recommended range is 3-10   ...Number of times to check lock"
  else
    max_attempts=$junk
    echo "s/>6</$max_attempts/" >> $SED1
  fi
done

echo " "
answer="no"
echo $nflag "Should it REMOVE the lockfile after $max_attempts checks [no] ?" \
     $cflag
read answer

if [ "$answer" != "y" -a "$answer" != "yes" ]
then
  echo \
"s/#define REMOVE_AT_LAST/\/** #define REMOVE_AT_LAST **\//" \
  >> $SED1
fi

if [ "$NAME" = "BSD" ]
then 
  ps="ps -cax"
else
  ps="ps -ef"
fi

echo " "
echo " "
echo "poking about a bit.."

result="`$ps | grep sendmail | grep -v grep`"

if [ "$result" = "" ]
then
  if [ -f /usr/lib/sendmail -a -f /usr/lib/sendmail.cf ]
  then
    echo \(sendmail available - assuming you don\'t run it as a daemon\)
    result="ok"
  fi
fi

if [ "$result" != "" ]
then
  echo "You're running sendmail.  Well done, I guess..."
  echo "s/\/\*\* #define ALLOW_BCC \*\*\//#define ALLOW_BCC/" \
    >> $SED1
  echo "s/\/\*\* #define DONT_ADD_FROM \*\*\//#define DONT_ADD_FROM/" \
    >> $SED1
    echo \
 "s/#define USE_DOMAIN/\/** #define USE_DOMAIN **\//" \
    >> $SED1
else

  cat << THE_END

Since you're not running sendmail, should I check local user entered addresses
THE_END

answer="yes"
echo $nflag "against the valid mailboxes on this system [yes] ? " $cflag
read answer

if [ "$answer" != "y" -a "$answer" != "yes" -a "$answer" != "" ]
then
  echo \
"s/#define NOCHECK_VALIDNAME/\/** #define NOCHECK_VALIDNAME **\//" \
  >> $SED1
fi

cat << THE_END

Are you running a machine where you want to have a domain name appended to the
THE_END

answer="yes"
echo $nflag "hostname on outbound mail [no] ? " $cflag
read answer

if [ "$answer" != "y" -a "$answer" != "yes" ]
then
    echo \
 "s/#define USE_DOMAIN/\/** #define USE_DOMAIN **\//" \
    >> $SED1
else
  echo " "
  echo $nflag "Enter the domain name (include leading '.') : " $cflag
  read answer
  echo "s/<enter your domain here>/$answer/" >> $SED1
fi

fi

# next let's see if we can find the vfork command on this system..

cat << EOF > .test.c
main()
{
	(void) vfork();
}
EOF

if [ "$NAME" = "UTS" ]
then
cat << EOF > .vfork
cc -la .test.c
EOF
else
cat << EOF > .vfork
cc .test.c
EOF
fi

sh .vfork > .log 2>& 1

if [ "`cat .log | wc -l`" -eq "0" ]
then
  echo "You have virtual memory system calls available.  Cool..."
else
  cat << THE_END

Your machine doesn't seem to have the vfork command available.  Should I assume
THE_END

  answer="no"
  echo $nflag "you have it, anyway [no] ? " $cflag
  read answer

if [ "$answer" != "y" -a "$answer" != "yes" ]
  then
    echo "s/\/\*\* #define NO_VM \*\*\//#define NO_VM/" >> $SED1
  fi
fi

$rm -f a.out .test.c .vfork .log

# next let's see if we have the gethostname() system call...

cat << EOF > .test.c
main()
{
	(void) gethostname();
}
EOF

cat << EOF > .hostname
cc .test.c
EOF

sh .hostname > .log 2>& 1

if [ "`cat .log | wc -l`" -eq "0" ]
then
  echo "You have the 'gethostname()' system call..."
else
  echo "s/\/\*\* #define NEED_GETHOSTNAME \*\*\//#define NEED_GETHOSTNAME/" \
  >> $SED1
fi

$rm -f a.out .test.c .hostname .log

# next let's see if we have long variable names...

cat << EOF > .test.c
main()
{
	int this_is_a_long_variable=0;

	(void) this_is_a_long_variable_routine_name(this_is_a_long_variable);
	
}
this_is_a_long_variable_routine_name() { }
EOF

cat << EOF > .varnames
cc .test.c
EOF

sh .varnames > .log 2>& 1

if [ "`cat .log | wc -l`" -eq "0" ]
then
  echo "You have long variable names.  Well done!!!!!"
else
  echo "How embarassing.  Your C compiler doesn't support long variables..."
  echo "s/\/\*\* #define SHORTNAMES \*\*\//#define SHORTNAMES/" \
  >> $SED1
fi

$rm -f a.out .test.c .varnames .log

if [ $dont_touch_addresses = 0 ]
then
cat << THE_END

When given a machine that you talk to directly, should the 'pathalias' route to
THE_END

 answer="no"
 echo $nflag "the machine be used instead [no] ? " $cflag
 read answer

 if [ "$answer" != "y" -a "$answer" != "yes" ]
 then
    echo \
 "s/#define LOOK_CLOSE_AFTER_SEARCH/\/** #define LOOK_CLOSE_AFTER_SEARCH **\//"\
    >> $SED1
 fi
fi

answer="yes"
echo " "
echo $nflag "Is the preferred address notation 'user@host' [yes] ?" $cflag
read answer

if [ "$answer" != "y" -a "$answer" != "yes" -a  "$answer" != "" ]
then
    echo \
 "s/#define INTERNET_ADDRESS_FORMAT/\/** #define INTERNET_ADDRESS_FORMAT **\//" \
    >> $SED1
fi

echo " "
answer="yes"
echo $nflag "Am I going to be running as a setgid program [yes] ? "$cflag
read answer

if [ "$answer" != "y" -a "$answer" != "yes" -a "$answer" != "" ]
then
  echo answer is currently equal to \"$answer\"
  echo \
 "s/#define SAVE_GROUP_MAILBOX_ID/\/** #define SAVE_GROUP_MAILBOX_ID **\//" \
    >> $SED1
fi
    
cat << THE_END


For any of the questions after this point, you can press RETURN if the 
questions doesn't apply, or there's no reasonable answer...

THE_END

if [ $dont_touch_addresses = 0 ]
then 
 if [ ! -f /usr/lib/nmail.paths ] 
 then
   echo $nflag "Where does the output of pathalias live ? " $cflag
   read answer

   if [ "$answer" != "" ]
   then
     echo "s^/usr/lib/nmail.paths^$answer^" >> $SED1
   fi
 fi
fi

use_dbm=0

if [ $dont_touch_addresses = 0 ]
then
  if [ -f $answer.pag -a -f $answer.dir ]
  then
    echo "You have pathalias as a DBM file...I'll use that instead."
    echo "s^/\*\* #define USE_DBM \*\*/^#define USE_DBM^" >> $SED1
    use_dbm=1
  fi
fi

case $OS_FLAG in 
  BSD) echo "s/>os-define</-DBSD/"    >> $SED2
       echo "s/>lib2</-lcurses/"      >> $SED2
       if [ $use_dbm = 1 ]
       then
         echo "s/>libs</-ltermcap -ldbm/" >> $SED2
       else
         echo "s/>libs</-ltermcap/"       >> $SED2
       fi
       ;;

  PYRAMID) echo "s/>os-define</"-DBSD -DNO_VAR_ARGS"/" >> $SED2
       echo "s/>lib2</-lcurses/"      >> $SED2
       if [ $use_dbm = 1 ]
       then
         echo "s/>libs</-ltermcap -ldbm/"     >> $SED2
       else
         echo "s/>libs</-ltermcap/"     >> $SED2
       fi
       ;;

  UTS) echo "s/>os-define</-DUTS/"    >> $SED2
       echo "s/>lib2</-la -lq/"       >> $SED2
       if [ $use_dbm = 1 ]
       then
         echo "s/>libs</-lcurses -ldbm/" >> $SED2
       else
         echo "s/>libs</-lcurses/"       >> $SED2
       fi
       ;;

  *)   echo "s/>os-define<//"         >> $SED2
       echo "s/>lib2<//"              >> $SED2
       if [ $use_dbm = 1 ] 
       then
         echo "s/>libs</-ltermcap -ldbm/" >> $SED2
       else
         echo "s/>libs</-ltermcap/"       >> $SED2
       fi
       ;;

esac

 
if [ $dont_touch_addresses = 0 ]
then
 if [ ! -f /usr/lib/domains ] 
 then
   echo $nflag "Where does the 'domains' file live ? " $cflag
   read answer

   if [ "$answer" != "" ]
   then
     echo "s^/usr/lib/domains^$answer^" >> $SED1
   fi
 fi
fi

if [ $not_using_lsys_file = 1 ]
then
  if [ ! -f /usr/lib/uucp/L.sys ]
  then
    echo $nflag "Where does the 'L.sys' file live ? " $cflag
    read answer

    if [ "$answer" != "" ]
    then
      echo "s^/usr/lib/uucp/L.sys^$answer^" >> $SED1
    fi
  fi
fi
 
if [ ! -d /tmp ]
then 
  echo $nflag "/tmp isn't a directory!  What should I use?? " $cflag
  read answer

  if [ "$answer" != "" ]
  then
    echo "s^/tmp^$answer^" >> $SED1
  fi
fi

if [ ! -f /usr/ucb/vi -a "$os_name" = "BSD" ]
then
  echo $nflag "I can't find the 'vi' editor!  Where is it? " $cflag
  read answer

  if [ "$answer" != "" ]
  then
    echo "s^/usr/ucb/vi^$answer^" >> $SED1
  fi
elif [ ! -f /usr/bin/vi -a "$os_name" = "" ]
then
  echo $nflag \
    "I can't find the 'vi' editor!  Where is it? " $cflag
  read answer

  if [ "$answer" != "" ]
  then
    echo "s^/usr/bin/vi^$answer^" >> $SED1
  fi
fi

if [ ! -d /usr/spool/mail -a "$os_name" = "BSD" ]
then
  echo $nflag "I can't find your inbound mail directory!  Where is it? " $cflag
  read answer

  if [ "$answer" != "" ]
  then
    echo "s^/usr/spool/mail^$answer^" >> $SED1
  fi
elif [ ! -d /usr/mail -a "$os_name" = "" ]
then
  echo $nflag "I can't find your inbound mail directory!  Where is it? " $cflag
  read answer

  if [ "$answer" != "" ]
  then
    echo "s^/usr/mail^$answer^" >> $SED1
  fi
fi

if [ ! -f /bin/rm ]
then
  echo $nflag "Where's the 'rm' program? " $cflag
  read answer

  if [ "$answer" != "" ]
  then
    echo "s^/bin/rm^$answer^" >> $SED1
  fi
fi

if [ ! -f /bin/cat ]
then
  echo $nflag "Where's the 'cat' program? " $cflag
  read answer

  if [ "$answer" != "" ]
  then
    echo "s^/bin/rm^$answer^" >> $SED1
  fi
fi
 
if [ ! -c /dev/rct ]
then
  if [ ! -c /dev/rmt ]
  then
    echo $nflag "What's the name of your remote-tape unit? " $cflag
    read answer

    if [ "$answer" = "" ]
    then
      echo "s^>tapeunit<^unknown-remote-tape-unit^" >> $SED2
    else
      if [ ! -c $answer ] 
      then
	if [ -c /dev/$answer ]
	then
          echo "s^>tapeunit<^/dev/$answer^" >> $SED2
        else
          echo \
           "I can't find either $answer or /dev/$answer.  I'll set it to junk" 
          echo "s^>tapeunit<^unknown-remote-tape-unit^" >> $SED2
        fi
      else
        echo "s^>tapeunit<^$answer^" >> $SED2
      fi
    fi
  else
    echo "s^>tapeunit<^/dev/rmt^" >> $SED2
  fi
else
  echo "s^>tapeunit<^/dev/rct^" >> $SED2
fi

if [ ! -d /usr/local/bin ]
then
  echo $nflag "Where do you want the system software installed? " $cflag
  read answer

  if [ "$answer" = "" ]
  then
    echo "s^>dest-dir<^unknown-destination-directory^" >> $SED2
  else
    if [ -d $answer ]
    then
      echo "s^>dest-dir<^$answer^" >> $SED2
    else
      echo "I don't know what you're talking about.  I'll set it to junk" 
      echo "s^>dest-dir<^unknown-destination-directory^" >> $SED2
    fi
  fi
else
  echo "s^>dest-dir<^/usr/local/bin^" >> $SED2
fi

if [ ! -f /usr/bin/troff ]
then
  if [ ! -f /usr/local/bin/troff ]
  then
    if [ ! -f /usr/contrib/bin/troff ]
    then
      if [ ! -f /usr/ucb/troff ]
      then
        if [ ! -f /bin/troff ]
        then
          echo $nflag "Where does the 'troff' program live? " $cflag
 	  read answer

	  if [ "$answer" = "" ]
	  then
            echo "s/>troff</nroff/" >> $SED2
	  else
            if [ -f $answer ]
	    then
              echo "s^>troff<^$answer^" >> $SED2
              troff=$answer
	    else
	      echo "Still can't find it.  I'll set it to \"nroff\" instead."
              echo "s/>troff</nroff/" >> $SED2
	    fi
	  fi
	else
          echo "s^>troff<^/bin/troff^" >> $SED2
          troff=/bin/troff
        fi
      else
        echo "s^>troff<^/usr/ucb/troff^" >> $SED2
        troff=/usr/ucb/troff
      fi
    else
      echo "s^>troff<^/usr/contrib/bin/troff^" >> $SED2
      troff=/usr/contribbin/troff
    fi
  else
    echo "s^>troff<^/usr/local/bin/troff^" >> $SED2
    troff=/usr/local/bin/troff
  fi
else
  echo "s^>troff<^/usr/bin/troff^" >> $SED2
  troff=/usr/bin/troff
fi

# phew!

troffdir=`dirname $troff`

if [ -f $troffdir/tbl ]
then
  echo "s^>tbl<^$troffdir/tbl^" >> $SED2
else
  echo $nflag "Where does the 'tbl' program live? " $cflag
  read answer

  if [ "$answer" = "" ]
  then
    echo "s^>tbl<^cat^" >> $SED2
  elif [ -f $answer ]
  then
    echo "s^>tbl<^$answer^" >> $SED2
  else
    echo "I can't find that either.  I'll just set it to junk..."
    echo "s^>tbl<^cat^" >> $SED2
  fi
fi

if [ -f /bin/cc ]
then
  echo "s^>cc<^/bin/cc^" >> $SED2
else
  echo $nflag "Where does the 'C' compiler live? " $cflag
  read answer

  if [ "$answer" = "" ]
  then
    cat << THE_END

I hope you realize that without a "C" compiler there's no point in doing any
of this.  If we can't compile anything then this is just so much disk filler.

In fact, thinking about it, let's just quit right now.

THE_END

    exit 1
  fi

  if [ -f $answer ]
  then
    echo "s^>cc<^$answer^" >> $SED2
  else
    cat << THE_END

I couldn't find what you specified, pal.  I hope you realize that without a 
"C" compiler there's no point in doing any of this.  If we can't compile 
anything then this system is just so much disk filler.

In fact, thinking about it, let's just quit right now.

THE_END
    exit 1
  fi
fi

echo "s^>rm<^$rm -f^" >> $SED2

if [ -f /bin/mv ]
then
  echo "s^>mv<^/bin/mv -f^" >> $SED2
else
  echo "s^>mv<^mv -f^" >> $SED2  
fi

if [ -f /bin/cp ]
then
  echo "s^>cp<^/bin/cp^" >> $SED2
else
  echo "s^>cp<^cp^" >> $SED2  
fi

cat << END

That's it.  Just  have to do some patching up and such...hang loose for a 
minute or two, please...

END

# process the four Makefiles accordingly...

echo "1 - processing the file \"Makefile\"..."
cat Makefile.mstr | sed -f $SED2 > Makefile

echo "2 - processing the file \"src/Makefile\"..."
cat src/Makefile.mstr | sed -f $SED2 > src/Makefile

echo "3 - processing the file \"utils/Makefile\"..."
cat utils/Makefile.mstr | sed -f $SED2 > utils/Makefile

echo "4 - processing the file \"filter/Makefile\"..."
cat filter/Makefile.mstr | sed -f $SED2 > filter/Makefile

# then filter the sysdefs file through the sed script we've created!

echo "Finally, processing the file \"hdrs/sysdefs.h\"..."

if [ -f hdrs/sysdefs.h ]
then
  mv hdrs/sysdefs.h hdrs/sysdefs.old
fi

cat hdrs/sysdefs.master | sed -f $SED1 > hdrs/sysdefs.h

echo Done\!

$rm -f $SED1 $SED2
exit 0
