#! SHELL
cd PROCESS
trace=/tmp/TRACE-$$
at 1800 <<EOF
./all-quick-req > ${trace} 2>&1
/usr/ucb/Mail -s "Trace of archive requests" ${archiveperson} < ${trace}
/bin/rm -f ${trace}
EOF

