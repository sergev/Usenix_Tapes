





                            RIPS
        Reference Information Program for Scientists
                          TUTORIAL
               S. Klyce          Jan. 4, 1983


     This program was written at  LSU  (Dr  Klyce)  to  help
staff  manage  their  reprint files.  The more useful attri-
butes include:

      1.  Storage of reprint references in  individual
      (personal)  files  which can be searched for the
      preparation of formatted bibliographies  or  for
      pulling articles on file.

      2.  Complete file management  with  editing  and
      automatic  regeneration  of  sorted keyword list
      including elimination of  "unkeywords"  such  as
      prepositions, adverbs, etc.

      3.  Automatic file card printing for  hard  copy
      index  file.   Latest  entries  stored in parent
      file plus in parent.new so the index file can be
      easily updated.

      4.  Production of reprint request post cards and
      mailing labels.

      5.  Menu driven, practically  self-instructional
      interactive programming.



     To get started you should have a user  account  on  the
LSU/Lions  Eye  Research Computer and have studied the docu-
ment "UNIX for  Beginners."   Following  this,  gather  your
reprints  or  file cards and enter the program by logging in
and typing "rips" followed by <return>.  This should get you
into  the system.  When asked for file name enter one of the
databases listed (NOT ones with suffixes .new or  .kwl),  or
create  a  new one by entering the initials of the Principle
Investigator, or get instructions (this  file)  by  entering
help.

      Note that you will have access to all  the
      files  in the system and can search these.
      However, you will be denied permission  to
      enter the editor if (1) you do not own the
      current file or (2) you are the owner, but
      it  is  currently  being  processed in the
      background.


     After a file has been entered, a menu will  be  printed



                      January 11, 1983





                           - 2 -


which lists available routines:

      e - enter the edit mode.  Currently the standard
      UNIX  ED(I)  is  used.  Refer to the ed tutorial
      manual if you are not familiar with ed.  Options
      in the rips editor are:

            a - append or add references to  the
            file.  When  prompted  for reference
            type, four  options  are  available:
            'j'  for  regular  journal articles,
            'c' for  a  book  chapter,  'b'  for
            book,  and  'q'  for  quit.  For the
            sake of uniformity follow this  for-
            mat:  for authors Lastname II (Jones
            DQ); for year enter year without any
            punctuation  (1899 or 1967 for exam-
            ple);  for  title   enter   complete
            title,  but  DO  NOT add a period at
            its end (rips will do this for you);
            for  journal  use approved abbrevia-
            tion (J. Irreproduc.  Results);  for
            volume,  enter  the  number  without
            punctuation (eg.,  213);  for  pages
            enter  complete pagination (eg, 111-
            123); and for keywords use one  key-
            word  per  entry.   If  you  wish to
            document a book (b) or chapter  (c),
            the  prompts  differ  from  those of
            regular journal articles, but should
            be self-explanatory.

            c - correct entries in file or  key-
            word list.

            s - strategic  search  of  file  for
            structural errors.  While the length
            of a reference is  virtually  unlim-
            ited,   its   structure   (explained
            below)  is  invariant   for   proper
            operation.    Running   this  option
            checks for this type  of  error  and
            reports references out of specifica-
            tion which should then be  corrected
            with the c option.

            r - return to RIPS

            q - exit to the shell

      The editor is useful also to  peruse  the  file.
      Note that when listing it out with the editor on
      the terminal, the raw, unformatted form will  be
      seen.   For  debugging  purposes, this condensed



                      January 11, 1983





                           - 3 -


      format is: | #auths| auth1|
       auth2| ...| authn|  year|  title|  journal  (or
      book  chapter)|  volume # (or rest of book info|
      #keywords| keyword1| keyword2|  ...|  keywordn|.
      Disagreements  between  #auths and actual number
      of authors is fatal.


      r - retrieval of articles in file.  Instructions
      for  use of this module can be obtained on entry
      to it (try it and see).  The retrieval is set up
      interpret  a  command  line composed of (nearly)
      any number of keywords and intercalated conjunc-
      tions   with   (nearly)   any  level  of  nested
      parenthetical expressions.  AND and OR are  true
      Boolean  logical  equivalents.  The NOT conjunc-
      tion modifier is useful for exclusive or limited
      searches.  The prepositions BEFORE, AFTER and IN
      are also recognized for restricting searches  to
      certain  time  periods.   The  search  algorithm
      looks at the entire  reference  entry  including
      the  keywords you originally associated with the
      article.  NOTE:  As you create the file, do  not
      enter  keywords that are in the title since this
      is redundant and wastes disk space.   Successful
      searches  can be listed on the terminal, but are
      always  stored  in  your   home   directory   as
      /usr/{your_name}/refs?????,  where  ?????  is  a
      fairly unique number (the process ID  number  of
      your  current rips job).  To relist this file on
      the terminal, type "cat refs*"; to print on  the
      line  printer  type  "cat  refs*  |  {d}lpr -f".
      Note: if you have more than one refs?????  file,
      cat  refs* will list them all.  To list the last
      one, type "lss" and cat the refs????? file  with
      the   numerically   greatest  suffix,  e.g.  cat
      refs01234 or cat *01234.

      g - this command  will  regenerate  your  entire
      keyword  list (stored in {file_name}.kwl).  Note
      that a kwl is created only on command.  You  are
      warned when printing a keyword list if it is not
      current.

      d - this will allow you to change over to a  new
      file.

      p - general purpose print function.

            t - print reference list  on  termi-
            nal.

            p - print  reference  list  on  line
            printer.



                      January 11, 1983





                           - 4 -


            k - print keyword list on terminal.

            l  -  print  keyword  list  on  line
            printer.

            c - print cards.  Options: f - print
            file  cards  from  either  the whole
            file  list  or  from  only  the  new
            entries   ({file_name}.new).    r  -
            create  reference  reprint   request
            cards   and   corresponding  labels.
            This also adds entered references to
            the  current file.  More information
            is available within rips.

      w - this command produces a  temporary  exit  to
      the  UNIX  shell  (%).   The first <cntrl> d you
      type while out there should bring  you  back  to
      rips.

      q - permanent exit from rips to the shell.




     This tutorial is terse for lack  of  time  (as  usual),
since  we're expected to do science first and program later.
Now it's much later, so GOOD LUCK.  Please report any diffi-
culties, problems by <mail>ing klyce.




























                      January 11, 1983


