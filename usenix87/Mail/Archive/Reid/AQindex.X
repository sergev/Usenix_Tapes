#! /bin/sh
#
# This program is called from AQgetentry to send an index
#
# usage: AQindex "destination" "username" arg1 ... argn

PATH=#PATH#
export PATH
Tmpfile=#TMP#/AQindex.$$
HomeDir=#HOME#

trap "rm -f $Tmpfile; exit" 0 1 2 3 15

Path="$1"; shift
User="$1"; shift

echo "`date`" index $Path $* >> #REQQ#/out.log
cat << xxx > $Tmpfile
Subject: index, per your request
From: #SVC-TITLE# <#SVC-MAILBOX#>
In-reply-to: message from $User
To: $Path

xxx
FirstFlag=0
cd $HomeDir
for j in $*
do
    Dir="$j";
    case $j in
	index) continue;;
	recipe) Dir="Recipes";;
        recipes) Dir="Recipes";;
        receipt) Dir="Recipes";;
        recipie) Dir="Recipes";;
        program) Dir="Software";;
        programs) Dir="Programs";;
        software) Dir="Programs";;
        prog) Dir="Programs";;
        progs) Dir="Programs";;
	system) Dir="Systems";;
	systems) Dir="Systems";;
	software) Dir="Programs";;
	map) Dir="Maps";;
	maps) Dir="Maps";;
	flowmap) Dir="FlowMaps";;
	flomap) Dir="FlowMaps";;
	flowmaps) Dir="FlowMaps";;
	flow-maps) Dir="FlowMaps";;
	.) Dir=".";;
    esac
    case $Dir in
	.) DirName="top level";;
	*) DirName=$Dir;;
    esac
    case $FirstFlag in
	1) echo "aaaaaa" | tr "a" "\012" >> $Tmpfile;;
    esac
    FirstFlag=1;
    cat >> $Tmpfile << halfmast
Index of $DirName from #SVC-TITLE# (updated `ls -l $Dir/Index | colrm 1 32`)
-----------------------------------------------------------------
halfmast
	if [ -f $Dir/Index ]
	then
	    cat $Dir/Index >> $Tmpfile
	else
	    echo "Sorry, I don't have an index on file for \"$DirName\"" >> $Tmpfile
	    echo >> $Tmpfile
	fi
done

#MAILER# "$Path" < $Tmpfile
