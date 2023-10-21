#! /bin/csh -f
# This shell script is called from nu to initialize the contents of a
# newly-created user's directory.
#
# it is named "nu2.sh" instead of something like "addfiles.sh" to discourage
# people from trying to run it standalone, without going through nu.
#
# Created:  25 Aug 84	Brian Reid

set logindir=$1
set uid=$2
set gid=$3
set wantMHsetup=$4
set debug=$5
set noglob; set path=(/etc $path); unset noglob
set verbose
cd $logindir
cp /usr/skel/.[a-z]* .
if ($wantMHsetup) then
    mkdir Mail
    mkdir Mail/inbox
    cp /usr/skel/Mail/inbox/* Mail/inbox
    if ($debug == 0) chown $uid Mail Mail/* Mail/inbox/*
    if ($debug == 0) chgrp $gid Mail Mail/* Mail/inbox/*
    chmod 0711 Mail
endif
if ($debug == 0) chown $uid .[a-z]*
if ($debug == 0) chgrp $gid .[a-z]*
