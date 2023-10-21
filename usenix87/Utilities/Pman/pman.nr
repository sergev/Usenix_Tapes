


PMAN(1WI)              UNIX User's Manual               PMAN(1WI)



NAME
     pman - extract selected parts of an on-line manual page

SYNTAX
     pman [-abcdehknosvwx] [-+ name] [section] [-] names

DESCRIPTION
     pman is used to print one or more parts of a man page, such
     as the NAME or EXAMPLE section.  The program allows quick
     reference to the on-line manual, showing only parts of
     interest.  Synonyms are used to avoid overlooking topics.  A
     part begins with a section name, and ends with a new part.
     A new part is a line beginning with at least with two upper
     case letters (except for nroff page breaks that may contain
     the upper case name of the entry).  Sections are printed in
     the order they appear in the manual entry, not in the order
     requested.  Blank lines are suppressed, and there is an
     option for extra-compact output, making pagination by pro-
     grams like more unnecessary.

OPTIONS
     The section of the manual (1-8) can be supplied after all
     the flag options and before the names.  Otherwise, all sec-
     tions are searched until an entry is found.  The manual
     entry can be read from the standard where a single - is
     placed instead of a name.

     Most option letters correspond to a man page section.  With
     no options, the -n, -s, and -e options are assumed.

     -a   Print all entries in a requested section or all sec-
          tions of the given name.  This allows searching for
          more than one version of an entry, which can happen
          with local implementations.  This option implies the -h
          option so that the section is printed.

     -b   Non-redundant blank lines are printed.  By default,
          these are removed to compress the output.

     -c   Compacted output.  With this option, pman tries to put
          parts on one line.  This option assumes that text is
          indented five spaces and may be upset by tabs in the
          formatted entry.

     -h   Print a header with the entry's name and source sec-
          tion.

     Sections

     The following options print parts of manual entries or types
     of manual entries (such as all sections with warnings).




Printed 6/2/85            May 27, 1985                          1






PMAN(1WI)              UNIX User's Manual               PMAN(1WI)



     -+ name
          Add name to the list of printed parts.  This allows
          access to sections not anticipated by pman, or using
          pman without having to know the options.

     -d   DESCRIPTION, OPTIONS, NOTES, COMMENTS, COMMANDS (detail
          about the entry)

     -e   EXAMPLES or HINTS

     -k   KEYWORDS, NAME, or SEE ALSO

     -n   NAME (name and purpose of the entry)

     -o   OPTIONS (unfortunately, few files use a separate sec-
          tion expressly for option descriptions)

     -s   SYNTAX, SYNOPSIS, or USAGE

     -v   VARIABLES, ENVIRONMENT, or FILES (special variables or
          files used by the entry)

     -w   WARNINGS, BUGS, LIMITATIONS, RESTRICTIONS, DIAGNOSTICS,
          CAVEATS, NOTES, and STATUS

     -x   SEE ALSO, REFERENCES, ALGORITHM, and AUTHOR (cross-
          references)

EXAMPLES
     pman -s echo             # SYNTAX from man echo
     pman -vw 1 man           # VARIABLES & WARNINGS from man 1 man
     pman -+ 7 REQ man        # REQUESTS section on man macros
     pman -hd w | sed 's/.^H//g'# header & detail unhighlighted

FILE
     /usr/man/cat?/* (formatted manual entries searched for here)

SEE ALSO
     man(1)

KEYWORDS
     help, documentation

AUTHOR
     Gary Perlman (based on a shell script by Bob Marcus)

BUGS
     The compacted output may have problems when there are tabs
     in the formatted text.

RESTRICTIONS




Printed 6/2/85            May 27, 1985                          2






PMAN(1WI)              UNIX User's Manual               PMAN(1WI)



     pman will not work if sections headings are indented or con-
     tain overstriking to embolden.  The filter: sed 's/.^H//g'
     can be used with the standard input option to help with this
     problem.

STATUS
     I am open to suggestions for enhancements.  Bug fixes and
     new features are appreciated and will be acknowledged.

     The program has been tested on 4.2 BSD UNIX and System V.













































Printed 6/2/85            May 27, 1985                          3



