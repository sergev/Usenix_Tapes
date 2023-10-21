cd /usr/adm
/sys/prog/mgt.sum
/bin/date >/tmp/sa.rpt
/bin/echo "COMMAND	   EXECUTIONS	    REAL TIME	    USER TIME	  SYSTEM TIME	JLF" >>/tmp/sa.rpt
/bin/echo -n "" >>shm.hold
/bin/echo -n "" >>sht.hold
/bin/cp shm.hold shm
/bin/cp sht.hold sht
/sys/prog/sa -aclts /usr/adm/sh2.hold >>/tmp/sa.rpt
/bin/cp shm shm.hold
/bin/cp sht sht.hold
/bin/if $Dx != 1x /bin/goto daily
: monthly
set e = "Cmd.rpt "$X/$Y
/bin/opr -c 2 -h $e /tmp/sa.rpt
/bin/empty /usr/adm/sht
/bin/cp /tmp/sa.rpt /usr/adm/sa.$X.rpt
: daily
/bin/mv /tmp/sa.rpt /usr/adm/sa.rpt
set d = "mgt.rpt "$X/$D/$Y
/bin/mgt.rpt | /bin/opr -c 2 -h $d
/bin/exit
Name: /sys/prog/mrpt

char *prog_id "%Z%%M% Release %R% Level %L% Branch %B% Sequence %S% %D% %T%\n";

Function: Perform daily command accounting, and
	generate daily management reports.

Algorithm: Uses shell accounting "sa" to process
	shell accounting data, and uses "mgt.rpt"
	to generate reports.  At end of month
	consolodated report is generated for
	all commands executed for the entire month.

Parameters: none

Returns: nothing

Globals:

Calls:
	/sys/prog/sa
	/bin/mgt.rpt


Called by:
	cron, early in the morning.

Files and Programs:
	/tmp/sa.rpt - cumulative command summary
	/usr/adm/sa.rpt - available cumulative command summary
	/usr/adm/sa.M.rpt - Monthly report for month "M"
	/usr/adm/sht - Summary file
	/usr/adm/shm - Summary file
	/usr/adm/sht.hold - Holding area
	/usr/adm/shm.hold - Holding area

Notes and Memorabilia:

Installation Instructions:
	Place in /sys/prog with mode 700.
	Install ececution of mrpt in /lib/crontab
	for some time early in morning (/sys/prog/sa
	is a real system hog).

History:
	Written in may 1980, by Charlie Muir, AFDSC, as part of the
	UNIX management reporting system.
