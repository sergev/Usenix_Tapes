
#! /bin/sh
#set -x
for nfile in $* 
do
     sed -e '1s/^.* - \(D.*\) .*$/rm \/usr\/spool\/uucp\/D.apcisea\/\1/;
             2s/^.* - \(D.*\) .*$/rm \/usr\/spool\/uucp\/D.apciseaX\/\1/' /usr/spool/uucp/C./$nfile >/tmp/rmdata
     rm /usr/spool/uucp/C./$nfile
done
sh /tmp/rmdata
rm /tmp/rmdata
