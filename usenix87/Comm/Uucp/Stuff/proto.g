From wuphys!wucs!busch!we53!mgnetp!ihnp4!cbosgd!clyde!burl!ulysses!allegra!bellcore!decvax!genrad!panda!talcott!encore!wegrzyn 25 
Article 2161 of net.unix:
Relay-Version: version B 2.10.2 9/17/84; site plus5.UUCP
Posting-Version: version B 2.10 beta 3/9/83; site encore.UUCP
Path: plus5!wuphys!wucs!busch!we53!mgnetp!ihnp4!cbosgd!clyde!burl!ulysses!allegra!bellcore!decvax!genrad!panda!talcott!encore!wegrzyn
>From: wegrzyn@encore.UUCP (Chuck Wegrzyn)
Newsgroups: net.unix,net.unix-wizards
Subject: UUCP protocol information
Message-ID: <168@encore.UUCP>
Date: 25 Feb 85 15:57:23 GMT
Date-Received: 28 Feb 85 07:12:38 GMT
Organization: Encore Computer Corp., Wellesley Hills, MA
Lines: 311
Xref: wucs net.unix:2649 net.unix-wizards:5144


	I am posting this over the network because I believe that
	others are interested in knowing the protocols of UUCP.
	Below is listed all the information that I have acquired
	to date. This includes the initial handshaking phase, though
	not the login phase. It also doesn't include information
	about the data transfer protocol for non-packet networks
	(the -G option left off the uucico command line). But, just
	hold on - I am working on that stuff.

	For a point of information : the slave is the UUCP site being
	dialed, and the master is the one doing the calling up. The
	protocols listed in the handshaking and termination phase are
	independent of any UUCP site : it is universal. The stuff in
	the work phase depends on the specific protocol chosen. The
	concepts in the work phase are independent of protocol, ie. the
	sequences are the same. It is just the lower level stuff that
	changes from protocol to protocol. I have access only to level
	g and will document it as I begin to understand it.

	Most of the stuff you see here is gotten from the debug phase
	of the current BSD UUCP system.

	I hope this is useful. Maybe this will get some of the real
	'brains' in UUCP to get off their duffs and provide some real
	detail. In any case, if you have any questions please feel
	free to contact me. I will post any questions and answers over
	the network.


				Chuck Wegrzyn
				{allegra,decvax,ihnp4}!encore!wegrzyn

				(617) 237-1022



			UUCP Handshake Phase
			====================

Master							Slave
------							-----

					<-----		\020Shere\0     (1)


(2)  \020S<mastername> <switches>\0	----->


					<-----		\020RLCK\0      (3)
							\020RCB\0
							\020ROK\0
							\020RBADSEQ\0

					<-----		\020P<protos>\0 (4)


(5) \020U<proto>\0			----->
    \020UN\0


(6) ...


(0) This communication happens outside of the packet communication that
	is supported. If the -G flag is sent on the uucico line, all
	communications will occur without the use of the packet
	simulation software. The communication at this level is just
	the characters listed above.

(1) The slave sends the sequence indicated, while the master waits for
	the message.

(2) The slave waits for the master to send a response message. The message
	is composed of the master's name and some optional switches.
	The switch field can include the following

			-g		(set by the -G switch on the
					 master's uucico command line.
					 Indicates that communication
					 occurs over a packet switch net.)
			-xN		(set by the -x switch on the
					 master's uucico command line.
					 The number N is the debug level
					 desired.)
			-QM		(M is really a sequence number
					 for the communication.)

	Each switch is separated from the others by a 'blank' character.

(3) The slave will send one of the many responses. The meanings appear to
	be :

	RLCK

		This message implies that a 'lock' failure occurred:
		a file called LCK..mastername couldn't be created since
		one already exists. This seems to imply that the master
		is already in communication with the slave.

	RCB

		This message will be sent out if the slave requires a
		call back to the master - the slave will not accept a
		call from the master but will call the master instead.

	ROK

		This message will be returned if the sequence number that
		was sent in the message, attached to the -Q switch, from 
		the master is the same as that computed on the slave.

	RBADSEQ

		Happens if the sequence numbers do not match.

	(Notes on the sequence number - if a machine does not keep
	 sequence numbers, the value is set to 0. If no -Q switch
	 is given in the master's line, the sequence number is
	 defaulted to 0.

	 The sequence file, SQFILE, has the format

		<remotename> <number> <month>/<day>-<hour>:<min>

	 where <remotename> is the name of a master and <number>
	 is the previous sequence number. If the <number> field
	 is not present, or if it is greater than 9998, it is
	 set to 0. The <number> field is an ascii representation
	 of the number. The stuff after the <number> is the time
	 the sequence number was last changed, this information
	 doesn't seem important.)

(4) The slave sends a message that identifies all the protocols that
	it supports. It seems that BSD supports 'g' as the normal case.
	Some sites, such as Allegra, support 'e' and 'g', and a few
	sites support 'f' as well. I have no information about these
	protocols. The exact message sent might look like

		\020Pefg\0

	where efg indicates that this slave supports the e,f and g 
	protocols.

(5) The slave waits for a response from the master with the chosen
	protocol. If the master has a protocol that is in common the
	master will send the message

		\020U<proto>\0

	where <proto> is the protocol (letter) chosen. If no protocol
	is in common, the master will send the message

		\020UN\0

(6) At this point both the slave and master agree to use the designated
	protocol. The first thing that now happens is that the master
	checks for work.

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			UUCP Work Phase
			===============


Master							Slave
------							-----

(a) Master has UUCP Work

	(1) X file1 file2 	----->

					<-----		XN		(2)
							XY

	When the master wants the slave to do a 'uux' command
	it sends the X message. If the slave can't or won't
	do it, the slave will send an XN message. Otherwise
	it will send an XY message.

(b) Master wants to send a file

	(1) S file1 file2 user options  ----->

					<-----		SN2		(2)
							SN4
							SY

			<---- <data exchanged>---->			(3)


					<-----		CY		(4)
							CN5

	If the master wishes to send a file to the slave, it will
	send a S message to the slave. If the slave can or will do
	the transfer, it sends a SY message. If the slave has a
	problem creating work files, it sends a SN4 message. If
	the target file can't be created (because of priv's etc)
	it sends a SN2 message.

	The file1 argument is the source file, and file2 is the
	(almost) target filename. If file2 is a directory, then
	the target filename is composed of file2 concatenated
	with the "last" part of the file1 argument. Note, if the
	file2 argument begins with X, the request is targeted to
	UUX and not the normal send.

	The user argument indicates who, if anyone, is to be notified
	if the file has been copied. This user must be on the slave
	system.

	I am not sure what the options argument does.

	After the data has been exchanged the slave will send one of
	two messages to the master. A CY message indicates that every-
	thing is ok. The message CN5 indicates that the slave had
	some problem moving the file to it's permanent location. This
	is not the same as a problem during the exchange of data : this
	causes the slave to terminate operation.

(c) Master wishes to receive a file.

	(1) R file1 file2 user	----->

						<-----	RN2		(2)
							RY mode

	(3)		<---- <data exchanged> ---->

	(4)	CY		----->
		CN5

	If the master wishes the slave to send a file, the master sends
	a R message. If the slave has the file and can send it, the
	slave will respond with the RY message. If the slave can't find
	the file, or won't send it the RN2 message is sent. It doesn't
	appear that the 'mode' field of the RY message is used.

	The argument file1 is the file to transfer, unless it is a
	directory. In this case the file to be transferred is built
	of a concatenation of file1 with the "last" part of the file2
	argument.

	If anything goes wrong with the data transfer, it results in
	both the slave and the master terminating.

	After the data has been transferred, the master will send an
	acknowledgement to the slave. If the transfer and copy to the
	destination file has been successful, the master will send the
	CY message. Otherwise it will send the CN5 message.

(d) Master has no work, or no more work.

	(1) H			----->

				<-----				HY	(2)
								HN

	(3) HY			----->

				<----				HY	(4)

	(5) ...

	The transfer of control is initiated with the master sending
	a H message. This message tells the slave that the master has
	no work, and the slave should look for work.

	If the slave has no work it will respond with the HY message.
	This will tell the master to send an HY message, and turn off
	the selected protocol. When the HY message is received by the
	slave, it turns off the selected protocol as well. Both the
	master and slave enter the UUCP termination phase.

	If the slave does have work, it sends the HN message to the
	master. At this point, the slave becomes the master. After
	the master receives the HN message, it becomes the slave.
	The whole sequence of sending work starts over again. Note,
	the transmission of HN doesn't force the master to send any
	other H messages : it waits for stuff  from the new master.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


			UUCP Termination Sequence
			=========================

 Master								Slave
 ------								-----

 (1) \020OOOOOO\0		----->

				<-----			\020OOOOOOO\0 (2)



	At this point all conversation has completed normally.


+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			UUCP Data Transfers
			===================

	After the initial handshake the systems send messages in one
	of two styles : packet and not packet. A Packet protocol is
	just raw data transfers : there is no protocol or acknowledgements;
	this appears to assume that the lower level is a packet network
	of some type. If the style is not Packet, then extra work is
	done. I am still working on this stuff.


From wuphys!wucs!busch!we53!mgnetp!ihnp4!cbosgd!gatech!seismo!rlgvax!dennis Mon Nov 18 20:06:37 CST 1985
Article 928 of net.dcom:
Relay-Version: version B 2.10.3 4.3bsd-beta 6/6/85; site plus5.UUCP
Posting-Version: version B 2.10.2 9/18/84; site rlgvax.UUCP
Path: plus5!wuphys!wucs!busch!we53!mgnetp!ihnp4!cbosgd!gatech!seismo!rlgvax!dennis
>From: dennis@rlgvax.UUCP (Dennis Bednar)
Newsgroups: net.dcom,net.unix-wizards
Subject: uucp level g protocol
Message-ID: <841@rlgvax.UUCP>
Date: 14 Nov 85 22:33:33 GMT
Date-Received: 15 Nov 85 21:51:04 GMT
Organization: CCI Office Systems Group, Reston, VA
Lines: 101
Xref: plus5 net.dcom:928 net.unix-wizards:7758

Recently there was some requests for info on the uucp level g protocol.
I dug up some old notes that I had, and recently editted them, based
on what I remember.  Here they are:


              The level g protocol (g-protocol) is a  link  level  protocol
         analogous  to X.25 level 2.  g-protocol uses timeouts, retransmis-
         sions,  acknowledgements,  and  packet  checksums   for   reliably
         transferring  information  from  one machine to another. Data sent
         from the user program on the sending machine is passed to the user
         program  on  the receiving machine, in the same order, and without
         loss or duplication.  Each machine has a send window which  limits
         the  number  of  outstanding  DATA  packets,  and serves as a flow
         control.  Ordering  is  maintained  by  adding  PS  (packet  send)
         sequence  numbers  on  the  sending  side, and by replying with PR
         (packet receive) sequence numbers on the acknowledging side.   The
         protocol  details  are  hidden from the user program on each side.
         The program on each side first issues an  open  call.  The  sender
         transmits  data  with  protocol write calls, and the receiver gets
         data with protocol read calls.  At the end of the link connection,
         both programs normally call a protocol close routine.

	[Ed note: level g is *functionally* the same as X.25 level 2,
	but the packet types, and packet formats are drastically different.
	For opening a connection, X.25 uses the SABM and UA frames,
	level g, however, uses the INITA, INITB, and INITC frames,
	both sides must send all 3 packets and receive all 3 packets.
	For transmission of DATA, X.25 uses I-frames, level g uses
	DATAL (data long), and DATAS (data short).  For acknowledgement
	purposes, both protocols use RR (Receiver Ready).
	X.25 has 2 frames that level g doesn't have: REJ, and RNR.
	For closing a connection, X.25 uses the DISC/UA sequence,
	uucp level g simply sends 2 consecutive CLOSE packets and
	doesn't wait for any confirmation.  In fact, it sends the
	2 CLOSE packets even when there are outstanding DATA packets,
	which is discussed again later.

	Also, the g protocol is typically imlemented in user programs,
	so that the protocol interfaces to the UNIX tty driver.
	However, the code has been written with #ifdef's so that
	it can be compiled, and run in the kernel, if desired.
	One of the earlier versions of UNIX had a pkon() call
	which, apparently, told the kernel tty driver to start doing
	the level g protocol.

	]


              g-protocol does not provide certain functions.  For instance,
         it does not provide routing any more elaborate than point-to-point
         between  two machines.  Also g-protocol does not support more than
         one connection over the link.

	[ Ed note: that is level g is a point-to-point protocol, parallel
	point-to-point links between the same machines are possible,
	but the data traffic on each link would be independent. ]

              There  are  certain  other  minor  deficiencies   that   were
         discovered while experimenting with the g-protocol:

	[ Ed note: these deficiencies were brought to my attention
	when I ran a personal program that runs on one machine, and
	illustrates the protocol in live operation. It is implemented
	by running the protocol in different 2 UNIX processes
	connected by UNIX pipes.  I hope to eventually post the source
	to net.sources. ]

              -  DATA  packets  are  always  always padded so that the DATA
         field is fixed length (64 bytes  in  this  version).   This  means
         wasted space in some packets, and decreased performance.

              -  it is possible to send less data than the send window size
         and close the connection without receiving any  error  indication:
         At  the local side, two protocol writes were issued, followed by a
         protocol close. The send window happened to be  3  packets.   Each
         protocol  write call returned "numwrote" (number of bytes written)
         that was the same as "num2write" (number of bytes to be written to
         network),  indicating  success.   Each  protocol write call caused
         data to be sent to  the  network,  but  it  was  not  acknowledged
         because  the  remote task was programmed to only call the protocol
         open code, but not to call its protocol read  routine.   Thus  the
         data  was  still  buffered  in  the  local  side's driver, and un-
         delivered when the call to the protocol close was issued.

	[ Ed note:  level g allows more than one DATA packet to be
	outstanding.  That is, several DATA packets can be sent,
	without having to wait for an explicit RR acknowledment packet
	for each one. Therefore, if you call pkwrite() the first time,
	pkwrite() sends the data over the link, and returns with
	success, even though the remote protocol was not prepared to read
	the data yet.  At this point the received data will be in
	the ttydriver's input buffer, but not yet passed on to the
	receiving program.  A copy of the data will also be in the
	sending protocol's send buffer, because the sender normally
	does not discard such data until it has been acknowledged.
	The fact that the pkclose() call does not
	attempt to make sure that outstanding DATA has been acknowledged,
	is, apparently, a bug in the protocol. ]
-- 
Dennis Bednar	Computer Consoles Inc.	Reston VA	703-648-3300
{decvax,ihnp4,harpo,allegra}!seismo!rlgvax!dennis


