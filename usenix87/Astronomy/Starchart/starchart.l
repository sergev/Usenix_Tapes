.TH starchart LOCAL 12/1/87
.UC
.SH NAME
stardsp, starpic, startek \- print astronomical star charts using Yale database
.SH SYNOPSIS
These programs generate star charts based on data extracted from the Yale
public domain star catalog.
Output is to the terminal 
.RB ( stardsp ),
Unix PIC file
.RB ( starpic )
or in Tektronix
vector format
.RB ( startek ).
Star data is optionally overlayed with other cosmic objects, such as planets.
.SH EXAMPLES 
.nf
# Orion: the belt lies near 5h40m in RA, just below the CE.
stardsp 5.6 -4.1 11.2 "Orion & belt" | more
.fi
.SH DESCRIPTION
The starchart center is specified by two parameters: floats defining Right
Ascension [0.0 to 24.0 hours] and Declination [-90.0 to +90.0 degrees].
An optional third parameter defines the N/S range of the output window, in
degrees of declination.
An optional fourth parameter gives a title.
.PP
The software reads data from a local file
.RB ( yale.star )
and optional planetary file
.RB ( planet.star )
and clips to a rectangular (in RA and DEC) window.
Objects within this window are projected onto the output media, and displayed
using the appropriate glyphs.
Present implementations draw two viewports: a "master" chart plus a
"thumbnail" overview with low limiting magnitude.
Their location and the limiting magnitude is specified by records in
the device driver, allowing the chart layout be tuned on a per-device basis.
The output is annotated with viewport boundaries, a legend and axis labels.
.PP
Sanson's sinusoidal projection is used to map coordinates.
This projection preserves both area and linearity in Declination (y axis).
It gives good conformality (angle correctness) near the equator, so it is
useful along the Ecliptic.
Lines of RA converge at the poles (unlike cylindrical projections),
though Cassiopeia and the Dipper reproduce well.
.SH BUGS
No testing for bogus ranges and scales which may wrap the poles.
The present implementation expects
.B yale.star
sorted by decreasing magnitude so that output is sequential, and then cuts off
below a limiting magnitude.
For more detailed charts spatial sorting is more appropriate.
.SH AUTHOR
Alan Paeth, University of Waterloo (AWPaeth@watCGL)
