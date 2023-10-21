/*
 * tinyftp.c - user ftp built on tinytcp.c
 *
 * Written March 31, 1986 by Geoffrey Cooper
 *
 * Copyright (C) 1986, IMAGEN Corporation
 *  "This code may be duplicated in whole or in part provided that [1] there
 *   is no commercial gain involved in the duplication, and [2] that this
 *   copyright notice is preserved on all copies.  Any other duplication
 *   requires written notice of the author (Geoffrey H. Cooper)."
 */
#include "tinytcp.h"

tcp_Socket ftp_ctl, ftp_data, ftp_data2;
byte ftp_cmdbuf[120];
int ftp_cmdbufi;

byte ftp_outbuf[80];
int ftp_outbufix, ftp_outbuflen;
    
short ftp_rcvState;
#define ftp_StateGETCMD     0       /* get a command from the user */
#define ftp_StateIDLE       1       /* idle connection */
#define ftp_StateINCOMMAND  2       /* command sent, awaiting response */
#define ftp_StateRCVRESP    3       /* received response, need more data */

char *ftp_script[7];
int ftp_scriptline;
char ftp_retrfile[80];
BOOL ftp_echoMode;

ftp_ctlHandler(s, dp, len)
    tcp_Socket *s;
    byte *dp;
    int len;
{
    byte c, *bp, data[80];
    int i;

    if ( dp == 0 ) {
        tcp_Abort(&ftp_data);
        return;
    }

    do {
        i = len;
        if ( i > sizeof data ) i = sizeof data;
        MoveW(dp, data, i);
        len -= i;
        bp = data;
        while ( i-- > 0 ) {
            c = *bp++;
            if ( c != '\r' ) {
                if ( c == '\n' ) {
                    ftp_cmdbuf[ftp_cmdbufi] = 0;
                    ftp_commandLine();
                    ftp_cmdbufi = 0;
                } else if ( ftp_cmdbufi < (sizeof ftp_cmdbuf)-1 ) {
                    ftp_cmdbuf[ftp_cmdbufi++] = c;
                }
            }
        }
    } while ( len > 0 );
}

ftp_commandLine()
{
    printf("> %s\n", ftp_cmdbuf);
    switch(ftp_rcvState) {
     case ftp_StateIDLE:
        if ( ftp_cmdbuf[3] == '-' )
            ftp_rcvState = ftp_StateRCVRESP;
        break;

     case ftp_StateINCOMMAND:
        if ( ftp_cmdbuf[3] == '-' )
            ftp_rcvState = ftp_StateRCVRESP;
     case ftp_StateRCVRESP:
        if ( ftp_cmdbuf[3] == ' ' )
            ftp_rcvState = ftp_StateIDLE;
        break;
    }
}

ftp_Abort()
{
    tcp_Abort(&ftp_ctl);
    tcp_Abort(&ftp_data);
}


ftp_application()
{
    char *s;
    char *dp;
    int i;

    i = -1;
    if ( isina() ) {
        i = busyina() & 0177;
#ifdef DEBUG
        if ( i == ('D' & 037) ) SysBug("Pause to DDT");
#endif
        if ( i == ('C' & 037) ) {
            printf("Closing...\n");
            tcp_Close(&ftp_ctl);
        }
    }

    switch (ftp_rcvState) {
      case ftp_StateGETCMD:
 getcmd:if ( i != -1 ) {
            ftp_outbuf[ftp_outbuflen] = 0;
            switch (i) {
                case 'H' & 037:
                case 0177:
                    if ( ftp_outbuflen > 0 ) {
                        ftp_outbuflen--;
                        printf("\010 \010");
                    }
                    break;

                case 'R' & 037:
                    if ( ftp_echoMode )
                        printf("\nFtpCmd> %s", ftp_outbuf);
                    break;

                case 033:
                    ftp_echoMode = ! ftp_echoMode;
                    break;

                case '\r':
                case '\n':
                    busyouta('\n');
                    dp = &ftp_outbuf[ftp_outbuflen];
                    goto docmd;

                default:
                    if ( i >= ' ' && ftp_outbuflen < sizeof ftp_outbuf ) {
                        ftp_outbuf[ftp_outbuflen++] = i;
                        if ( ftp_echoMode ) busyouta(i);
                    }
            }
        }
        break;

      case ftp_StateIDLE:
        if ( ftp_scriptline < 0 ) {
            ftp_rcvState = ftp_StateGETCMD;
            ftp_echoMode = true;
            ftp_outbuflen = 0;
            printf("FtpCmd> ");
            goto getcmd;
        }
        s = ftp_script[ftp_scriptline];
        if ( s == NIL )
            break;
        ftp_scriptline++;
        printf("%s\n", s);
        dp = ftp_outbuf;
        while ( *dp++ = *s++ ) ;
        dp--;
 docmd: *dp++ = '\r';
        *dp++ = '\n';
        ftp_outbuflen = dp - ftp_outbuf;
        ftp_outbufix = 0;
        ftp_rcvState = ftp_StateINCOMMAND;
        /* fall through */
    case ftp_StateINCOMMAND:
        i = ftp_outbuflen - ftp_outbufix;
        if ( i > 0 ) {
            i = tcp_Write(&ftp_ctl, &ftp_outbuf[ftp_outbufix], i);
            ftp_outbufix += i;
            tcp_Flush(&ftp_ctl);
        }
        /* fall through */
    case ftp_StateRCVRESP:
        break;
    }

}

ftp(host, fn, dataHandler)
    in_HwAddress host;
    char *fn;
    procref dataHandler;
{
    word port;
    char filecmd[80];

    port = (sed_lclEthAddr[2] + clock_ValueRough()) | 0x8000;

    if ( fn ) {
        /* set up the script for this session */
        ftp_script[0] = "user foo";
        ftp_script[1] = "pass foo";
        ftp_script[2] = "type i";
        sprintf(filecmd, "retr %s", fn);
        ftp_script[3] = filecmd;
        ftp_script[4] = "quit";
        ftp_script[5] = 0;
        ftp_scriptline = 0;
    } else {
        ftp_scriptline = -1;        /* interactive mode */
        ftp_echoMode = true;
    }

    /* set up state variables */
    ftp_rcvState = ftp_StateRCVRESP;
    ftp_cmdbufi = 0;
    tcp_Listen(&ftp_data, port, dataHandler, 0);
    tcp_Open(&ftp_ctl, port, host, 21, ftp_ctlHandler);
    tcp(ftp_application);
}
