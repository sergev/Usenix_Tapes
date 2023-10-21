:/*
: * Binary batching
: */
:
:#define BVERSION 0		/* version number */
:
:#define LFTYPE 2		/* length of an (encoded) file type */
:char ftypes[] = "nma" ;	/* file types */
:
:#define BS 8			/* size of a byte */
:

Usenet header lines:
NHRELAY		"Relay-Version: version "
NHVERSION	"Posting-Version: version "
NHPATH		"Path: "
NHFROM		"From: "
NHNEWSGROUPS	"Newsgroups: "
NHSUBJECT	"Subject: "
NHRESUBJECT	"Subject: Re: "
NHMESSAGEID	"Message-ID: <"
NHDATE		"Date: "
NHARTICLEID	"Article-I.D.: "
NHPOSTED	"Posted: "
NHEXPIRES	"Expires: "
NHREF		"References: "
NHCONTROL	"Control: "
NHSENDER	"Sender: "
NHREPLYTO	"Reply-To: "
NHFOLLOWUPTO	"Followup-To: "
NHDIST		"Distribution: "
NHORG		"Organization: "
NHLINES		"Lines: "
NHKEYWORDS	"Keywords: "
NHAPPROVED	"Approved: "
NHSUMMARY	"Summary: "
NHPRIORITY	"Priority: "
NHNFID		"NF-ID: #"
NHNFFROM	"NF-From: "
NHUNKNOWN	""
NHSEP
NHEND
:
:#ifdef UNBAT
:
:struct decode {	/* decoding table */
:      char cval ;	/* decoded character */
:      char clen ;	/* length */
:} ;
:
:#else
:
:struct encode {	/* encoding table */
:      short cbits ;	/* character length */
:      short clen ;	/* character length */
:} ;
:
:#endif
:
Character lengths:
' '	2
'e'	4
't'	4
'o'	4
'a'	5
'n'	5
'i'	5	/* second half of table begins here */
's'	5
'r'	5
'h'	5
'l'	5
'd'	5
10	5	/* newline */
'u'	6	/* final quarter of table should begin here */
'c'	6
'm'	6	/* final quarter of table */
'p'	6
'f'	6
'y'	6
'g'	6
'w'	7
'b'	7	/* final eighth of table should begin here */
'.'	7
'v'	7
','	7
9	7	/* horizontal tab */
'-'	8	/* final eighth of table begins here */
'k'	8
'I'	8
'0'	8
'T'	8
'S'	8
39	8	/* single quote "'" */
'"'	9
'A'	9
'!'	9
'#'	9
'('	9
')'	9
'/'	9
'1'	9
'2'	9
'3'	9
'4'	9
'5'	10
'6'	10
'7'	9
'8'	10
'9'	10
':'	9
'<'	9
'@'	9
'B'	9
'C'	9
'D'	9
'E'	9
'M'	9
'N'	9
'O'	9
'P'	9
'R'	9
'U'	9
'W'	9
'Y'	9
'\'	9
'x'	9
'{'	9
'}'	9
8	10	/* backspace */
13	10	/* carriage return (could be left to EXTCHAR) */
'$'	10
'%'	10
'&'	10
'*'	10
'+'	10
';'	10
'='	10
'>'	10
'?'	10
'F'	10
'G'	10
'H'	10
'J'	10
'K'	10
'L'	10
'Q'	10
'V'	10
'X'	10
'Z'	10
'['	10
']'	10
'^'	10
'_'	10
'`'	10
'j'	10
'q'	10
'z'	10
'|'	10
'~'	10
EOFMARK	10
EXTCHAR	9
