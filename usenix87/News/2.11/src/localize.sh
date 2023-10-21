rm -f Makefile
cp Makefile.dst Makefile
chmod u+w Makefile
ed - Makefile  <<'EOF'
g/^#USG /s///
g/^#V7 /d
g/^#BSD4_[123] /d
g/#NOTVMS/s/#NOTVMS.*//
/^SPOOLDIR/s:spool:spool:
/UUXFLAGS/s/-r -z/-r -z -gd/
g/-ltermlib/s//-lcurses/
w
q
EOF
rm -f defs.h
cp defs.dist defs.h
chmod u+w defs.h
ed - defs.h << 'EOF'
/ROOTID/s/10/100/
/N_UMASK/s/000/002/
/DFLTEXP/s/2\*WEEKS/1\*WEEKS/
/define TMAIL/s:ucb/Mail:bin/mailx:
/PAGE/s:usr/ucb:usr/bin:
/UUPROG/s:/\* ::
/INTERNET/s:/\* ::
/UNAME/s:/\* ::
/DOXREFS/s:/\* ::
/LOCKF/s:/\* ::
/SENDMAIL/s:/\* ::
/MYORG/s/".*"/"Plus Five Computer Services, St. Louis"/
w
q
EOF
