.TL
Mail Handling at Gatech
.br
Revision III
.AU
Gene Spafford
.AI
School of Information and Computer Science
Georgia Institute of Technology
27 October 1985
.PP
Site "gatech" is running a "smart" version of sendmail.  I have 
hacked at the sendmail configuration files extensively, and although
they are not yet doing 100% of what I want, they seem to work pretty well
and handle our many (sometimes unusual) mail needs.
What follows are brief descriptions of what happens to various bits of
mail passing through our site. 
.PP
There have been some changes since the last time I circulated this
document.  Most of the changes have been inspired (?) by the changes
to sendmail done at Rutgers which allow "sendmail" to distinguish
the source of incoming mail with mixed syntax addresses (e.g., a!b@c),
and which rewrite the names of Internet hosts into the preferred form
(as given in the /etc/hosts table, derived from NIC data).
.NH 1
Why
.PP
Site "gatech" is directly on the uucp network
(with over 75 contacts and acting as a de facto name server for a
"southeast US domain"), the CSNet, and we serve as gateway for
our local networks.  We also have network traffic with some other
major networks, and we might possibly get BITNET and ARPA access in
the not-too-distant future, from "gatech" or some other campus machines.
We'd like to have as complete and robust
a mailing environment as possible.  At the same time, we'd like
to minimize our current phone bills as they are related to UUCP mail 
traffic.
.NH 1
Routing
.NH 2
Known Domains
.PP
Currently, there is considerable effort going on to identify and
establish domains for mailing.  Some of these domains are already
established, if only in a de facto manner.  Our sendmail
currently recognizes the following well-known domains: ARPA, CSNET,
GOV, EDU, COM, MIL, ORG, NET, UK,
UUCP, BITNET, DEC, and MAILNET.  The following 
are also recognized when used as top-level domains:
GTNET (local to Georgia Tech), OZ (the Australian network),
TEK (recognized internal to Tektronix), and SDC (with sdcrdcf as the gateway).
.PP
There are a number of other "domains" that are recognized when used
in a second-level position within a uucp address.  That is, we
recognize person@site.ATT.UUCP as something that should be
directed to cbosgd via uucp for further resolution.  This recognition
is done by building psuedo-sites into the uucp routing database
based upon the data distributed by the uucp map project.  
Among domains recognized like this are NCAL, SCAL, ATL, ATT, and
so on; the list may change based on updates to the map.
(As an aside, at the time of this writing, "gatech" is one of
the regional repositories of the current map, and we get automatic
updates whenever the "real" map gets changed.)
.PP
In the following descriptions, any of the above can be used in
place of a "DOMAIN" specifier.
.NH 2
user@host[.DOMAIN] -and- user%host[.DOMAIN]
.PP
If mail comes in addressed specifically to one of the known domains,
it is routed as described in the next section without any further
changes.  If the domain is not given, an attempt is made to derive
the domain based on available lists of host names and aliases,
and then routed as described in the next section.  If no host/domain
match can be found, the mail is returned with an error stating this.
.NH 3
Domain Derivation
.PP
Host derivation is attempted in the following order:  First, the
host is checked to see if it is in the GTNET domain.  Next, the
host is checked to see if it is a host one hop away via UUCP.  Next,
the host is checked against all CSNET sites.  Then it is checked
against all Arpa Internet sites. Next, it is checked against the list of
known MAILNET hosts. Then it is checked against the list of all (other) known
UUCP sites. Then it is checked against the list of
known BITNET hosts.  Finally, it is checked against the list
of known DEC E-net sites.  
.PP
This kind of checking is not 100% accurate because our lists are not
always up-to-date.  In particular, the Arpa list is updated infrequently
due to the fact that we aren't actually on the Arpanet, and there is no
list of DEC net sites available outside of DEC (we make due with
gleaning names from posted news articles and exchanges with other sites
interested in compiling such a list).
.NH 3
Collisions
.PP
If the same hostname exists in more than one domain, the first match
found will be the one used.  Qualification of the address with an
explicit domain specifier will ensure that the mail goes to the
correct host (when routed through gatech).  That is, the domain
is considered to be the specifer for routing and if one is not
explicitly provided (or implicitly, as in the case of "!" notation),
then an attempt to made to guess a domain.
.NH 2
host1!host2!host3...!hostn!user
.PP
Starting with "hostn" and working backwards to "host1" our mailer
will attempt to find a host listed in our master UUCP path database.
This database is generated using pathalias at least weekly based on the latest
version of the uucp maps.  If a match is found at "hostk", then the 
address is rewritten to be "<path to hostk>!hostk!...hostn!user"
and then mailed via UUCP.  Such addresses are 
.B never
routed over
any other network, unless "hostn" is recognized to be a GTNET
host, in which case our internal transport mechanism is invoked, or
unless a domain specifier is present in "host1" (see next section).
.PP
There is 
.B no
way at present to force a path on UUCP mail through "gatech".
This is perhaps a "not very good thing" but I can't come up with a good
way to work in explicit paths.  The map data is generally very good
and I have observed very, very few failures since we first started
doing this rewriting about 6 months ago.  If this presents a
major problem for someone, let me know and I'll see what I can work out.
.NH 2
host.DOMAIN!person
.PP
This is a special case and mail to an address such as this is handled
in a reasonably sane manner.  That is, the mail is sent to user@host.DOMAIN
by the proper transmission medium (probably 
.B not
UUCP).  This is the same syntax that gateways like 
.I seismo 
and 
.I ucbvax
use to leave the UUCP mail "domain."  Thus, mail
to ...gatech!seismo.css.gov!rick will, in fact, get to Rick Adams at
seismo via UUCP to gatech, PMDF to CSNet-Relay, and Arpa to seismo.
.PP
For this notational form to work, the 
.B first
component of the address seen at gatech must be qualified with one of the recognized
top-level domains (EDU, ARPA, CSNET, BITNET, etc.).  
.B No
attempt is made to validate the hostname or the remainder of the
address (the part after the first "!") since we don't know about 
second level domains or syntaxes in most cases (nor should be expected
to be so aware -- that's the whole idea behind domain addressing).
.NH 2
host1!host2!host3...!hostn!user@site[.DOMAIN]
.br
host1!host2!host3...!hostn!user%site[.DOMAIN]
.PP
This one diverges somewhat from the standard (RFC822 et.al.).  
The way these addresses get treated is based on the way the mail
gets into our "sendmail."  If the message originates on any of the
local (GTNET) machines, or if it comes in via PMDF from CSNet, then
the mail is routed to "site" for eventual delivery to host1!...hostn!user.
Mail coming in via a UUCP link with a mixed-mode address like this will
have the mail routed via uucp to hostn for eventual delivery to
user@site.  Thus, if one of our neighbors, such as someone at akgua,
were to send mail to us addressed as seismo!person@ucbvax.ARPA, we would
send the mail to seismo via uucp and present it to their "rmail"
program as "rmail person@ucbvax.ARPA".
.PP
On the other hand, should someone on gitpyr send mail addressed
as seismo!person@ucbvax.ARPA, it would arrive at Gatech via SMTP and
then be sent to CSNet-relay via PMDF for delivery to site "ucbvax" with
a request to be delivered to "seismo!person" relative to that site.
In most cases, depending on the sites involved,
this kind of treatment would result in the mail failing.  The sendmail
configurations I have created for all the local GTNET sites are such
that it should not be required to specify such an address.  Simply
mailing to person@site should see the correct address and network
transport mechanism chosen.  The Usenet "news" programs on most of these
sites have been built to use the Internet-style of address when
mailing replies, so there should be few cases of users even seeing mixed
mode addresses presented to them (mail passing through any of the mailers
gets rewritten to show a consistent format).
.NH 2
user%site1%site2%site3
.PP
Addresses of this format get turned into user%site1%site2@site3,
and an appropriate routing is provided to "site3," if known.
.NH 2
Other network characters
.PP
The ":" delimiter gets turned into "!" symbols in any address
presented to our sendmail.
The "^" delimiter gets turned into "!" also.  Addresses of the
form "site=user" get turned into "user@site.BITNET" by convention.
.NH 1
Errors
.PP
I have tried to trap all possible errors and generate return mail
with meaningful messages.  
If you get errors you don't know how to interpret, please contact me.
.NH 1
Source
.PP
I posted an ancestor of my current sendmail files to "mod.sources" a
few months ago.  If these latest versions appear stable, I will post
them to the same place.  If you'd like a copy right away, let me know.
This includes the sendmail files for all the local GTNET machines, and
the source for my "uumail" program which sits between sendmail and
uux.
