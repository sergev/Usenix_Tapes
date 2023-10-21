# makeloo - shell/awk program to generate a script of loo commands that
#	is a comprehensive coverage of everything the ships given as 
#	input can see.  Try to get a B and D in each sector (for 
#	range and sonar).  If neither B nor D is in a sector then use 
#	a S if present, and also one of the following: C, P, T, F, M if
#	present.
#	Creates a file called "looall" in current directory.  Use empire
#	"ex" command to execute the script (ex looall).

#	usage: makeloo

trap "rm -f /usr/tmp/ml.$$ /usr/tmp/ml.$$.q" 1 2 3 9 15
rm -f /usr/tmp/ml.$$ /usr/tmp/ml.$$.q
umask 077
rdship > /usr/tmp/ml.$$
echo rdship done

# Set up for awk:
# Save only ship no., type, and coord
# Change pt boat to pt_boat
# Change comma between x and y coord to space
sed  -e "s/ *[^ ]*%.*//" -e "s/pt b/pt_b/" -e "s/,/ /" < /usr/tmp/ml.$$ > /usr/tmp/ml.$$.q
echo sed done

# Sort by coordinate
sort -n -o /usr/tmp/ml.$$ +2 -3 +3 -4 /usr/tmp/ml.$$.q
echo sort done

umask 022
awk '
	{
		coord = $3 "," $4;
		if( x[coord] == 0 ) x[n++] = coord;
		x[coord]++;
		if( $2 == "pt_boat"   ) p[coord] = $1;
		if( $2 == "minesweep" ) m[coord] = $1;
		if( $2 == "destroyer" ) d[coord] = $1;
		if( $2 == "submarine" ) s[coord] = $1;
		if( $2 == "freighter" ) f[coord] = $1;
		if( $2 == "tender"    ) t[coord] = $1;
		if( $2 == "battleship") b[coord] = $1;
		if( $2 == "carrier"   ) c[coord] = $1;
	}
END	{
		ships = 0;
		loostr = "loo ";
		for( i=0; i<n; i++ ) {
			coord = x[i];
			batt = "";
			if( b[coord] != "" ) batt = b[coord];
			dest = "";
			if( d[coord] != "" ) dest = d[coord];
			sub = "";
			if( batt == "" && dest == "" ) {
				if( s[coord] != "" ) {
					sub = s[coord];
				}
			}
			other = "";
			if( batt == "" && dest == "" ) {
				if( c[coord] != "" )
					other = c[coord];
				else if( p[coord] != "" )
					other = p[coord];
				else if( t[coord] != "" )
					other = t[coord];
				else if( f[coord] != "" )
					other = f[coord];
				else if( m[coord] != "" )
					other = m[coord];
			}
			if( batt != "" ) {
				loostr = loostr batt "/";
				ships++;
			}
			if( dest != "" ) {
				loostr = loostr dest "/";
				ships++;
			}
			if( sub != "" ) {
				loostr = loostr sub "/";
				ships++;
			}
			if( other != "" ) {
				loostr = loostr other "/";
				ships++;
			}
			if( ships < 12 ) continue;
			print substr(loostr,1,length(loostr)-1);
			ships = 0;
			loostr = "loo ";
		}
		if( ships > 0 ) print substr(loostr,1,length(loostr)-1);
	}
' /usr/tmp/ml.$$ > looall
rm -f /usr/tmp/ml.$$ /usr/tmp/ml.$$.q
