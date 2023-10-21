% run this through PhD-TeX

\input dcustom
\articlestyle
\input version

\def\draftext{%
    \begingroup
	\eightpoint\sf
	Reprinted from
	{\it Computer Networks and ISDN Systems}, 10(2),
	September, {\oldstyle 1985}%
    \endgroup
}
\catcode`\@=11
\def\draftstring{%
    \ifnum\pageno>\z@
	\begingroup
	    \eightpoint\sf
	    Copyright $\copyright$ {\oldstyle 1985},
	    North Holland Publishing Company
	\endgroup
    \fi
}
\def\uci@footpage{\draftstring\hfil\rm\folio}
\catcode`\@=12

\input sfwmac
\def\MTA/{{\sf MTA}}
\def\MTS/{{\sf MTS}}
\def\UA/{{\sf UA}}

\def\uitem#1{\item{\underbar{#1}:}}

\header
    MH: A Multifarious User Agent\title
    Marshall T.~Rose\\
	Member, Research Technical Staff\\
	Northrop Research and Technology Center$^\dagger$\\
	\\
    Einar A.~Stefferud\\
	President, Network Management Associates$^\ddagger$\\
	and Visiting Lecturer, University of California, Irvine\\
	\\
    Jerry N.~Sweet\\
	Member, Technical Staff\\
	Local Network Systems$^{\bowtie}$\author\info
\footnote{}{\hskip -\parindent $^\dagger$
One~Research Park,
Palos Verdes Peninsula, CA  90274.
Telephone: 213/377--4811.\hbreak
Computer mail: {\tx MRose\%NRTC@USC-ECL}}
\footnote{}{\hskip -\parindent $^\ddagger$
17301~Drey Lane,
Huntington Beach, CA  92647.
Telephone: 714/842--3711.\hbreak
Computer mail: {\tx EStefferud@ICS.UCI.EDU}}
\footnote{}{\hskip -\parindent $^{\bowtie}$
130~McCormick Avenue, Suite~102,
Costa Mesa, CA  92626.
Telephone: 714/754--6631.\hbreak
Computer mail: {\tx JSweet@ICS.UCI.EDU}}

\centerline{\sc Abstract}			% mtr
{\rightskip=0pt
\lp
The UCI version of the Rand Message Handling System (\MH/) is discussed,
including important extensions.
\MH/ is a powerful user agent which operates in the ARPA Internet and UUCP
environments.
In addition to the basic functions provided by a user agent,
such as reading and sending mail,
\MH/ has several distinguishing characteristics which give the user
additional message handling capabilities.
In particular,
\MH/ provides mechanisms for organizing messages,
tailoring its own behavior,
and extending its functions.

\lp
This document describes \MH/ from several perspectives.
Particular emphasis is given to:
the \MH/ user environment,
advanced features of \MH/ which have proven to be particularly useful for
sophisticated users of electronic mail,
\MH/'s potential as a record manager,
and
\MH/ as a part of a distributed mail environment.
Although \MH/ as been widely used since its creation in 1979,
a discussion of its perspectives and functionality has not appeared in the
open literature.
\par}

\input text
\input refs
\input appendixA

\printcontents
\unskip\footnote{}{\hskip -\parindent
This document (version \versiontag/)
was \TeX set \today\ with DISS.STY v\version.}
\showsummary

\bye
