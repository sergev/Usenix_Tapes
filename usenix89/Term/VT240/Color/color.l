.TH COLOR 1L
.UC
.SH NAME
.I color 
\- Change colors on a VT240 terminal.
.SH SYNOPSIS
.I color
[-t text-color] [-b background-color] [-c cursor-color] [-h]
.SH DESCRIPTION
.I Color
will change the color on a VT240 terminal.
If no options are specified, then the default colors
are selected. Otherwise, the options are;
.TP 1.0i
.B "-h"
Help - shows the options and possible colors.
.TP
.B "-t text-color"
Change Text Color - Changes the color of the text.
.TP
.B "-b background-color"
Change Background Color - Changes the color of the background.
.TP
.B "-c cursor-color"
Change Cursor Color - Changes the color of the cursor.
.PP
The options t, b, and c can be used in the same command.
The option h will cause all others to be ignored.
.SH COLORS
The following are the available colors:
.PP
aquamarine, md-aquamarine, black, blue, cadet-blue, 
cornflwr-blue, dk-slate-blue, lt-blue, lt-steel-blue, 
md-blue, md-slate-blue, midnight-blue, navy-blue,
sky-blue, slate-blue, steel-blue, coral, cyan, firebrick, 
gold, goldenrod, md-goldenrod, green, dk-green,
dk-olive-green, forest-green, lime-green, md-forest-green, 
md-sea-green, md-spring-green, pale-green, sea-green,
spring-green, yellow-green, dk-slate-grey, dim-grey, 
lt-grey, khaki, magenta, maroon, orange, orchid, dk-orchid, 
md-orchid, pink, plum, red, indian-red, md-violet-red, 
orange-red, violet-red, salmon, sienna, tan, thistle, 
turquoise, dk-turquoise, md-turquoise, violet, violet-blue,
wheat, white, yellow, yellow-green

.SH AUTHOR
Xev Gittler, SUNYA, 1987
