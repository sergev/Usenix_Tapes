# pl - shell program to enter empire, save output, provide minimal security

#	usage: pl [ > /dev/null ]

trap "chmod 700 .;mv .yout .oyout" 1 2 3 9 15
chmod 711 .			# open up read access for empire
empire Cname rep | tee eout	# execute the script (.yout)
chmod 700 .			# prevent access by snoopy adversaries
mv .yout .oyout			# save script in case it's needed
