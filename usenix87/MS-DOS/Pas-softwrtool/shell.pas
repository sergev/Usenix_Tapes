{SHELL.PAS}

{
        Copyright (c) 1981
        By:     Bell Telephone Laboratories, Inc. and
                Whitesmith's Ltd.,

        This software is derived from the book
                "Software Tools in Pascal", by
                Brian W. Kernighan and P. J. Plauger
                Addison-Wesley, 1981
                ISBN 0-201-10342-7

        Right is hereby granted to freely distribute or duplicate this
        software, providing distribution or duplication is not for profit
        or other commercial gain and that this copyright notice remains
        intact.
}

PROGRAM TOOLS;
{$I TOOLU.PAS}
{$I INITCMD.PAS}
{$I CHAPTER1.PAS}
{$I CHAPTER2.PAS}
{$I CHAPTER3.PAS}
{$I CHAPTER4.PAS}
{$I CHAPTER5.PAS}
{$I CHAPTER6.PAS}
{$I CHAPTER7.PAS}
{$I CHAPTER8.PAS}



VAR
  STR,STR1:STRING80;
  COMMAND:XSTRING;
  DONE:BOOLEAN;
  I:INTEGER;





BEGIN {SHELL}

DONE:=FALSE;
WHILE NOT DONE
DO
    BEGIN
    INITCMD;
    IF GETARG(1,COMMAND,MAXSTR)
    THEN
        BEGIN
        STR:='';
        STR1:='X';
        FOR I:=1 TO XLENGTH(COMMAND)
        DO
            BEGIN
            if COMMAND[I]in[97..122]
            then
                str1[1]:=chr(command[i]-32)
            ELSE STR1[1]:=chr(COMMAND[I]);
            STR:=CONCAT(STR,STR1)
            END;
        if str = 'COPY' then copy
        else if str = 'LINECOUNT' then linecount
        else if str = 'WORDCOUNT' then wordcount
        else if str = 'DETAB' then detab
        else if str = 'ENTAB' then entab
        else if str = 'OVERSTRIKE' then overstrike
        else if str = 'COMPRESS' then compress
        else if str = 'EXPAND' then expand
        else if str = 'ECHO' then echo
        else if str = 'TRANSLIT' then translit
        else if str = 'COMPARE' then compare
        else if str = 'INCLUDE' then include
        else if str = 'CONCAT' then concat
        else if str = 'PRINT' then print
        else if str = 'MAKECOPY' then makecopy
        else if str = 'ARCHIVE' then archive
        else if str = 'SORT' then sort
        else if str = 'UNIQUE' then unique
        else if str = 'KWIC' then kwic
        else if str = 'ROTATE' then writeln('ROTATE not directly supported.')
        else if str = 'UNROTATE' then unrotate
        else if str = 'FIND' then find
        else if str = 'CHANGE' then change
        else if str = 'EDIT' then edit
        else if str = 'FORMAT' then format
        else if str = 'DEFINE' then macro
        else if str = 'MACRO' then macro
        else if str = 'QUIT' then halt
        ELSE
            BEGIN
            WRITELN('?');
            DONE:=FALSE
            END
        END;
    endcmd;
    END;

END.
