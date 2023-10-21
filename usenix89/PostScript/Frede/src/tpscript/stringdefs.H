#	source file for definitions for special characters in postscript
#	this file is processed by sed to produce stringdefs.h
#	syntax is:
#	troff-name <white-space> { definition including optional comments
#		terminated by "}" on a line of its own
#		white space is minimised on output
#	anything following `#' is ignored
#	a line commencing with `#' is deleted
#	as are blank lines
#	note `\` should be escaped as "\\"


12	{
	(1) (2) Cfract
}

14	{
	(1) (4) Cfract
}

34	{
	(3) (4) Cfract
}

13	{
	(1) (3) Cfract
}

18	{
	(1) (8) Cfract
}

23	{
	(2) (3) Cfract
}

38	{
	(3) (8) Cfract
}

58	{
	(5) (8) Cfract
}

78	{
	(7) (8) Cfract
}

ru	{
	0 ysiz pt 5 div 2 copy rmoveto		# up dist from bottom to baseline
	(_) show
	neg rmoveto	# back down
}


sr	{		# square root needs to be shoved to the right a touch
	currentfont			# save for later restoring
	/dx xsiz pt 0.05 mul def
	dx 0 rmoveto			# jump right by a bit
	f.S [ xsiz pt 0 0 ysiz pt 0 0 ]makefont setfont
	(\\326)s
	dx neg 0 rmoveto		# back again
	setfont				# restore prev font
}
	

sq	{		# draw a square box
	currentlinewidth currentpoint currentpoint
	ysiz pt 20.8 div dup dtransform round idtransform setlinewidth pop
	newpath moveto
	/dy ysiz pt 0.8 mul def		# side is 0.8em
	/dx xsiz pt 0.8 mul def		# side is 0.8em
	dx 8 div dy .1875 mul neg rmoveto	# move to starting point
	0 dy rlineto dx 0 rlineto
	0 dy neg rlineto
	closepath
		# the following search will put a different number of elements
		# on the stack depending whether true or false; thus we mark
		# here to clean up later
	mark
	currentfont /FontName get	# get the font name
	40 string cvs			# turn into a string
	(Bold) search			# is it bold type?
		{fill}			# if so then block in
		{stroke}		# otherwise outline
	ifelse
	cleartomark			# clean up stack
	moveto				# back to start
	xsiz pt 0 rmoveto			# to the end of the char
	setlinewidth
}

ff	{
	(f) show xsiz pt 20 div neg 0 rmoveto (f) s
}

Fi	{
	(f) show xsiz pt 20 div neg 0 rmoveto (\\256) s
}

Fl	{
	(f) show xsiz pt 20 div neg 0 rmoveto (\\257) s
}


#		fraction construction common procedure
#		this won't work properly yet for fonts slanted with \S
fract	{
	/fbot exch def			# arg 1 is bottom character
	/ftop exch def			# arg 2 is top character
	currentfont currentpoint	# for later restoration
	/f0 ysiz pt 2 div def		# little chars half size
	(0) stringwidth pop 2 div 0 rmoveto	# charwidth at half size
	(\\244)show			# draw the slanting line
	currentpoint 4 2 roll moveto	# save current and back to beginning
	0 f0 0.75 mul rmoveto		# up a bit
	fonttype [ xsiz pt 2 div 0 0 f0 0 0 ]makefont setfont
					# set a smaller font size
	ftop show
	moveto 				# back to the end of the /
	fbot show
	setfont				# restore font
}

