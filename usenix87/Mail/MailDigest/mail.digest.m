
# mail_issue - This shell script mails out the previously created digest...
#

       digest="outbound.digest"
      archive="Digest."
       volume="volume_number"

# does the file exist???

if [ -r $digest ]
then
  volnum=`cat $volume`

  (mailx -s "$digest_name, #$volnum" $to < $digest

  mv $digest $archive$volnum;
  rm -f $digest;

  rm -f $volume;
  echo Incrementing volume number..
  echo `expr $volnum + 1` > $volume) > $log 2>&1 &
  
fi

exit 0
