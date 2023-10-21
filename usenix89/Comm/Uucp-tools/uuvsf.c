
sed -e '1s/^.* - \(D.*\) .*$/cat \/usr\/spool\/uucp\/D.apcisea\/\1/;
             2d' /usr/spool/uucp/C./$1 >vdata
sh vdata
rm vdata

