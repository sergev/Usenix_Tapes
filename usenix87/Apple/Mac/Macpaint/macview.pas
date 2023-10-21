
(*
       MACVIEW.PAS - Display MacPaint pictures on the IBM PC
                     graphics adapter.

                     Mark Anacker    09/24/85
                                                                     *)
PROGRAM MACVIEW;
(*$V-*)

TYPE
  STRING255 = STRING[255];
  SCANLINE = ARRAY [1..80] OF BYTE;      (* hi-res screen is 80 bytes wide *)

CONST
  SCANFLAG : BOOLEAN = TRUE;
  ODDLINE  : INTEGER = 0;
  EVENLINE : INTEGER = 0;

VAR
  FILNAM : STRING[64];
  PNTFIL : FILE OF BYTE;
  ELEN   : BYTE;
  ELEM   : STRING255;
  OUTPAT : STRING255;
  LCNT   : INTEGER;
  CNT    : INTEGER;
  XPOS,YPOS : INTEGER;
  STLINE,ENLINE : INTEGER;
  EVSCREEN : ARRAY [0..99,1..80] OF BYTE ABSOLUTE $B800:$0000;
  ODSCREEN : ARRAY [0..99,1..80] OF BYTE ABSOLUTE $B800:$2000;

  DOIT : BOOLEAN;

  SLPTR : ARRAY [0..719] OF ^SCANLINE;   (* pointers to scan line buffers *)

PROCEDURE HELPMSG;
       BEGIN
         WRITE('MACVIEW - View MacPaint images  ...  by Mark Anacker');
         WRITELN('  09/24/85'); WRITELN;;
         WRITELN('This program will let you view an entire MacPaint image.');
         WRITELN('First, transfer the picture file from the Mac.  Then,');
         WRITELN('run this program and give it the file name.  You may use');
         WRITELN('the keypad keys 1-3 and 7-9 to scroll over the image.');
         WRITELN('Press the space bar to exit back to DOS.');
         WRITELN;
         WRITELN('You may specify the file name on the command line.');
         WRITELN;
         WRITELN('Remember, the aspect ratio is different.  The picture will');
         WRITELN('be distorted somewhat in the vertical direction.');
         WRITELN;
       END;

PROCEDURE GETFILE;
VAR    CNT : INTEGER;
       INCH : CHAR;
       BEGIN
         ASSIGN(PNTFIL,FILNAM);
         (*$I-*) RESET(PNTFIL); (*$I+*)
         IF IORESULT<>0 THEN
           BEGIN
             WRITELN('** Error opening file - halting **'); HALT(1);
           END;
         SEEK(PNTFIL,512);               (* skip brush patterns *)
         YPOS:=0; LCNT:=0; OUTPAT:='';
         SCANFLAG:=TRUE;
         ODDLINE:=0; EVENLINE:=0;
         STLINE:=0;
       END;

PROCEDURE OUTSCREEN;                               (* display line *)
VAR    CNT,CNT2 : INTEGER;
       BEGIN
         FOR CNT:=1 TO LENGTH(OUTPAT) DO           (* invert bits to black *)
           OUTPAT[CNT]:=CHR(NOT ORD(OUTPAT[CNT])); (* on white like the Mac *)
         NEW(SLPTR[LCNT]);                         (* allocate a new buffer line*)
         FILLCHAR(SLPTR[LCNT]^,80,CHR(0));         (* fill it to black *)
         MOVE(OUTPAT[1],SLPTR[LCNT]^,72);          (* and copy the decoded bits *)
       END;

PROCEDURE PUTLINE;                       (* decide if we need to do line *)
       BEGIN
         OUTPAT:=OUTPAT+ELEM;            (* build scan line pattern *)
         IF LENGTH(OUTPAT)>=72 THEN      (* if we have a full line, *)
           BEGIN
             IF LENGTH(OUTPAT)>72 THEN   (* if too long, truncate *)
               OUTPAT:=COPY(OUTPAT,1,72);
             OUTSCREEN;                  (* put it in the buffer *)
             LCNT:=LCNT+1; OUTPAT:='';
             FILLCHAR(OUTPAT,75,CHR(0)); (* reset the string *)
           END;
       END;

PROCEDURE REPBLOCK;                      (* block of repeating data *)
VAR    TMPBYTE : BYTE;
       CNT : INTEGER;
       BEGIN
         ELEN:=(256-ELEN);               (* get character count *)
         READ(PNTFIL,TMPBYTE);           (* get character to repeat *)
         ELEM:='';
         FOR CNT:=0 TO ELEN DO
           ELEM:=CONCAT(ELEM,CHR(TMPBYTE));  (* make string of chars. *)
         PUTLINE;                        (* test for a complete scan line *)
       END;

PROCEDURE MIXBLOCK;                      (* block of mixed, raw data *)
VAR    TMPBYTE : BYTE;
       CNT : INTEGER;
       BEGIN
         ELEM:='';
         FOR CNT:=0 TO ELEN DO
           BEGIN
             READ(PNTFIL,TMPBYTE);             (* get characters *)
             ELEM:=CONCAT(ELEM,CHR(TMPBYTE));  (* add to running pattern *)
           END;
         PUTLINE;                        (* test for complete scan line *)
       END;

PROCEDURE LOADBUF;                       (* read data from file *)
       BEGIN
         GETFILE;                        (* open file *)
         WRITELN('Loading picture into buffer ... Please wait a moment');
         REPEAT
           BEGIN
             READ(PNTFIL,ELEN);          (* get a byte *)
             IF ELEN>127 THEN            (* if 8th bit set, *)
               REPBLOCK                  (* it's a repeater, else *)
              ELSE
               MIXBLOCK;                 (* it's mixed *)
           END;
         UNTIL LCNT>=720;                (* until all 720 lines are done *)
         CLOSE(PNTFIL);
       END;

PROCEDURE SHOWBUF;                       (* display the buffer on screen *)
VAR    CNT : INTEGER;
       BEGIN
         EVENLINE:=0; ODDLINE:=0;
         FOR CNT:=STLINE TO STLINE+199 DO  (* show the current 200 scan lines *)
           BEGIN
             IF (CNT AND 1)<>1 THEN       (* even line *)
               BEGIN
                 MOVE(SLPTR[CNT]^,EVSCREEN[EVENLINE,1],80);  (* even lines *)
                 EVENLINE:=EVENLINE+1;
               END
              ELSE
               BEGIN                     (* odd line *)
                 MOVE(SLPTR[CNT]^,ODSCREEN[ODDLINE,1],80);   (* odd lines *)
                 ODDLINE:=ODDLINE+1;
               END;
             END;
       END;

PROCEDURE MOVEIT;                        (* scroll the picture up and down *)
CONST
  FKTABLE : STRING[6] = 'OPQGHI';        (* cursor/numeric key *)
  FKEQUIV : STRING[6] = '123789';        (* conversion table *)
VAR    INCH : CHAR;
       CNT : INTEGER;
       BEGIN
         REPEAT
           GOTOXY(75,1); WRITE(STLINE:3);  (* put top scan line in upper corner *)
           READ(KBD,INCH);               (* get the key from the user *)
           IF (INCH=CHR(27)) AND KEYPRESSED THEN   (* if a real cursor key, *)
             BEGIN
               READ(KBD,INCH);           (* get the key code *)
               IF POS(INCH,FKTABLE)>0 THEN (* and convert to it's number *)
                 BEGIN
                   CNT:=POS(INCH,FKTABLE); INCH:=FKEQUIV[CNT];
                 END;
             END;
           CASE INCH OF
            '8' : BEGIN                  (* move image UP *)
                    IF STLINE<520 THEN
                      BEGIN
                        FOR CNT:=0 TO 98 DO
                          BEGIN
                            MOVE(EVSCREEN[CNT+1,1],EVSCREEN[CNT,1],72);
                            MOVE(ODSCREEN[CNT+1,1],ODSCREEN[CNT,1],72);
                          END;
                        FILLCHAR(EVSCREEN[99,1],72,CHR(0));
                        FILLCHAR(ODSCREEN[99,1],72,CHR(0));
                        STLINE:=STLINE+2;
                        MOVE(SLPTR[STLINE+198]^,EVSCREEN[99,1],72);
                        MOVE(SLPTR[STLINE+199]^,ODSCREEN[99,1],72);
                      END;
                  END;
            '2' : BEGIN                  (* move image DOWN *)
                    IF STLINE>1 THEN
                      BEGIN
                        FOR CNT:=99 DOWNTO 1 DO
                          BEGIN
                            MOVE(EVSCREEN[CNT-1,1],EVSCREEN[CNT,1],72);
                            MOVE(ODSCREEN[CNT-1,1],ODSCREEN[CNT,1],72);
                          END;
                        FILLCHAR(EVSCREEN[0,1],72,CHR(0));
                        FILLCHAR(ODSCREEN[0,1],72,CHR(0));
                        STLINE:=STLINE-2;
                        MOVE(SLPTR[STLINE]^,EVSCREEN[0,1],72);
                        MOVE(SLPTR[STLINE+1]^,ODSCREEN[0,1],72);
                      END;
                  END;
            '3' : BEGIN                  (* page image DOWN *)
                    STLINE:=STLINE-100;
                    IF STLINE<0 THEN STLINE:=0;
                    SHOWBUF;
                  END;
            '9' : BEGIN                  (* page image UP *)
                    STLINE:=STLINE+100;
                    IF STLINE>520 THEN STLINE:=520;
                    SHOWBUF;
                  END;
            '7' : BEGIN                  (* go to TOP of image *)
                    STLINE:=0; SHOWBUF;
                  END;
            '1' : BEGIN                  (* go to BOTTOM of image *)
                    STLINE:=514; SHOWBUF;
                  END
           END;
         UNTIL INCH=' ';                 (* exit when SPACE bar is pressed *)
       END;                              (* moveit *)

               (* main section *)

BEGIN
  TEXTCOLOR(7);                          (* set color to gray *)
  IF PARAMCOUNT=0 THEN                   (* if a blank command line, *)
    BEGIN
      HELPMSG;                           (* show message, prompt for file *)
      WRITE('MacPaint file name : '); READLN(FILNAM);
      IF FILNAM='' THEN HALT(2);         (* if none given, exit to DOS *)
    END
   ELSE
    FILNAM:=PARAMSTR(1);                 (* else get file name from cmd line *)
  IF POS('.',FILNAM)=0 THEN              (* if no extension was given, *)
   FILNAM:=FILNAM+'.MCP';                (* assume .MCP *)
  LOADBUF;                               (* load the file *)
  HIRES;                                 (* switch to hi-res mode *)
  SHOWBUF;                               (* display the buffer *)
  MOVEIT;                                (* move it up and down *)
  CLRSCR;                                (* clear the screen when done *)
END.
