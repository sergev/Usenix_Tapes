#! SHELL
# to process the end of the month, update the Current directroy
# etc.  This calculates the new month/old month name, and
# then calls mv-current-to with the correct parameters.
#
# run this from 'cron' with the following:
# 30 0 1 * * su news -c eom
#
PATH=/usr/lib/news/archive:/bin:/usr/bin:/usr/ucb:/usr/local:
export PATH
set `date`
mon=$2
day=$3
year=`echo $6 | sed -e 's/19//'`
nextyear=${year}
to=0
if [ ${day} -gt 15 ]
then
    to=1
fi
case ${to} in
    1) case ${mon} in
	"Jan")
	    nextmon="Feb"
	    ;;
	"Feb")
	    nextmon="Mar"
	    ;;
	"Mar")
	    nextmon="Apr"
	    ;;
	"Apr")
	    nextmon="May"
	    ;;
	"May")
	    nextmon="Jun"
	    ;;
	"Jun")
	    nextmon="Jul"
	    ;;
	"Jul")
	    nextmon="Aug"
	    ;;
	"Aug")
	    nextmon="Sep"
	    ;;
	"Sep")
	    nextmon="Oct"
	    ;;
	"Oct")
	    nextmon="Nov"
	    ;;
	"Nov")
	    nextmon="Dec"
	    ;;
	"Dec")
	    nextmon="Jan"
	    nextyear=`expr ${year} + 1`
	    ;;
       esac
       ;;
    0) case ${mon} in
	"Jan")
	    nextmon="Dec"
	    nextyear=`expr ${year} - 1`
	    ;;
	"Feb")
	    nextmon="Jan"
	    ;;
	"Mar")
	    nextmon="Feb"
	    ;;
	"Apr")
	    nextmon="Mar"
	    ;;
	"May")
	    nextmon="Apr"
	    ;;
	"Jun")
	    nextmon="May"
	    ;;
	"Jul")
	    nextmon="Jun"
	    ;;
	"Aug")
	    nextmon="Jul"
	    ;;
	"Sep")
	    nextmon="Aug"
	    ;;
	"Oct")
	    nextmon="Sep"
	    ;;
	"Nov")
	    nextmon="Oct"
	    ;;
	"Dec")
	    nextmon="Nov"
	    nextyear=`expr ${year} + 1`
	    ;;
       esac
       ;;
esac

if [ ${to} = 1 ]
then
    current=${mon}_${year}
    next=${nextmon}_${nextyear}
else
    current=${nextmon}_${nextyear}
    next=${mon}_${year}
fi

# echo "mv-current-to ${next} ${current}"
mv-current-to ${next} ${current}
