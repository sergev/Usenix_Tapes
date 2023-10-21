#! /bin/sh
#
# This program is called from AQgetentry to send a rejection message 
#
# usage: AQreject "destination" "username"

PATH=#PATH#
export PATH
WHO="$2"; export WHO
#MAILER# "$1" << xxx
Subject: I don't understand your message
From: #SVC-TITLE# <#SVC-MAILBOX#>
In-reply-to: message from $WHO
To: $1

This message comes to you from the archive server at #UUCP-HOST#,
#SVC-MAILBOX#@#SVC-HOST#. It received a message from you that it
was not able to understand. The server is just a dumb program, and it is not
very creative about figuring out what you want.

If you want to learn how to use the archive server, then send it a one-word
message whose contents is the word "help". Remember that the mail address of
the archive server is #SVC-MAILBOX#@#SVC-HOST#, also reachable as
#UUCP-NBRS##UUCP-NAME#!#SVC-MAILBOX#

If you want to communicate with a person and not a program, then send mail to
	#MANAGER#@#SVC-HOST#
or
	#UUCP-NBRS##UUCP-NAME#!#MANAGER#

Brian Reid
DEC Western Research Laboratory, Palo Alto, California, USA
xxx
echo "`date`" reject $1 $WHO >> #REQQ#/out.log
