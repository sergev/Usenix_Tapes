
	Submitted to Int. J. Biomed. Computing Feb. 2, 1983.
	(Please do not violate the implied copyright!)


RIPS: a UNIXT-based  Reference Information Program for Scientists



            Stephen D. Klyce and Andrew J. Rozsa



From the Lions  Eye  Research  Laboratories,  Department  of

Ophthalmology, Louisiana State University School of Medicine

New Orleans, Louisiana 70112 U.S.A.


























Address reprint requests and correspondence to  Dr.  Stephen

D.  Klyce,  LSU  Eye  Center,  136  South  Roman Street, New

Orleans, LA 70112 U.S.A.


UNIXT is a registered trademark of Bell Laboratories.















Klyce, et al                -2-                         RIPS



                          SUMMARY


     A set  of  programs  is  described  which  implement  a

personal  reference  management  and  information  retrieval

system on a UNIXT-based minicomputer.  The  system  operates

in  a  multiuser  configuration with a host of user-friendly

utilities that  assist  entry  of  reference  material,  its

retrieval,  and  formatted printing for associated tasks.  A

search command language was developed without restriction in

keyword   vocabulary,   number  of  keywords,  or  level  of

parenthetical expression nesting.   The  system  is  readily

transported,  and  by  design  is applicable to any academic

specialty.

                        INTRODUCTION


     The explosive proliferation of scientific publications,

even  in  relatively  specialized  research  areas, tends to

overwhelm  the  academic  author.   Fortunately,  the  truly

mammoth   tasks  associated  with  comprehensive  literature

searches  can  be  handled  (at  some   expense)   at   most

institutions  by remote access to commercial data bases.  In

fact, these services and a well-funded institutional library

system  are  all  the young entrepreneur engaged in academic

research may require.  However, in time one's interests  and

personal  reprint  collections procreate unmercifully to the

point where  local  data  base  management  and  information

retrieval would provide needed relief.

     With the decentralization of computer  resources  (made

possible  by increasingly affordable and powerful micro- and







Klyce, et al                -3-                         RIPS



mini-computers) comes the age of personal  and  departmental

computers  and  along  with it a broad base of user-friendly

software including a host of data  base  management  systems

applied  to  the maintenance of personal references (Sterner

et al., 1976;  Clark et al., 1979; Bertrand et al., 1980).

     In this paper we describe a set of programs that can be

used  to  accomplish  most  of  the  tasks  associated  with

maintaining, searching, and printing  a  personal  reference

file.    The   programs   are  written  in  the  portable  C

Programming  Language  (Kernighan  et  al.,  1978)  and  for

compactness,  calls  are  made  to  UNIXT  operating  system

utilities for tasks such as error  correction  and  sorting.

It  is  noted  that  while  UNIXT  provides  some  reference

handling utilities, it was our  intention  to  make  as  few

demands  in  terms  of computer knowledge as possible on the

prospective   users   (academicians   and   their    support

personnel).   In  keeping with UNIXT tradition, however, the

programs provide  a  true  multiuser  environment  in  which

simultaneous  users  can not only share portions of the main

program code, but also have access privileges to all current

reference  lists.   The  programs are self-instructional and

can be used by  computer  novices  with  minimal  additional

assistance.   Importantly,  the  main  program requires less

than 64 Kbytes of memory so that it runs easily on minimally

configured  UNIXT  systems  and where there are restrictions

(e.g. number of characters per entry, number of keywords  in

a  search,  etc.),  these limits are liberal, adjustable and

easily tuned to meet local needs.







Klyce, et al                -4-                         RIPS



                      SYSTEM OVERVIEW

     RIPS is an acronym for "Reference  Information  Program

for  Scientists,"  where  scientist  is  used in the broader

sense meaning any investigational person.  Written  locally,

RIPS  is  currently  running  on  the LSU/Lions Eye Research

Facility under  a  variant  of  UNIXT  Version  6  with  the

UNIX/PWBT  shell program.  The relevant equipment includes a

Digital Equipment Corp. PDP11/34A processor with 256  Kbytes

of  random  access  memory,  cache memory, 80 Mbytes of hard

disk, a line printer, a letter quality printer, and multiple

inexpensive CRT terminals.

     The RIPS data base resides  in  one  directory  of  the

UNIXT   tree  structured  hierarchical  file  system.   This

directory is  in  turn  subdivided  by  function  into  five

subdirectories:

     1) files - the complete sets of user reference lists.

     2) new - all recent additions to user  lists  for  file

card updating.

     3) kwl - keyword lists associated with user files.

     4) bkup - on-line copies of user files.

     5) rr - master file of unprinted reprint request data.

     6) doc - text files of on-line help.

     The data base is virtually open-ended,  depending  upon

the  availablity  of  mass  storage (300 Kbytes of disk/1000

references on the average; half of  this  if  the  automatic

disk backup feature is disabled).

     The major attributes of the RIPS system include:

     1.   Editing  facilities   for   the   entry,







Klyce, et al                -5-                         RIPS



     correction  and  checking  of  references  to

     personal files.

     2.  Automatic generation of canonical keyword

     lists.

     3.  A  search  and  retrieval  facility  with

     heuristic syntax, keyword root searching with

     wild  card   character,   unlimited   keyword

     vocabulary, time period specification, nested

     parenthetical expression parsing, single line

     complex  command  statement  capability,  and

     optional use of an external command file.

     4.  A  print  facility  with  output  formats

     suitable   for   a  variety  of  devices  and

     specific tasks (printing bibliographies, file

     cards,  etc.) and with a reprint request post

     card generator.

     5.    Menu    driven,    practically    self-

     instructional interactive programming.


                    PROGRAM DESCRIPTION


     A flow chart of the RIPS system components is presented

in  Figure  1.   Entry  to  RIPS is made from the UNIX shell

program  by  simply  naming  the  program.    Current   news

concerning  the  RIPS  system  (if  any) is displayed on the

user's  terminal  and  the  files  currently  available  are

listed.   On-line assistance is available from this point in

the form of a tutorial which describes each of  the  options

at various levels in the system.  More specific instructions







Klyce, et al                -6-                         RIPS



are available to  the  user  on  entry  to  individual  task

levels.   The  session can be ended at this time or the user

can enter the  name  of  a  file  in  which  to  work.   For

unrecognized  files, the user is asked to confirm that a new

name is to be created which limits the growth of  misspelled

and erroneous entries in the data base directory.

                        Central menu

     The central routine is at the hub of  the  system;  the

user  is generally returned to this level from which most of

the options are  directly  specifiable.   A  printout  of  a

session  to  this  point  is  illustrated  below  with  user

responses italicized:

                        * * * R I P S * * *

             Reference Information Program for Scientists

Current files:

ajr_rr          capsai          cec             cornea          diabetes
disso           disso2          dlt_thesis      elt             infosci
master          model           psychophys      reflex          rwb
sdk             sdk_rr          yso

Enter file name, help, or quit =? cec
File size (# refs  # words) =    14     244 /usr/lib/rips/files/cec
If this too small a number, exit rips and contact the management!


Enter option -

        e - editor
        r - retrieval
        g - regenerate keyword list
        d - change to new file
        p - print cards, refs, keywords, or request reprints
        w - temporary exit to shell
        q - exit to shell

=? q

One can end the session from this point  either  permanently

(exit  to  the  UNIXT  shell)  as  in  the  above example or







Klyce, et al                -7-                         RIPS



temporarily by the invocation of  a  new  shell  (wait).   A

change  to  a  new  file  is accomplished by a return to the

higher RIPS level.   The  remaining  options  are  described

individually below.

     Keyword list generation.  At the user's option, a  list

of  keywords can be generated by RIPS.  This is accomplished

by a routine which  is  forked  off  from  within  RIPS  and

returns  the  user to the central menu while the task is run

concurrently in the  background.   The  keyword  list  is  a

composite  of  all title words in the current file, plus all

the keywords the user has  associated  with  the  references

(see  edit  append  below),  and  minus a dictionary of RIPS

unkeywords.   The  dictionary  of  unkeywords  is  generally

maintained  by the more computer knowledgeable consisting of

simply  adding  user  submitted  files  of  unwanted   words

(conjunctions,  prepositions,  and  the  like) to the master

unkeyword file.  Under UNIXT the task could be  accomplished

with  a  one  line  shell  command:  "sort  -d {master_file}

{user_file1} ...  {user_filen}  |  uniq  >  {temp_file};  mv

{temp_file}  {master_file}",  which  translates  to: sort in

dictionary order the  following  list  of  files,  pass  the

result  to  "uniq"  (which reduces multiple occurrences to a

single  occurrence),  place  the  subsequent  result  in   a

temporary  file,  and  replace  the old master list with the

update.  One of the unplanned uses of this routine has  been

the user detection of spelling errors.

     The RIPS editing facility.  When a request to  use  the

editor is made, mandatory checks on the ownership and status







Klyce, et al                -8-                         RIPS



of the current file are made.   While  all  files  are  made

accessible  to all users for information retrieval and other

non-destructive tasks, only the owner of a file  is  granted

permission to add to or make corrections to a file.  Failing

this test, the user is told who owns the current file and is

returned  to the central menu.  Otherwise RIPS checks to see

whether the file is currently in use (either by another user

with  owner  permission  or  by  a  background  RIPS-spawned

process).  Failing this a busy message is  printed  and  the

user  is  returned  again  to the central menu.  If the user

passes both these tests, on-line disk backup of the file  is

performed to guard against data loss in case of fatal errors

while in the editor.  It is noted that such a  situation  is

apt  to  occur only in the correction editor mode which uses

the original UNIXT ED(I) editor for compactness.

     Within the RIPS editor, the user has the options to end

the  session, change to a new file, append new references to

the file, make corrections to the file,  or  run  a  routine

that  checks  for and reports errors in the structure of the

file.   Files  consist  of  the  list  of  references,   one

reference per line where the length of the line may be up to

1024 characters long.  Each line is composed of 7  plus  the

number of authors plus the number of associated keywords (if

any) variable length fields.  An  example  of  the  specific

format (with line feeds inserted for printing here) is:

      2|Bell CLM|Jones KP|1980|The development of a highly
     interactive searching technique for MORPHS-minicomputer
     operated retrieval (partially heuristic) system|Information
     Processing and Management|16|37-47| 1|information science|








Klyce, et al                -9-                         RIPS



The number of authors (2) and the number of keywords (1)  in

this  example  are  inserted by RIPS and are used throughout

for processing the file.   Note  the  absence  of  a  unique

reference  number; the reference is identified and retrieved

soley on the basis of information content.

     Appending new references to a file is carried out in an

heuristic fashion.  The user is prompted first for reference

type which can  be  a  journal  article,  a  chapter,  or  a

complete  book.   Alternately, the user can quit the session

at this point whereupon a return to the central menu is made

and  the  file  is  sent  off for processing, which includes

sorting into dictionary order and removal of duplications if

any.   Entering  a valid reference type produces a query for

authors until a blank line  (carriage  return)  is  entered.

Then  queries  are  sent for the reference publication date,

the title, the name of the journal, the volume  number,  the

pagination,  and finally, the keywords.  After the reference

is entered, it is formatted  and  printed  on  the  terminal

giving the user the option to make corrections before adding

it to the file.  At the end  of  the  appending  session,  a

background  process  is spawned which sorts the current file

into dictionary order  and  removes  duplications  (if  any)

while the user is returned to the central menu.

     A posteriori corrections to the file are made with  the

UNIXT ED(I) editor as noted above.  This is the only routine

in the RIPS system requiring some degree  of  facility  with

the  usual  UNIXT  utility  programs.   This  feature can be

upgraded by one of the screen-oriented word processing  type







Klyce, et al                -10-                        RIPS



editors  which  are  starting  to  appear for UNIXT systems.

Following a correction session, the user is returned to  the

central menu.

     The  RIPS   search   and   retrieval   facility.    For

compactness  and portability RIPS does not employ any of the

associative  methods  for  information  retrieval.   With  a

linear  searching  technique  extra  disk  files  containing

dictionaries, associative lists, and pointers do not have to

be  added or maintained (often at considerable expense).  In

addition, since RIPS was designed  for  the  maintenance  of

personal  reference files, the increase in time used by such

a searching method is minimally important for files of a few

thousand  references  as long as a medium fast disk drive is

available (average access time in the range of 20-40  mSec).

For  higher  volumes,  incorporation  of an associative file

processer (Bird, 1982) is an  alternative  to  the  task  of

creating inverted file indices which, for efficiency, really

must be custom-tailored to  the  specific  application.   By

staying  with the linear search technique, we preserve total

generality in the search method and serve the needs of users

from vastly different fields.  This makes the system readily

portable and eliminates  the  administrative  tasks  usually

associated  with  the creation and maintenance of index term

dictionaries.

     For a linear  searching  method  to  be  at  all  times

efficient,  a  command  language should be developed so that

the entire strategy can be expressed  a  priori  allowing  a

single  pass  through  the  file.  To assist the user in the







Klyce, et al                -11-                        RIPS



formulation of searches, a method of  command  line  parsing

was  developed  with the following syntax excerpted from the

on-line retrieval help file:


     1.  Conjunctions.

             a.  & or ampersand.
                     Meaning - AND or BY (if A & B exist)
             b.  | or pipe.
                     Meaning -  OR (if A | B exist)

     2.  Conjunction suffix.

             c.  ! or exclamation point.
                     Meaning - NOT (if A &! B)

     3. Publication date prepositions.

             d.  < or less than.
                     Meaning - BEFORE (<1975 if before 1975)
             e.  > or greater than.
                     Meaning - AFTER (>1975 if after 1975)
             f.  = or equal to.
                     Meaning - IN (if published IN 1975)

     4. Alternate conjunction and preposition sets:

             &       AND (BY)
             &!, -   AND NOT
             |       OR
             |!, X   OR NOT
             <       BEFORE
             >       AFTER
             =       IN

     5.  Keywords.

             In theory any number of keywords can be specified.
     A context search is applied to the reference so that root
     words can be found despite prefixes and suffixes.  The
     character '*' can be used to substitute for any single
     character in a word.

     6.  Parentheses.

             Carefully placed, nested or not, parentheses can be
     used to implement a true complex boolean expression.

     7.  Reading command line from file.

             Enclosing a command file name in quotes in response to
     a search command line query will permit execution of a search







Klyce, et al                -12-                        RIPS



     based on the content of the named file.


The search operators (AND, OR, NOT, BY, BEFORE,  AFTER,  and

IN)  can  be  entered  by  the user in this long form, or as

their abbreviations listed above.  Note that BY is really  a

pseudo-operator in that while it implies a search restricted

to author name, this is not actually implemented at present.

Rather,  BY  is simply translated to AND by the command line

parser.  In keeping with the design philosophy, it is  noted

that an exclusive or operator can be implemented the command

(a AND NOT b) OR (b AND NOT a) or (a OR b) AND NOT (a AND b)

according to preference.

     The command line length is practically  set  (by  local

need)  to a maximum length of 1024 characters, the number of

keywords  to  100,  and  the  number  of  open  and   closed

parentheses to 20, but these limits are adjustable upward or

downward according to need and  available  computer  memory.

In  a  search, all the reference fields are searched so that

keywords can  be  matched  to  author  names,  title  words,

journal  names,  or  associated  keywords.   When  reference

materials are added while in edit append (above),  the  user

is  informed  that adding associated keywords that appear in

the title of the article is redundant.  Search keywords  are

found  in  references purely by a pattern matching algorithm

which returns a "hit" when the keyword pattern exists in any

portion  of  a  reference word.  This root search method can

occasionally produce false hits (finds "skin" in asking, for

example),  but  the occasional false hit was found tolerable








Klyce, et al                -13-                        RIPS



in comparison to alternate schemes of more specific types of

root  word  searching.   The  mechanics of a RIPS search are

available in the form of a short on-line tutorial  in  which

the  syntax  is  explained  and  a  few examples offered.  A

search command line might read:


 (cornea OR skin) AND ((nerv OR neur) AND pain) AND NOT beuerman


This will retrieve all articles dealing with nerves in  pain

work  for  cornea and skin except those written by Beuerman.

Using the alternate conjunction set, the above example would

be written:


   (cornea|skin)&((nerv|neur)&pain)-beuerman


An example which includes time period specifications:

   (corne OR skin) AND (pain OR itch OR tickle) BY rozsa

   AND AFTER 1965 AND BEFORE 1975


or alternatively:

   (corne|skin)&(pain|itch|tickle)&rozsa&>1965&<1975


     As an option for more advanced users, a  file  name  in

quotes  can  be entered at this point; the subsequent search

will be based on the contents of the named file.   This  has

been  found extremely useful for the automatic generation of

reference lists for manuscripts.   Such  command  files  are

easily  generated from manuscript texts using standard UNIXT

utilities.  This option also accelerates the development  of

complex search strategies.

     The response time for the  method  using  our  hardware







Klyce, et al                -14-                        RIPS



(described  above)  in  an  unloaded  situation was found to

average  26+9*(number  of   keywords)   Sec   per   thousand

references  in  a file.  Hence, for the more mature personal

files on our system of 2000 references, a search with  three

keywords  is  completed  in under two minutes of clock time,

which we find tolerable.

     The results of searches are optionally printed  on  the

user's  terminal,  and  stored  under  a  unique name in the

user's main directory for subsequent use or printing.

     The RIPS  printing  facility.   Most  of  the  printing

options available are accessed through the print routine and

each is documented as to source,  date,  etc.   All  options

requested  here  return the user to the central menu.  These

include the formatting and  printing  of  the  file  or  its

keyword  list  on  the  terminal,  line  printer,  or letter

quality printer.  Since a keyword list is generated only  on

command,  a  request  to view or print it is followed with a

comparison of the modification times of the  reference  list

and the keyword list.  A message is sent if the keyword list

needs updating.  At the user's option during a  printout  of

the reference list, the printout can include the keywords or

not as desired.  The printed  references  are  numbered  and

printed  alphabetically  by  the  first  author's  last name

according  to  the  style  requested  by  the   Journal   of

Physiology   (London).    This  format  style  was  selected

primarily  because  this  highly  cited   journal   requires

complete  reference information for its bibliographies.  The

reference format can be  altered  to  suit  local  needs  by







Klyce, et al                -15-                        RIPS



personnel   with  some  programming  experience.   A  sample

listing is reproduced below:


    1.  Abelson PH  (1982).  Essential federal information services.
          Science 216:937.

    2.  Bates M  (1981).  Search techniques.  In, Annual review of
          information science and technology.  Ed. by Williams ME.
          Knowledge Industry Pubs. for ASIS, White Plains,
          NY:pp.139-170.

    3.  Bell CLM  & Jones KP  (1978).  A minicomputer retrieval
          system with automatic root finding and roling facilities.
          Program (England) 10(1):14-27.

    4.  Bell CLM  & Jones KP  (1980).  The development of a highly
          interactive searching technique for MORPHS-minicomputer
          operated retrieval (partially heuristic) system.
          Information Processing and Management 16(1):37-47.

    5.  Benenfeld AR  & Kazlauskas EJ  (1980).  Communicating
          information: Proceedings of the 43rd ASIS Annual Meeting.
          Knowledge Industry Publishers for ASIS,  White Plains, NY:
          345 pp.

    6.  Bertrand D  & Bader CR  (1980).  Storage and retrieval of
          bibliographic references using a microprocessor system.
          Int. J. Bio-Med. Comput. 11:285-293.

        .
        .

   12.  Grams RR  (1979).  Medical information systems.  Humana
          Press,  Clifton, NJ:430 pp.

        .
        .

   15.  Johns KA,  Stuart GM  & Wilson DW  (1980).  An effective and
          reliable computerized system for storage and retrieval of
          references to scientific publications.  Int. J. Bio-Med.
          Comput. 11:265-284.

        .
        .
        .

     The print  utility  can  also  be  used  to  print  the

references  in  a  similar  format  with  appropriate margin

changes and forms length on continuous form file cards.








Klyce, et al                -16-                        RIPS



     The final  print  function  assists  the  user  in  the

preparation of reprint request post cards.  In this utility,

the user is prompted as before with the edit append  routine

(in  fact,  it  is the same routine entered from a different

point  in  RIPS).   As  entered  from  the  reprint  request

utility,  references  are added both to the current file and

in a condensed formatted form in a master  file  printed  by

batch process at appropriate intervals.  After the requested

references  are  entered,  the  user  is  prompted  for  the

addresses  of  the  first  author  of each article, and this

information along with the identity of the user  are  stored

in   the   master  reprint  request  file.   RIPS  currently

recognizes about 20 foreign countries and prints  the  cards

in  English  (the  default language), German, Dutch, French,

Italian, and Spanish as a courtesy.

     While the printing of reference reprint requests is not

normally   a   task  associated  with  reference  management

systems, using this facility has  been  found  to  be  quite

economical  in  terms  of clerical time spent both preparing

requests and subsequently entering the article  in  a  file;

both these tasks can be done simultaneously.

             Data security and error reporting

     While the UNIXT system protection schemes are generally

adequate  to  protect user files from inadvertent alteration

by others, RIPS goes one step beyond in protecting the  user

against  self-made errors.  Files are further safeguarded by

automatic  disk  backup  (by  RIPS)  as  well  as  regularly

scheduled  incremental  file  dumps  to  removable  magnetic







Klyce, et al                -17-                        RIPS



storage media (by the system manager).  Further, since  RIPS

spawns  concurrent  file processing tasks at several points,

several locks were devised which  deliver  busy  signals  to

users  rather  than  allow clashes that could result in data

loss.

     An error routine was devised to handle two  classes  of

error.   Those  which  can  be  resolved within RIPS by user

action (waiting for a file to become not busy, correcting an

inappropriate  response  to  a  query,  etc.) are classed as

nonfatal and result in the delivery of a clear statement  of

the  problem  with  a  return to a point in the program that

permits resolution of the error.  The second class of errors

is  termed  fatal; these are not resolvable from within RIPS

or correctable by the average  user.   Such  errors  include

missing  directories  or files, operating system errors, and

hardware failures.  In these  cases  RIPS  delivers  a  curt

message  specifying  the  site of the conflict and the error

type  and  performs  a  subsequent  exit   to   the   shell.

Throughout   the   instructions   available   to  users  are

advisements to send such problems via electronic mail to the

management for resolution.


                        CONCLUSIONS


     The goal in the development of RIPS was to provide  the

academician with an economical and flexible means with which

to  store,   organize,   maintain,   request,   and   search

information  contained  in a personal library.  As suggested

by Horowitz and Bleich (1981), idealized information  access







Klyce, et al                -18-                        RIPS



must  meet  certain  criteria  such  as  self-service, rapid

retrieval, accommodation of  all  material,  data  security,

etc.    Several   systems   for  storage  and  retrieval  of

bibliographic references have been suggested (Clark et  al.,

1979; Bertrand et al. Bader, 1980; Johns et al.  , 1980) and

programs are available for the interactive maintenance of  a

personal library (Sterner et al., 1976; Hosono, 1980; Marks,

1981).  However, none  incorporate  many  of  the  desirable

features  found  in RIPS.  Perhaps the best liked feature of

RIPS as reported by users is its user friendliness.  None of

the individual tasks for which RIPS can be used requires any

computer  knowledge   beyond   signing   on.    Secretaries,

students,  technicians,  and  academicians  use  the  system

effectively within minutes of the first log on.   The  entry

of  information is simple and retrievals are straightforward

since the vocabulary used for keywords is unrestricted.  The

amount  of information that may be associated with reference

items is virtually  unlimited.   Entire  files  composed  of

thousands  of  references  can be scanned for retrieval in a

few minutes without the need  of  complex  iterative  search

strategies  suggested  elsewhere  (Bell et al., 1980; Bates,

1981).   In  fact,  the  simplicity  or  complexity   of   a

literature  search depends entirely on the cleverness of the

user and his familiarity with the information content of his

personal  files.   Simple  searches  can yield very specific

bibliographic  listings  for  manuscript  preparation;   the

formatting  and  sorting  can be standardized or flexible as

the user wishes.  In a way, the rapid feedback of  retrieved







Klyce, et al                -19-                        RIPS



information  reinforces  a  user's  recollection of a file's

content, a useful associative tool with  which  to  remember

key citations in various areas of knowledge.

     Currently, staff members at the  LSU  Eye  Center  have

used  RIPS  for nearly two years to create complete files of

all references in their personal libraries, to  print  cards

for  card  files,  to  merge  files  of  similar information

content,  to  retrieve  sources  from  files  outside  their

specialty,  to  request reprints of recent publications, and

to prepare final draft bibliographies that can be optionally

transferred electronically to dedicated word processors used

by the secretaries.  During this time  of  development,  the

software  has  proven  robust  and  the  data  base has been

totally secure.

     Our  goal  for  management  of  personal  libraries  is

largely  met  by  RIPS.   The  bulk  of the system should be

easily portable  to  other  UNIXT  or  UNIXT-like  operating

systems  with  a  minimum  of  adaptation  for  site to site

variations in hardware and software.  While RIPS  by  design

is  compact, it is targeted to UNIXT-based multiuser systems

with a minimum recommended hardware  configuration:  16  bit

processor(s);  256  Kbyte  of random access memory; 10 Mbyte

hard disk mass storage; standard 80x24 character CRT display

terminals;  and  a quality printer with upper and lower case

capability.  However, such a system with four user terminals

and  a  UNIXT  operating  system  can be purchased today for

under  US$  20,000.   The  relative  ease  with   which   we

implemented   RIPS,   although   we   are  neither  computer







Klyce, et al                -20-                        RIPS



scientists nor information scientists, is a tribute  to  the

authors of UNIXT (Ken Thompson and Dennis Ritchie) and the C

Programming Language (Dennis Ritchie).




                      ACKNOWLEDGEMENTS


     This work  was  supported  by  N.I.H.  grants  EY03311,

EY07073,  and  EY02377 from the National Eye Institute.  The

programs  (source  listings  available  on   request)   were

developed  and the manuscript was prepared with the standard

software development tools of  the  UNIXT  operating  system

under  academic  license  from  Bell  Laboratories using the

LSU/Lions Eye Research Computer  Facility  provided  by  the

Louisiana Lions Eye Foundation.


                         REFERENCES


Bates M., Search techniques.  In, Annual review of

   information science and technology.  Ed. by Williams

   M.E. Knowledge Industry Pubs. for ASIS, White Plains, NY

   (1981) pp.139-170.



Bell C.L.M. and Jones K.P., The development of a highly

   interactive searching technique for MORPHS-minicomputer

   operated retrieval (partially heuristic) system.

   Inf. Process. Manage. 16(1) (1980) 37-47.



Bertrand D. and Bader C.R., Storage and retrieval of

   bibliographic references using a microprocessor system.







Klyce, et al                -21-                        RIPS



   Int. J. Bio-Med. Comput. 11   (1980) pp. 285-293.



Bird R., Data base systems: all software versus associative

   file processors.  Comput. Technnol. Rev. II(2) (1982)

   pp. 55-61.



Clark M.E. and Wildhagen E.C.,  An information storage and

   retrieval system using a microcomputer.  Bull. Am. Meteorol

   Soc. 60(1) (1979) pp. 28-31.



Hosono K., Development of a personalized information

   retrieval system by using APL Programming Language.  In,

   Communicating Information.  Proceedings of the 43rd ASIS

   Annual Meeting.  Ed. by Benenfeld AR and Kazlaukas EJ.

   Knowledge Industry Pubs. for ASIS, WhitePlains (1980)

   pp. 77-79.



Johns K.A.,  Stuart G.M.  and Wilson D.W., An effective and

   reliable computerized system for storage and retrieval of

   references to scientific publications.  Int. J. Bio-Med.

   Comput. 11 (1980) pp. 265-284.



Kernighan B. and Ritchie D., The C Programming Language.

   Prentice-Hall,  Englewood Cliffs (1978) 228 pp.



Marks R.H.L., A Fortran computer program for storage and

   retrieval of personal journal references.  Int. J.

   Bio-Med. Comput. 12 (1981) pp. 283-290.







Klyce, et al                -22-                        RIPS



Sterner R.T. and Breidenstein C.P., Set of advanced-BASIC

   programs for literature storage and retrieval with

   minicomputers.  Behav. Res. Methods Instrum. 8(4)

   (1976) pp. 397-398.























































Klyce, et al                -23-                        RIPS



                          LEGENDS

Figure 1.  Flow chart of the RIPS system.  The  UNIXT  shell

is  the  entry  level  program  of the operating system from

which RIPS is called.  Note that  keyword  regeneration  and

file  processing  after  reference  appending  are processes

forked off to run concurrently in the background  permitting

the  user  to quickly continue remaining tasks.  Elements in

quotations are short forms of user-directed queries  offered

in menus.  Abbreviations: DB @ data base; RRflag @ indicator

to the edit append routines that  the  call  came  from  the

reprint  request  generator; TTY @ the user's terminal; LP @

the line printer.






































