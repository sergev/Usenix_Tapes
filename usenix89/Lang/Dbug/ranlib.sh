
# Warning - first line left blank for sh/csh/ksh compatibility.  Do not
# remove it.  fnf@Unisoft

# ranlib --- do a ranlib if necessary

if [ -r /usr/bin/ranlib ]
then
	/usr/bin/ranlib $*
elif [ -r /bin/ranlib ]
then
	/bin/ranlib $*
else
	:
fi
