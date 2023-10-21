#	sfont2defs.H
#	source file for definitions of special characters in the second
#	locally-defined special font
#	this file is processed by sed to produce sfont2defs.h
#	syntax is:
#	troff-name <white-space> { definition including optional comments
#		terminated by "}" on a line of its own
#		white space is minimised on output
#	anything following `#' is ignored
#	a line commencing with `#' is deleted
#	as are blank lines
#	note `\` should be escaped as "\\"


#	The bracket end pieces just load the stack with values which are then
#	used by the common procedure C.brk.end which does a
#	moveto rlineto rcurveto (then back to the start and across by 60)
#		rlineto rcurveto fill
#	This is only done to save the output file size and to make it
#	more confusing to interpret what is going on.
#	Remember the stack is loaded last in first out

lt	{
	0 150	   50 210	140 250		# inside rcurve to just below tip
	0 730
	0 150	   50 250	200 250		# outside rcurve to tip
	0 750				# rline up
	220 -250			# starting point
	C.brk.end
}

lb	{
	0 -150	   50 -210	140 -250	# inside rcurve to just below tip
	0 -730
	0 -150	   50 -250	200 -250	# outside rcurve to tip
	0 -750				# rline up
	220 750			# starting point
	C.brk.end
}

lk	{
	1 setlinewidth
	220 -250 moveto
	0 400 rlineto
	0 50    -50 100     -100 100  rcurveto
	50 0     100 50       100 100  rcurveto
	0 400 rlineto
	60 0 rlineto
	0 -400 rlineto
	0 -50    -50 -100     -100 -100  rcurveto
	50 0     100 -50       100 -100  rcurveto
	0 -400 rlineto
	closepath fill
}

rt	{
	0 150	   -50 250	-200 250	# outside rcurve to tip
	0 750				# rline up
	0 150	   -50 210	-140 250	# inside rcurve to just below tip
	0 730
	220 -250			# starting point
	C.brk.end
}

rb	{
	0 -150	   -50 -250	-200 -250	# outside rcurve to tip
	0 -750				# rline up
	0 -150	   -50 -210	-140 -250	# inside rcurve to just below tip
	0 -730
	220 750			# starting point
	C.brk.end
}

rk	{
	1 setlinewidth
	220 -250 moveto
	0 400 rlineto
	0 50    50 100       100 100  rcurveto
	-50 0     -100 50     -100 100  rcurveto
	0 400 rlineto
	60 0 rlineto
	0 -400 rlineto
	0 -50    50 -100       100 -100  rcurveto
	-50 0     -100 -50     -100 -100  rcurveto
	0 -400 rlineto
	fill
}


#		the floor and ceiling bracket parts are made up with two
#		procedures defined within the context of the font:
#			C.bv	which draws the bold vertical
#			C.barc	which adds the horizontal piece to the ceiling;
#				the X coord on the stack should be set to the
#				left edge of where the bar is to be put
#			C.barf	likewise for floor

lc	{
	C.bv				# draw the bv part
	280 				# move to bottom right edge of vert bar
	C.barc				# and draw in horizontal bar
}

lf	{
	C.bv 280 C.barf
}

rc	{
	C.bv 40 C.barc
}

rf	{
	C.bv 40 C.barf
}

br	{
	40 C.setl
	0 -250 moveto
	0 1000 rlineto
	stroke
}

rn	{
	40 C.setl
	0 895 moveto
	500 0 rlineto
	stroke
}

ci	{
	40 C.setl
	500 250 				# centre
	400					# radius
	0 360 arc				# draw circle
	stroke
}

sp_6	{		# 1/6 em narrow space
			# do nothing, the width is all that is needed
}

sp_12	{		# 1/12 em narrow space
			# null op
}

r1	{		# reversible chemical reaction
	40 C.setl
	0 setlinejoin
	700 180 moveto -650 currentlinewidth add 0 rlineto 200 -200 rlineto
	50 360 moveto 650 currentlinewidth sub 0 rlineto -200 200 rlineto
	stroke
}

r2	{		# reversible reaction with full arrowheads
	40 C.setl
	2 setlinejoin
	217 18 moveto
					# left arrowhead
	-150 150 rlineto 150 150 rlineto -150 -150 rlineto
	633 0 rlineto
	50 360 moveto
	633 0 rlineto
	-150 150 rlineto 150 -150 rlineto -150 -150 rlineto
	stroke
}
