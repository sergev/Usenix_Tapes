/*
 * This program emulates a dump terminal with file transfer support using
 * CompuServe's B-Protocol.  This program is just a sample of how to interface
 * the BP module (BP.C) with the rest of the terminal emulator.
 */

#include "\lc\stdio.h"

extern int Transfer_File();     /* Transfer a file using the "B" protocol */
extern int Read_Keyboard();     /* Get a "raw" character from the keyboard */
extern Init_Comm();             /* Initialize the comm port */
extern int Read_Modem();        /* Read a character from the comm port */
extern int Write_Modem();       /* Send a character to the comm port */
extern Term_Comm();

#define True  1
#define False  0
#define Exit_Key  0x010F
#define Is_Function_Key(C)  ((C) & 0x0100)

#define ENQ  0x05
#define DLE  0x10
#define ESC  0x1B

static char VIDTEX_Response[] = "#IB1,PB,DT\015";

static int
    Old_Break_State,
    I,
    Ch,                         /* 16-bit "raw" character. If > 127 than we
                                   have a function key. */
    Want_7_Bit,
    ESC_Seq_State;              /* Escape sequence state variable */


int Wants_To_Abort()
{
    return Read_Keyboard() == ESC;
}

main()
{
    char *cp;

    Want_7_Bit = True;
    ESC_Seq_State = 0;
    Old_Break_State = Get_Break();
    Set_Break(0);
    Init_Comm();
    puts("[ Terminal Mode ]");
    Ch = Read_Keyboard();

    while (Ch != Exit_Key)
        {
        if (Ch > 0)
            {
            if (Is_Function_Key(Ch))
                {
                /* Here to process any local function keys. This sample
                 * program is real dumb. The only function key is the EXIT
                 * key (Alt-X).
                 */
                }
            else
                Write_Modem(Ch & 0x7F);
            }

        if ((Ch = Read_Modem()) >= 0)
            {
            if (Want_7_Bit) Ch &= 0x7F;

            switch (ESC_Seq_State)
                {
                case 0:
                    switch (Ch)
                        {
                        case ESC:
                            ESC_Seq_State = 1;
                            break;

                        case ENQ:
                            /* Enquiry -- send ACK for packet 0 */

                            Write_Modem(DLE);
                            Write_Modem('0');
                            break;

                        case DLE:
                            ESC_Seq_State = 2;
                            break;

                        default:
                            Put_Char(Ch);
                        }

                    break;

                case 1:
                    /* ESC -- process any escape sequences here */

                    switch (Ch)
                        {
                        case 'I':
                            /* Reply to the VIDTEX "ESC I" identify sequence */
                            cp = VIDTEX_Response;
                            while (*cp != 0) Put_Char(*cp++);
                            ESC_Seq_State = 0;
                            break;

                        default:
                            Put_Char(ESC);
                            Put_Char(Ch);
                            ESC_Seq_State = 0;
                        }

                    break;

                case 2:
                    /* DLE */

                    if (Ch == 'B')
                        {
                        /* Start of "B" protocol packet. Go into protocol
                         * mode and transfer the file as requested.
                         */

                        if (!Transfer_File()) puts("Transfer failed!");
                        }
                    else
                        {
                        Put_Char(DLE);
                        Put_Char(Ch);
                        }

                    ESC_Seq_State = 0;
                }
            }

        Ch = Read_Keyboard();
        }

    Term_Comm();
    Set_Break(Old_Break_State);
}
