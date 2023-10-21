BEGIN {errors=0; badlinecounts=0; received=0; posted=0; dups=0;
	cancelled=0; cfail=0; csuccess=0; cgroups=0; ctlmsgs=0; old=0}
/Bad option/{next}
/Duplicate/{dups++; next}
/Ctl Msg/  {ctlmsgs++; next}
/Article too old/  {old++; next}
/posted/   {posted++; next}
# Sent to corresponds to posted, except for local group postings.
/sent to/     {next}
/subj 'cmsg cancel/{cancelled++; next}
/subj 'cancel/{cancelled++; next}
/Can't cancel/{cfail++; next}
/Cancelling/{csuccess++; next}
/cancel article/{cgroups++; next}
/from/ {next}
/received/ {received++; next}
/linecount/{badlinecounts++; next}
	{if (length==0) next;
	if (errors++ == 0) {
		print "Miscellaneous errors:";
		}
	 print; next}
/cancel/ {next}
END {print "     SUMMARY"
     if (posted)
	print posted,"articles posted."
     if (ctlmsgs)
	print ctlmsgs,"control messages received."
     if (received)
	print received,"articles received."
     if (old)
	print old " old articles moved to junk."
     if (dups)
	print dups,"duplicate articles rejected."
     if (badlinecounts)
	print badlinecounts,"articles/duplicates had bad line counts."
     if (cancelled) {
	print cancelled " articles cancelled, " cfail " unsuccessfully,"
	print "   " csuccess " successfully in " cgroups " groups."
     }
}
