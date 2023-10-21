#! /bin/sh
#
# This program is called from AQgetentry to send a help message 
#
# usage: AQhelp "destination" "username"

PATH=#PATH#
export PATH
WHO="$2"; export WHO
ADDRESS="$1"; export ADDRESS
/usr/lib/sendmail -ba "$1" << xxx
Subject: How to use the #SVC-NAME#
From: #SVC-TITLE# <#SVC-MAILBOX#>
In-reply-to: message from $WHO
To: $1

This message comes to you from the archive server at decwrl,
#SVC-MAILBOX#@#SVC-HOST#. It received a message from you asking for
help.

The archive server is a mail-response program. That means that you mail it a
request, and it mails back the response.

The archive server is a very dumb program. It does not have much error
checking. If you don't send it the commands that it understands, it will just
answer "I don't understand you".

The archive server has 4 commands. Each command must be the first word on a
line. The archive server reads your entire message before it does anything,
so you can have several different commands in a single message. The archive
server treats the "Subject:" header line just like any other line of the
message. You can use any combination of upper and lower case letters in the
commands.

The archives are organized into a series of directories and subdirectories.
Each directory has an index, and each subdirectory has an index. The
top-level index gives you an overview of what is in the subdirectories, and
the index for each subdirectory tells you what is in it.

---------------------------------------------------------------------------
If you are bored with reading documentation and just want to try something,
then send the server a message containing the line
	send index recipes
When you get the index back, it will give you the names of all of the recipes
in the archive; send the server another message asking it to send you the
recipes that you want:
	send recipes corn-stew potato-soup cheese-bread
etc. If you want the mod.recipes software and macros and such, say this:
	send program full.shar
If you are using a mailer that understands "@" notation, send to
#SVC-MAILBOX#@#SVC-HOST#. If your mailer deals in "!" notation, try
sending to {someplace}!#UUCP-HOST#!#SVC-MAILBOX#, e.g.
ihnp4!#UUCP-HOST#!#SVC-MAILBOX#. For other mailers, you're on your own.
---------------------------------------------------------------------------

Here is some more documentation. The server has 4 commands:


"help" command: The command "help" or "send help" causes the server to
	send you the help file. You already know this, of course, because
	you are reading the help file. No other commands are honored in a
	message that asks for help (the server figures that you had better
	read the help message before you do anything else).

"index" command: if your message contains a line whose first word is "index",
	then the server will send you the top-level index of the contents of
	the archive. If there are other words on that line that match
	the name of subdirectories, then the indexes for those subdirectories
	are sent instead of the top-level index. For example, you can say
		index
	or
		index programs 
	or
		index appetizers

	You can then send back another message to the archive server,
	using a "send" command (see below) to ask it to send you the files
	whose name you learned from that list.

	(Footnote: "index programs" and "send index programs" mean the
	same thing: you can use the "send" command instead of the "index"
	command, if you want, for getting an index.

	If your message has an "index" or a "send index" command, then all
	other "send" commands will be ignored. This means that you cannot
	get an index and data in the same request. This is so that index
	requests can be given high priority.)

"send" command: if your message contains a line whose first word is "send",
	then the archive server will send you the item(s) named on the
	rest of the line. To name an item, you give its directory and its
	name. For example
		send recipe african-stew
	or
		send program rckeep
	Once you have named a category, you can put as many names as you
	like on the rest of the line; they will all be taken from that
	category. For example:
		send recipe choc-chip-1 choc-chip-2 choc-chip-3

	Each "send" command can reference only one directory. If you 
	would like to get one recipe and one program, you must use two
	"send" commands, one beginning "send recipe" and the other
	beginning "send program".

	You may put as many "send" commands as you like into one message
	to the server, but the more you ask for, the longer it will take
	to receive. See "FAIRNESS", below, for an explanation. Actually,
	it's not strictly true that you can put as many "send" commands 
	as you want into one message. If the server must use uucp mail
	to send your files, then it cannot send more than 100K bytes
	in one message. If you ask for more than it can send, then it
	will send as much as it can and ignore the rest.

"path" command: The "path" command exists to help in case you do not
	get responses from the server when you mail to it.

	Sometimes the server is unable to return mail over the incoming path.
	There are dozens of reasons why this might happen, and if you are a
	true wizard, you already know what those reasons are. If you are an
	apprentice wizard, you might not know all the reasons but you might
	know a way to circumvent them.

	If you put in a "path" command, then everything that the server
	mails to you will be mailed to that address, rather than to the
	return address on your mail. For example, if you say
	    path pyramid!rutgers!zakkaroo!jj
	then all mail sent by the server will be sent to that address.
	Note: decwrl does NOT have a uucp link to seismo. A command like
	    path seismo!someplace!name
	will guarantee that you do not receive the response. We do have
	a link to seismo.css.gov (and all other Internet sites), permitting
	    path seismo.css.gov!someplace!name

	If you would like the server to determine a uucp path for you,
	using the most recent pathalias data, then put in a "path" command
	with yourname@site.uucp, e.g.:
	    path person@place.uucp
	As you probably know, the pathalias data is sometimes wrong, but it
	is often right. 

EXAMPLES:

1) Find out the list of recipes that are in the archive. Send this message:
	To: #SVC-MAILBOX#@#SVC-HOST#
	Subject: hi there

	send index

2) Get the chocolate-chip-cookie recipes from the archive (you have learned
   their file names from the list that was sent to you in step 1).
	To: #SVC-MAILBOX#@#SVC-HOST#
	Subject: send recipe truffles

	send recipe choc-chip-1 choc-chip-2 choc-chip-3 choc-chip-4
	send recipe choc-chip-5 choc-chip-6 choc-chip-7

3) Get the eggnog recipes, and send them over the best path to my site:
	To: seismo.css.gov!#UUCP-HOST#!#SVC-MAILBOX#

	path myname@site.uucp
	send eggnog1 eggnog2 eggnog3

NOTES:

The archive server acknowledges every request by return mail. If you don't
get a message back in a day or two (depending on how close you are to #UUCP-HOST#
on the network) you should assume that something is going wrong, and perhaps
try a "path" command. If you aren't getting anywhere and you don't know a
wizard to help you, try putting
	path myname@site.uucp
in your message, where "myname" is your mailbox name and "site" is the uucp
name of your machine. 

The delays in sending out large items from the archives are intentional, to
make it difficult to get copies of everything in the archives. If you are new
to the network and would like to get all back issues of everything, you
should post a request to a regional newsgroup asking whether someone who is
geographically near you can provide them.

Don't send mail with long lines. If you want to ask for 20 recipes in one
request, you don't need to put all 20 of them in one "send" command. The
archive server is quite able to handle long lines, but before your mail
message is received by the archive server it might pass through relay
computers that will choke on long lines.

The archive server does not respond to requests from users named "root",
"system", "daemon", or "mailer". This is to prevent mail loops. If your name
is "Bruce Root" or "Joe Daemon", and you can document this, I will happily
rewrite the server to remove this restriction. Yes, I know about Norman
Mailer and Waverley Root. Norman doesn't use netmail and Waverley is dead.


FAIRNESS:

The archive server contains many safeguards to ensure that it is not
monopolized by people asking for large amounts of data. The mailer is set up
so that it will send no more than a fixed amount of data each day. If the
work queue contains more requests than the day's quota, then the unsent files
will not be processed until the next day. Whenever the mailer is run to send
its day's quota, it sends the requests out shortest-first. 

If you have a request waiting in the work queue and you send in another
request, the new request is added to the old one (thereby increasing its
size) rather than being filed anew. This prevents you from being able to
send in a large number of small requests as a way of beating the system.

The reason for all of these quotas and limitations is that the delivery 
resources are finite, and there are many tens of thousands of people who
would like to make use of the archive. 
xxx
echo "`date`" help $ADDRESS >> #REQQ#/out.log
