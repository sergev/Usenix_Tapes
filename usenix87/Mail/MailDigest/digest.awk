#
# Awk Script called by 'digest'.  See DIGEST(1)
#
# Version two - sucks in the '>From' lines too...
#               (guarantees a "valid" return address!)

BEGIN {
	message = 0;
	last = "this should never be matched"
	in_header = 1;
        have_from = 0;
      }

$1 ~ /^From$/	 {  if (message == 0) {
		      print "----------------------------------------------------------------------"
		      print " "
		    }
		    else {
		      print "------------------------------"
	              print " "
	   	    }
		    message++;
	            in_header = 1;
	            from = $2;
		    parse_count = 0;
	            have_from = 0;
		    last = "we should have a hard time matching this, too"
	         }

$1 ~ /^From:/    { if (in_header) print $0; 
		   have_from = 1 }
$1 ~ /^>From/	 { parse_count++;
		   address = $2
		   if (parse_count > 1)
		     from = from"!"$10
	           else
		     from = $10
		 }
$1 ~ /^Subject:/ { if (in_header) print $0 }
$1 ~ /^Date:/    { if (in_header) print $0 }

length($0) < 2   { 
	           if (in_header == 1)
	             if (have_from == 0)
		       print "From: "from"!"address

		   if ($0 != last)
		     print $0
		   in_header = 0
		   last = $0
	         }

in_header == 0   { if ($0 != last) 
                     print $0
		   last = $0 
		 }
