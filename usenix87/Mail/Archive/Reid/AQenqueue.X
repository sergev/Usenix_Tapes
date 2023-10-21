#! /bin/sh 
#
# This program error-checks and enqueues an archive retrieval request.
#
# usage: AQenqueue < chunkfile

PATH=#PATH#
export PATH
TmpFile=#TMP#/AQenqueue.$$
HomeDir=#HOME#
WorkQueue=#REQQ#
trap "rm -f $TmpFile; exit" 0 1 2 3 15

awk 'BEGIN {Dest="x"}
$1 == "PATH" {Path=$2; for(i=3;i<=NF;i++) Path = Path " " $i; next}
$1 == "ARGS" {Args=$2; for(i=3;i<=NF;i++) Args = Args " " $i; next}
$1 == "FROM" {From=$2; for(i=3;i<=NF;i++) From = From " " $i; next}
$1 == "VERB" {Verb=$2; next}
$1 == "DATE" {Date=$2; for(i=3;i<=NF;i++) Date = Date " " $i; next}
$1 == "DEST" {Dest=$2; next}
END {
   printf "Path='\''%s'\''\n",Path;
   printf "Dest=\"%s\"\n",Dest;
   printf "From='\''%s'\''\n",From;
   printf "Verb=\"%s\"\n",Verb;
   printf "Date=\"%s\"\n",Date;
   UUCP=0; if (index(Path,"!") > 0) UUCP=1;
   printf "ViaUUCP=%s\n",UUCP;
   print  "set ",Args
} ' > $TmpFile
cat >> $TmpFile << 'endall'

# Step 1: check for errors and send an acknowledgment

Files=""; Cumsize=1250; Unknown=""; Nfiles=0; NSharfiles=0; TooBig=0;
cd $HomeDir
Dir="Recipes"
cd $HomeDir
for j in $*
do
    case $j in
	recipe) Dir="Recipes";;
	recipes) Dir="Recipes";;
	receipt) Dir="Recipes";;
	recipie) Dir="Recipes";;
	program) Dir="Programs";;
	programs) Dir="Programs";;
	system) Dir="Systems";;
	systems) Dir="Systems";;
	software) Dir="Programs";;
	prog) Dir="Programs";;
	progs) Dir="Programs";;
	cleartext) Dir="Cleartext";;
	clear) Dir="Cleartext";;
	map) Dir="Maps";;
	maps) Dir="Maps";;
	netmap) Dir="Maps";;
	flowmap) Dir="FlowMaps";;
	flomap) Dir="FlowMaps";;
	flowmaps) Dir="FlowMaps";;
	flow-maps) Dir="FlowMaps";;
	*) case $j in
	       index) j="Index";;
	   esac;
	   cd $HomeDir/$Dir
# this permits the user to ask for the file in any case, and we fold
# to match the truth:
	   file=`ls -1 | grep -i \^${j}\$`
	   if [ $? -eq 0  ]
	   then
		file=`ls -1 | grep -i \^${j}\$ | sed 1q`
		Fsize=`wc -c $file | colrm 9 | tr -d ' '`
		Fsize=`expr $Fsize + 125`;
		TrialCumSize=`expr $Cumsize + $Fsize`
		if [ $ViaUUCP -gt 0 -a $TrialCumSize -gt #UUCPLIMIT#000 ]
		then
		    TooBig=1;
		    Unknown="$Unknown $Dir/$file"
		else
		    case $j in
			*.shar) NSharfiles=`expr 1 + $NSharfiles`
				if [ $NSharfiles -le 1 ]
				then
				    Cumsize=$TrialCumSize
				    Files="$Files $Dir/$file"
				    Nfiles=`expr 1 + $Nfiles`;
				else
				    Unknown="$Unknown $Dir/$file"
				fi;;
			*)  Cumsize=$TrialCumSize
			    Files="$Files $Dir/$file"
			    Nfiles=`expr 1 + $Nfiles`;;
		    esac
		fi
	   else
	        Unknown="$Unknown $Dir/$j"
	   fi;;
    esac
    shift
done
cd $HomeDir
if [ $NSharfiles -gt 1 ]
then
    Z="multishar $Cumsize";
else
    case $Unknown in
	 "") case $Files in
	    "") Z="null 0";;
	    *) Z="allgood $Cumsize";;
	 esac;;
	 *) case $Files in
	    "") Z="allbad 0";;
	    *) case $TooBig in
	       0) Z="somegood $Cumsize";;
	       *) Z="toobiguucp $Cumsize";;
	       esac;;
	 esac;;
    esac
fi

if AQack "$Path" "$From" "$Date" $Z "$Unknown" "$Files"
then
     Temp1="000000000$Cumsize"
     Unique=`date+ 0 hours "%S"`
     Filename=`expr $Temp1 : '.*\(.......\)$'`.$Unique
     cd $WorkQueue
     cat > $Filename << something
cat << nothing > #TMP#/ZQ.\$\$
From: decwrl archive service <archive-server>
Subject: per your request
In-reply-to: Request from $From dated $Date
To: $Path
\`cat /udir/recipes/copyright\`
$copr
nothing
something
    if [ $Nfiles -eq 1 ]
    then
	echo 'cat #TMP#/ZQ.$$ '$Files' | #MAILER# '\'$Path\' >> $Filename
    else
	echo 'rshar '$Files' | cat #TMP#/ZQ.$$ - | #MAILER# '\'$Path\' >> $Filename
    fi
    cat << logentry >> $Filename
rm -f #TMP#/ZQ.\$\$
echo "\`date\`" sent $Cumsize to '$Path' >> #REQQ#/out.log
logentry
fi

endall
    . $TmpFile
