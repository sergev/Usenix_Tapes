This is a suggested replacement for shar.  It is based on
the archive program in Kernighan and Plauger's Software Tools,
but has been heavily simplified.

It has the following advantages over shar:

1. it is not tied to Unix -- thus VMS users can unpack files without
   excessive effort.  Archc and archx should run without change on
   all Unix and Unix lookalike systems, as well as on VMS (VaxC)
   and all PDP-11 Decus C systems.  It has been in use for over 6
   years.

2. it does not execute the distributed image, but interprets it.  This
   means that trojan horses cannot be concealed in distributions.

3  The distribution file can be edited without damaging the archive.
   (Also, embedded archives can be handled).

It has the following disadvantages:

1. It is not as flexible as shar -- it cannot create directories or
   access any other Unix system services.

2. There is no checksum capability (it appears impossible to implement
   checksumming in a system-independent manner).

To use, save this message.  Then, use your favorite editor to extract
archx.c (delimited by lines beginning with "-h-" in column 1).  Then
compile archx and run it using the command:
	archx <this_file>
It should produce readme.txt, archx.c, and archc.c.
Manual pages can be produced by extracting the text delimited by
	#ifdef DOCUMENTATION
	...
	#endif

Please report problems to the author:

Martin Minow
decvax!minow

