#! /bin/csh -f
# This shell script is called from /etc/nu to create a new directory for a
# new user, and to make the necessary links and permissions for it.
# 
# it is named "nu1.sh" instead of something like "makeuser.sh" to discourage
# people from trying to run it standalone, without going through nu.

set uid=$1
set gid=$2
set logindir=$3
set linkdir=$4
set clobber=$5
set debug=$6
set noglob; set path=(/etc $path); unset noglob
set verbose
if ($clobber) then
    rm -rf $logindir
    mkdir $logindir
endif
if ($logindir != $linkdir) ln -s $logindir $linkdir
if (($debug == 0) && $clobber) chown $uid $logindir
if (($debug == 0) && $clobber) chgrp $gid $logindir
