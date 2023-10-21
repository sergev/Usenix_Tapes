.\" @(#) packet.driver.ms	Version hoptoad-1.3	87/03/24
.\"
.\" format this with [nt]roff -ms.
.\"
.\" From: greg@sgi.uucp (Greg Chesson)
.\" Newsgroups: mod.std.unix
.\" Volume-Number: Volume 9, Number 55
.\" Subject: Packet Driver Protocol
.\" Message-ID: <7136@ut-sally.UUCP>
.\" Date: 11 Feb 87 23:44:09 GMT
.\"
.\" This message contains a copy of ``Packet Driver Protocol,''
.\" written by G. L. Chesson while he was at Bell Laboratories.
.\" He remarks that it was approved for public distribution, and that
.\" 
.\" 	The version of the note that you probably have omits the
.\" 	detail that the transmitted checksum is really 0125252
.\" 	- the block checksum function.
.\" 
.\" [Note that 0125252 is 0xAAAA, which is easier to remember.
.\" I have folded this update into the document. -- hoptoad!gnu]
.ce
.B
Packet Driver Protocol
.R
.sp 1
.ce
G. L. Chesson
.br
.ce
Bell Laboratories
.SH
Abstract
.in +.5i
.PP
These notes describe the packet driver link
protocol that was supplied
with the
Seventh Edition of
.UX
and is used by the UUCP program.
.in -.5i
.SH
General
.PP
Information flow between a pair of machines
may be regulated by
first
representing the data 
as sequence-numbered 
.I
packets
.R
of data 
and then establishing conventions that
govern the use of sequence numbers.
The
.I
PK,
.R
or
.I
packet driver,
.R
protocol
is a particular instance of this type of
flow-control discipline.
The technique depends on the notion of a transmission
.I
window
.R
to determine upper and lower bounds for valid
sequence numbers.
The transmitter is allowed to retransmit packets
having sequence numbers
within the window until the receiver indicates that
packets have been correctly received.
Positive acknowledgement from the receiver moves the
window;
negative acknowledgement or no acknowledgement
causes retransmission.
The receiver must ignore duplicate transmission, detect
the various errors that may occur,
and inform the transmitter when packets are 
correctly or incorrectly received.
.PP
The following paragraphs describe the packet formats,
message exchanges,
and framing
used by the protocol as coded
in the UUCP program and the
.UX
kernel.
Although no attempt will be made here to present
internal details of the algorithms that were used,
the checksum routine is supplied
for the benefit of other implementors.
.SH
Packet Formats
.PP
The protocol is defined in terms of message
transmissions of 8-bit bytes.
Each message includes one
.I
control
.R
byte plus a
.I
data segment
.R
of zero or more information bytes.
The allowed data segment sizes range
between 32 and 4096 as determined by the formula
32(2\uk\d) where
k is a 3-bit number.
The packet sequence numbers are likewise constrained
to 3-bits; i.e. counting proceeds modulo-8.
.PP
The control byte is partitioned into three fields as
depicted below.
.bp
.nf
.sp 
.in 1i
.ls 1
bit	7	6	5	4	3	2	1	0
	t	t	x	x	x	y	y	y
.ls 1
.in -1i
.fi
.sp
The
.I
t
.R
bits indicate a packet type and
determine the interpretation to be placed on
the
.I
xxx
.R
and
.I
yyy
.R
fields.
The various interpretations are as follows:
.in +1i
.sp
.nf
.ls 1
.I
tt	interpretation
.sp
.R
00	control packet
10	data packet
11	`short' data packet
01	alternate channel
.ls 1
.fi
.sp
.in -1i
A data segment accompanies all non-control packets.
Each transmitter is constrained to observe the maximum
data segment size
established during initial synchronization by the
receiver that it sends to.
Type 10 packets have maximal size data segments.
Type 11, or `short', packets have zero or more data
bytes but less than the maximum.
The first one or two bytes of the data segment of a
short packet are `count' bytes that
indicate the difference between the
maximum size and the number of bytes in the short
segment.
If the difference is less than 127, one count
byte is used.
If the difference exceeds 127,
then the low-order seven bits of the difference
are put in the first data byte and the high-order
bit is set as an indicator that the remaining
bits of the difference are in the second byte.
Type 01 packets are never used by UUCP
and need not be discussed in detail here.
.PP
The sequence number of a non-control packet is
given by the
.I
xxx
.R
field.
Control packets are not sequenced.
The newest sequence number,
excluding duplicate transmissions,
accepted by a receiver is placed in the
.I
yyy
.R
field of non-control packets sent to the
`other' receiver.
.PP
There are no data bytes associated with a control packet,
the
.I
xxx
.R
field is interpreted as a control message,
and the
.I
yyy
.R
field is a value accompanying the control message.
The control messages are listed below in decreasing priority.
That is, if several control messages are to be sent,
the lower-numbered ones are sent first.
.in +1i
.nf
.ls 1
.sp
.I
xxx	name		yyy
.R

1	CLOSE	n/a
2	RJ		last correctly received sequence number
3	SRJ		sequence number to retransmit
4	RR		last correctly received sequence number
5	INITC	window size
6	INITB	data segment size
7	INITA	window size
.in -i
.ls 1
.fi
.sp
.PP
The CLOSE message indicates that the communications channel
is to be shut down.
The RJ, or
.I
reject,
.R
message indicates that the receiver has detected an error
and the sender should retransmit after using the 
.I
yyy
.R
field to update the window.
This mode of retransmission is usually
referred to as a
`go-back-N' procedure.
The SRJ, or
.I
selective reject,
.R
message carries with it the sequence number of
a particular packet to be retransmitted.
The RR, or
.I
receiver ready,
.R
message indicates that the receiver has detected
no errors; the
.I
yyy
.R
field updates the sender's window.
The INITA/B/C messages are used
to set window and data segment sizes.
Segment sizes are calculated by the formula
32(2\uyyy\d)
as mentioned above,
and window sizes may range between 1 and 7.
.PP
Measurements of the protocol running on communication
links at rates up to 9600 baud showed that
a window size of 2 is optimal
given a packet size greater than 32 bytes.
This means that the link bandwidth can be fully utilized
by the software.
For this reason the SRJ message is not as important as it
might otherwise be.
Therefore the
.UX
implementations no longer generate or respond to SRJ
messages.
It is mentioned here for historical accuracy only,
and one may assume that SRJ is no longer part of the protocol.
.SH
Message Exchanges
.SH
	Initialization
.PP
Messages are exchanged between four cooperating
entities: two senders and two receivers.
This means that the communication channel is thought of
as two independent half-duplex data paths.
For example the window and segment sizes need not
be the same in each direction.
.PP
Initial synchronization is accomplished
with two 3-way handshakes: two each of
INITA/INITB/INITC.
Each sender transmits INITA messages repeatedly.
When an INITA message is received, INITB is
sent in return.
When an INITB message is received
.I
and
.R
an INITB message has been sent,
an INITC message is sent.
The INITA and INITB messages carry 
with them the packet and window size that
each receiver wants to use,
and the senders are supposed to comply.
When a receiver has seen all three
INIT messages, the channel is 
considered to be open.
.PP
It is possible to design a protocol that starts up using
fewer messages than the interlocked handshakes described above.
The advantage of the more complicated design lies in its use as
a research vehicle:
the initial handshake sequence is completely symmetric,
a handshake
can be initiated by one side of the link while the
connection is in use, and the software to do this can
utilize code that would ordinarily be used only once
at connection setup time.
These properties were used in experiments with dynamically
adjusted parameters.
That is attempts were made to adapt the window and segment
sizes to changes observed in traffic while a link was in use.
Other experiments used the initial
handshake  in a different way
for restarting the protocol without data loss
after machine crashes.
These experiments never worked well in the packet driver and
basically provided the impetus for other protocol designs.
The result 
as far as UUCP is concerned is that initial synchronization
uses the two 3-way handshakes, and the INIT
messages are ignored elsewhere.
.SH
	Data Transport
.PP
After initial synchronization each receiver
sets a modulo-8 incrementing counter R to 0;
each sender sets a similar counter S to 1.
The value of R is always the number of the most recent
correctly received packet.
The value of S is always the first sequence number in
the output window.
Let W denote window size.
Note that the value of W may be different for each sender.
.PP
A sender may transmit packets with sequence numbers
in the range S to (S+W-1)\ mod-8.
At any particular time a receiver expects
arriving packets to have numbers in the range
(R+1)\ mod-8 to (R+W)\ mod-8.
Packets must arrive in sequence number order
are are only acknowledged in order.
That is,
the `next' packet a receiver
will acknowledge must have
sequence number (R+1)\ mod-8.
.PP
A receiver acknowledges receipt of data packets
by arranging for the value of its R counter to be
sent across the channel
where it will be used to update an S counter.
This is done in two ways.
If data is flowing in both directions across a
channel then each receiver's current R value is
carried in the
.I
yyy
.R
field of non-control packets.
Otherwise when there is no bidirectional
data flow,
each receiver's R value is transmitted across the link
as the
.I
yyy
.R
field of an RR control packet.
.PP
Error handling is up to the discretion
of the receiver.
It can ignore all errors in which case
transmitter timeouts must provide for
retransmission.
The receiver may also generate RJ 
error control packets.
The
.I
yyy
.R
field of an incoming RJ message replaces
the S value of the local sender and
constitutes a request for retransmission to start
at that sequence number.
The
.I
yyy
.R
field of an incoming SRJ message selects a particular
packet for retransmission.
.PP
The resemblance between the flow control procedure in the
packet driver and that defined for X.25 is no accident.
The packet driver protocol began life as an attempt at
cleaning up X.25.
That is why, for example,
control information is uniform in length (one byte),
there is no RNR message (not needed),
and there is but one timeout defined
in the sender.
.SH
	Termination
.PP
The CLOSE message is used to terminate communications.
Software on either or both ends of the communication
channel may initiate termination.
In any case when one end wants to terminate it sends
CLOSE messages until one is received from the other end
or until a programmable limit on the number of CLOSE
messages is reached.
Receipt of a CLOSE message causes a CLOSE message to be sent.
In the 
.UX
environment
it also causes the SIGPIPE or
`broken pipe' signal to be sent to
the local process using the communication channel.
.SH
	Framing
.PP
The term
.I
framing
.R
is used to denote the technique by which the
beginning and end of a message is detected
in a byte stream;
.I
error control
.R
denotes the method by which transmission
errors are detected.
Strategies for framing and error control depend
upon
additional information being transmitted along
with the control byte and data segment,
and the choice of a particular strategy usually
depends on characteristics of input/output
devices and transmission media.
.PP
Several framing techniques are in used in support
of PK protocol implementations,
not all of which can be described in detail here.
The technique used on asynchronous serial lines
will be described.
.PP
A six byte
framing
.I
envelope
.R
is constructed using the control byte
C of a packet and five other bytes as
depicted below.
.in +1i
<DLE><k><c0><c1><C><x>
.in -1i
The <DLE> symbol denotes the ASCII ctrl/P character.
If the envelope is to be followed by a data segment,
<k> has the value
log\d2\u(size)-4;
i.e. 1 \(<= k \(<= 8.
If k is 9, then the envelope represents a control packet.
The <c0> and <c1> bytes are the low-order and high-order
bytes respectively of 0xAAAA minus a 16-bit checksum.
For control packets, this 16-bit checksum is the same
as the control byte C.
For data packets, the checksum is calculated by the 
program below.
The <x> byte is the exclusive-or of <k><c0><c1><C>.
Error control is accomplished by checking 
a received framing envelope for compliance with the definition,
and comparing a checksum function of the data segment
with <c0><c1>.
.PP
This particular framing strategy assumes data segments
are constant-sized:
the `unused' bytes in a short packet are actually
transmitted.
This creates a certain amount of overhead which
can be eliminated by a more complicated framing technique.
The advantage of this strategy is that i/o
devices can be programmed to take advantage of the
constant-sized framing envelopes and data segments.
.bp
.PP
The checksum calculation is displayed below as a C function.
Note that the code is not truly portable because
the definitions of
.I short
and
.I char
are not necessarily uniform across all machines
that might support this language.
This code assumes that
.I short
and
.I char
are 16 and 8-bits respectively.
.PP
.in +.5i
.nf
.ft CW
.ls 1
/* [Original document's version corrected to actual version] */
chksum(s,n)
register char *s;
register n;
{
	register short sum;
	register unsigned short t;
	register short x;

	sum = -1;
	x = 0;

	do {
		if (sum<0) {
			sum <<= 1;
			sum++;
		} else
			sum <<= 1;
		t = sum;
		sum += (unsigned)*s++ & 0377;
		x += sum^n;
		if ((unsigned short)sum <= t) {
			sum ^= x;
		}
	} while (--n > 0);

	return(sum);
}
.fi
.in -.5i
.ft R
