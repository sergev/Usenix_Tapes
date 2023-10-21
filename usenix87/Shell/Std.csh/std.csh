#! /bin/csh -f
unset verbose
set outfile
set unkflag
set files
foreach argument ( $* )
    while ( 1 )
        switch ( $argument )
            case "-" :
                echo 'NULL FLAG'
                break
            case "-v*":
		set verbose 
		breaksw
            case "-o*":    
                set outfile = `expr $argument : '-.\(.*\)'` || set outfile = UNKNOWN
                break
            case "-*":    
                echo Unknown flag: \"$argument\"
                set unkflag = "$unkflag $argument"
                break
            case "*":    
                switch ( UNKNOWN )
                    case "$outfile":
			set outfile = $argument
			breaksw
                    default:        
			set files = "$files $argument" 
                endsw
                break
        endsw
        set argument = -`expr "$argument" : '-.\(.*\)'` || break
    end
end

# The following is just for testing this standard script
echo ' Argument analysis to' $0 ':'
echo Flags: -v = ${?verbose}, -o = \"$outfile\"
echo Unknown flags: $unkflag
echo Files: $files
exit

