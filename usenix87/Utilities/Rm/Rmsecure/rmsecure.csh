#! /bin/csh -f
# csh script ; secure remove of files
onintr out
set interact
set force
set recur
set minus
while ("$1" == "-" || "$1" =~ -[f,i,r])
    switch ($1)
	case -i: 
	    set interact=-i
	    shift
	    breaksw
	case -f: 
	    set force=-f
	    shift
	    breaksw
	case -r: 
	    set recur=-r
	    shift
	    breaksw
	case  -: 
	    set minus=-
	    shift
	    break
    endsw
end
if ($interact == -i) then
    /bin/rm -i $force $recur $minus $*
    exit($status)
endif
set f1=`echo $0`
set f1=$f1:t" : remove"
set r
while ("$1" != '')
    set b=`echo $1`
    if ("$b" != '') then
	set noglob
	set a=$1
	unset noglob
	if ("$a" == "$b") then
	    set r="$r $b"
	else
	    if ($#b == 1) then
		echo -n $f1 $b '? '
		if ( $< == "y" ) then
		    set r="$r $b"
		endif
	    else
		echo -n $f1 $b '? '| fold
		if ( $< == "y" ) then
		    set r="$r $b"
		else
		    foreach n ($b)
			echo -n $f1 $n '? '
			if ( $< == "y" ) then
			    set r="$r $n"
			endif
		    end
		endif
	    endif
	endif
    endif
    shift
end
/bin/rm $force $recur $minus $r
out: 
exit(0)
