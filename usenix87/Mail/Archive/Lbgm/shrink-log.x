#! SHELL
# shrink-log:
#	compress out un-necessary lines from a header.
#	NOT CALLED by the user directly

LOGFILE=SAVELOG
awk 'BEGIN { file="" }
	/^Jan_|^Feb_|^Mar_|^Apr_|^May_|^Jun_/ {file = $0}
	/^Jul_|^Aug_|^Sep_|^Oct_|^Nov_|^Dec_/ {file = $0}
	/^	Subject:/	{sub=$0}
	/^	Message-ID:/	{mid=$0}
	/^	Path:/		{path=$0}
	/^	Newsgroups:/	{ng=$0}
	/^	From:/		{from=$0}
	{ if ( $0 ~ /^[	 ]*$/ )
		printf "%s\n%s\n%s\n%s\n%s\n%s\n\t\n", \
			file, mid, sub, from, path, ng}' $LOGFILE
