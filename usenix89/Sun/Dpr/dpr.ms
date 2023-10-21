.RP
.TL
Screen dump printing on the Sun family of Workstations.
.AU
Rich Burridge.
.AI
Sun Computer Australia.
.AB
This document describes the general purpose screen dump printing utility "dpr" which
has been implemented on the Sun, including a description of how it was designed.
The current status of the project is given including future plans and known
limitations and problems.
.PP
The original paper was written on August 4, 1985. This paper has been substantially
revised in light of experience and with furthur ideas in mind.
.AE
.XS   2
History
.XA   3
Limitations
.XA   4
Design
.XA   5
Printer Capabilities
.XA   8
Examples
.XA   9
Screen dump printing
.XA   11
Additional software and features
.XA   12
Future Plans
.XA   13
Conclusions
.XE
.PX
.bp
.SH
History.
.PP
There are dozens of different printers on the market capable of producing a
copy on paper of a portion or the whole of a bitmapped graphics screen.
Almost every one has a different method of producing the screen image.
.PP
 Dpr is an attempt to try to rationalise the
way a new printer would be attached to a graphics computer in this case the
Sun range of workstations, for screen image copying.
Instead of having to write a new piece of software each time, an approach
similar to terminal and normal line printer attachment under Unix BSD 4.2
is used. This results in a working version of the screendump for the new
printer in minutes instead of days. It also allows for one simple piece of
software plus one file containing printer characteristics to be distributed,
instead of several separate software drivers.
.bp
.SH
Limitations.
.PP
As is to be expected with any piece of software that attempts to generalise,
it is not expectionally fast or optimised. What should also be considered is
the physical interface.
In this case it is the RS232 serial interface which is standard on most computers
 (certainly every Sun) and also can use relatively inexpensive printers.
This is a very slow method of output, normally at 9600 baud.
.PP
Consider the example of dumping the
whole screen of a monochrome Sun 2 to a printer at 9600 baud. The screen is
1152 wide by 900 pixels high by 1 pixel deep. Not allowing
for leading and trailing control characters or for formatting of data, the
minimum time to print the screen is 108 seconds. This also does not allow
for any physical movements needed by the printer.
.PP
If speed is a vital requirement, then a different interfacing method should
be used, ideally DMA. This is not cheap, and also is not easy to program.
.bp
.SH
Design.
.PP
Several approaches were considered:
.IP (1)
A library of subroutines which can be incorporated within user programs.
.IP (2)
A program/subroutine which has several parameters describing how to do
the screen dump.
.IP (3)
Similar to the Unix BSD 4.2 termcap (for terminals) and printcap
(for printers) facilities.
.PP
Printcap can achieve the required results "after a fashion", but needs a
special piece of software to filter the output. Dpr takes this one step
further. It uses the file "/etc/dumpcap" which is a simplified version of
the termcap / printcap data bases. This allows for dynamic addition and
deletion of printers. Each entry in the database describes one printer.
.PP
The default printer is lp, though the Unix environment variable PRINTER may be
used to override this. Dpr uses a -Pprinter command line option for explicitly
naming a destination printer.
.bp
.SH
Printer Capabilities.
.PP
Each entry in the dumpcap file describes a printer and is a line consisting
of a number of fields separated by ':' characters. The format of each entry
is identical to the Unix BSD 4.2 printcap format, consisting of boolean, numeric and
string entries describing printer capabilities. The following is a list of
those capability codes, their type and default values plus a brief
description.
.PP
Name   Type    Default
.PP
 br   numeric   9600
.sp
.PP
Set the baud rate. This is only applicable if the output device is a tty line.
 Valid values are 110, 150, 300, 1200, 1800, 2400, 4800, 9600 19200 and 38400.
Obviously there are not many printers on the market which can go at 19200 or 38400
but the Sun is capable of handling these speeds.
.PP
 dd   boolean   false
.sp
.PP
The printed screen image should be double sized. This means that for every single
pixel of the screen image, a 2x2 matrix is printed. As is expected, this
requires considerable extra processing.
.PP
 ed   string    null
.sp
.PP
Sequence to output at end of dump. In a normal situation this usually achieves a
return to text mode (from graphics mode) and setting the line spacing to default.
.PP
 el   string    null
.sp
.PP
Sequence to output at end of 'line'. Line here is considered to mean one
horizontal pass. This sequence usually is a linefeed.
.PP
 fn   string    null
.sp
.PP
Screen image file to use. If this is not supplied then a copy of the current
screen is printed. Files produced by screendump (or something similar) in
standard Sun rasterfile format may also be used.
.PP
 ha   numeric   0
.sp
.PP
Handshake between the Sun and the printer.
.br
(0 = None, 1 = XON/XOFF, 2 = DTR, 3 = CTS, 4 = RTS).
.br
In cases 2-4 the Sun will look at the state of the pin inside the RS232 plug
(using a special ioctl call), and either send or stop sending data depending
upon whether it is high or low. This will require a special
cable being produced to allow this operation to happen successfully.
.PP
 lp   string  "/dev/lp"
.sp
.PP
Name to open for output. This normally is a tty line.
It should be noted that this can also be a Unix filename (see example /etc/dumpcap
entries below).
.PP
 ma   numeric   0
.sp
.PP
Mask to be used for preparing output. Some printers require that certain bits in
each graphics byte be set, as well as the data image bits. The Phillips GP300 is
a typical example.
.PP
 nb   numeric   8
.sp
.PP
Bits per byte for output of image. This is the number of image bits in one byte
that the printer requires.  Normally 6,7 or 8.
.PP
 ro   boolean   false
.sp
.PP
Whether to reverse the order the graphics bits are placed in each graphics byte.
False means they are entered from left to right.
.PP
 sb   string    null
.sp
.PP
Sequence to output before printing the burst page. This will usually achieve the
initialisation of the printer, selection of any required fonts etc.
.PP
 sd   string    null
.sp
.PP
Sequence to output at start of dump. This usually will achieve the setting of
graphics mode and the required line spacing.
.PP
 sh   numeric   900
.sp
.PP
Height of the screen dump. 900 is the whole of a Sun 2 screen. It is possible to
dump any single rectangular portion of that.
.PP
 sl   string    null
.sp
.PP
Sequence to output at start of 'line'. Line here is considered to mean one
horizontal pass. Some printers require graphics mode to be set at the start of
each 'line'. That would be achieved using this parameter.
.PP
 sw   numeric   1152
.sp
.PP
Width of the screen dump. 1152 is the width of a Sun 2 screen. It is possible to
dump any single rectangular portion of that.
.PP
 sx   numeric   0
.sp
.PP
X origin of screen dump. For a Sun 2, this can be anything in the range 0-1151.
0 is at the left side of the screen.
.PP
 sy   numeric   0
.sp
.PP
Y origin of screen dump. For a Sun 2, this can be anything in the range 0-899.
0 is at the top of the screen.
.PP
 vs   numeric   1
.sp
.PP
Number of vertical lines, printer uses in one pass. Certain printers have several
pins on the printer head which will either fire or not depending whether
the associated pixel is black or white. Usually these are arranged vertically on
the print head.
.bp
.SH
Examples.
.IP (1)
To output a complete image of the current screen at double size to a
Toshiba P1350 printer using a CTS handshake over /dev/ttyb at 4800 baud,
the following entry in /etc/dumpcap would be used:
.sp
lp|to|Toshiba P1350:\\
.br
.RS
.RS
:br#4800:lp=/dev/ttyb:ha#3:vs#24:nb#6:dd:\\
.br
:sd=\\E^^\\b:ed=\\E^^\\t:sl=\\E;2304:el=\\n:
.RE
.RE
.IP (2)
To output a screen image from the file "screen.saved" into the file
"screen.filtered", so that it will be correctly formatted for later
sending to a Toshiba P1350, would use the following entry:
.sp
fi|Toshiba P1350 filtered:\\
.br
.RS
.RS
:fn=screen.saved:lp=screen.filtered:vs#24:nb#6:\\
.br
:sd=\\E^^\\b:ed=\\E^^\\t:sl=\\E;1152:el=\\n:
.PP
.RE
.RE
This is particularly useful if the printer is normally being used for
standard lpr Unix spooling, and the image of the screen can be
incorporated with that.
.bp
.SH
Screen dump printing.
.PP
Dpr is the utility that will output the screen image to the printer. It has
several command line parameters:
.IP -Pprinter
.br
Force output to a named printer. Normally the default printer is
used (site dependent), or the value of the PRINTER environment
variable may be used.
.IP -#num
.br
Produce multiple copies of output using num as the number of
copies.
.IP -Cclass
.br
Print class as the job classification on the burst page. This
replaces the system name (the name returned by hostname).
.IP -Jjob
.br
Print job as the job name on the burst page. Normally dpr uses
the filename specified by the "fn" capability switch in the
/etc/dumpcap file, or "current" if no switch was specified.
.IP -f
Generate the C code necessary to take a file in standard Sun rasterfile
format on standard program input and produce a filtered output version
on standard program output. This could then be incorporated with the
Unix line printer spooling program lpr to produce furthur raster dumps
directly.
.IP -r
Remove the screen dump file (if specified by "fn" in /etc/dumpcap),
upon completion of spooling.
.IP -m
Send mail upon completion.
.IP -h
Suppress printing the burst page.
.IP -Ddumpcap
.br
Alternate dumpcap file to use instead of /etc/dumpcap to specify
printer characteristics.
.bp
It was decided not to make the screen dumping use a daemon process like lpr
for several reasons:
.IP (1)
As opposed to line printer listings, screen dumping is rarely done.
.IP (2)
To prevent another daemon process slowing done the machine.
.IP (3)
To save having several copies of screen image files lying around in
spooling directories.
.IP (4)
Due to other CPU intensive processes, the copying of the screen to a
file might not be achieved immediately.
.IP (5)
The resulting program would be much more complicated.
.bp
.SH
Additional software and features.
.PP
It is expected that in most situations just the contents of a single window
will be required to be dumped to the printer. A special C routine is provided
called "print_window", which can be incorporated within a user program to
achieve this.
.PP
It determines which window it is running in, and puts a copy of the screen
image in a file in the /tmp directory using the process id to achieve a unique
name. It also creates an alternate dumpcap file in /tmp using the current
window to determine the height, width and starting position of the dumped
output portion.
.PP
It takes the remaining details from the /etc/dumpcap file using the printer
name specified as a parameter to "print_window". From these details it forks
dpr as another process before terminating.
.PP
Getting the software working is only about 25% of the work involved in getting
a printer to correctly dump a screen image. The rest of the time is spent on
the hardware. This means a correct setting for internal printer switches and
jumpers plus an appropriate RS232 cable. This information is quite often
written down on paper which is stored separately and is never to hand when
needed. To make life simpler for people who are trying to implement screen
dumping to a printer which already has an entry in the dumpcap file, it is
suggested that the associated hardware information should be written as
comments above the printers dumpcap entry.
.PP
Be aware that a very unusual printer may exposes deficiencies in the ability
of the dumpcap file to dscribe it, or bugs in dpr.
.bp
.SH
Future Plans
.PP
From experience I believe virtually every monochrome printer should work using
the above dumpcap capabilities database. There are several color printers on the
market nowadays and more capabilities will be added to try to make color
screen dumping general purpose too.
.PP
Transfer of this software to other Unix graphics machines is desired to try to make
the code as portable as possible and all the machine dependent bits to rely on
conditional combilation switches.
.bp
.SH
Conclusions.
.PP
This software should form the basis for getting all RS232 screen dump printers
connected to the Sun, working in the future. If distributed properly and with
contributions from dpr users on properly connected printers, life should get
easier and easier.
.PP
It is recognised that these are early days and that this second version of the
software will still contain flaws and ineffiencies in it. This should improve
in future versions. Optimistically it is forseen that this program should
be included in a future version of the Sun Unix operating system, and even
on a grander scale, to be included as a standard AT&T Unix utility with conditional
compilation code when required.
