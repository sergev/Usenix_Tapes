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
' '	3
'e'	4
't'	4
'o'	4
'a'	4
'n'	4
'i'	5
's'	5
'r'	5
'h'	5
'l'	5
'd'	5
10	6	/* newline */
'u'	6
'c'	6
'm'	6
'p'	6
'f'	6
'y'	7
'g'	7
'w'	7
'b'	7
'.'	7
'v'	7
','	7
9	8	/* horizontal tab */
'-'	8
'k'	8
'I'	8
'O'	8
'T'	8
'S'	8
39	8	/* single quote "'" */
'"'	8
8	9	/* backspace */
'!'	9
'#'	9
'$'	9
'%'	9
'&'	9
'('	9
')'	9
'*'	9
'+'	9
'/'	9
'0'	9
'1'	9
'2'	9
'3'	9
'4'	9
'5'	9
'6'	9
'7'	9
'8'	9
'9'	9
':'	9
';'	9
'<'	9
'='	9
'>'	9
'?'	9
'@'	9
'A'	9
'B'	9
'C'	9
'D'	9
'E'	9
'F'	9
'G'	9
'H'	9
'J'	9
'K'	9
'L'	9
'M'	9
'N'	9
'P'	9
'Q'	9
'R'	9
'U'	9
'V'	9
'W'	9
'X'	9
'Y'	9
'Z'	9
'['	9
'\'	9
']'	9
'^'	9
'_'	9
'`'	9
'j'	9
'q'	9
'x'	9
'z'	9
'{'	9
'|'	9
'}'	9
'~'	9
EOFMARK	9
EXTCHAR	9
