From CROFT@SUMEX-AIM.ARPA Tue Jul 31 17:03:37 1984
Received: from SUMEX-AIM.ARPA by safe with TCP; Tue, 31 Jul 84 17:03:27 pdt
Date: Tue 31 Jul 84 17:02:41-PDT
From: Bill Croft <CROFT@SUMEX-AIM.ARPA>
Subject: [Ed Pattermann <PATTERMANN@SUMEX-AIM.ARPA>: C version of MACIMP (* source code *)]
To: croft@SUMEX-AIM.ARPA
Status: R

Mail-From: PATTERMANN created at 28-Jul-84 14:29:42
Mail-From: PATTERMANN created at 27-Jul-84 15:27:55
Date: Fri 27 Jul 84 15:27:55-PDT
From: Ed Pattermann <PATTERMANN@SUMEX-AIM.ARPA>
Subject: C version of MACIMP (* source code *)
To: info-mac@SUMEX-AIM.ARPA
ReSent-date: Sat 28 Jul 84 14:29:42-PDT
ReSent-From: Ed Pattermann <PATTERMANN@SUMEX-AIM.ARPA>
ReSent-To: info-mac: ;

Here is the UNIX/C version of MACIMP (also on {SUMEX-AIM}<INFO-MAC>MACIMP.C).
Thanks to Dan Winkler of Harvard for this version.

---------

/*
     This filter converts MacPaint files into Impress files
     for printing on an Imagen laser printer.  Based on MACimp
     for Tops-20 by Ed Pattermann, Stanford.

     Usage:  macimp < macdoc.mp | ipr

     Send enhancements and bug reports to winkler@harvard.

     History:      
	
	25 July 1984  Winkler created. 

	27 July 1984  Burgess (sumex-aim): modified write_imp_init()
		      code to reflect appropriate owner and spooldate.

*/

#include <stdio.h>
#include <strings.h>

#define     MAG_POWER           1      /* can be 0, 1, or 2 */
#define     DOTS_PER_INCH       240

#define     OPAQUE              3
#define     OPERATION_TYPE      OPAQUE
#define     HSIZE               18
#define     VSIZE               23

#define     MAC_HEADER_LENGTH   512
#define     PAD_BYTES           1152

#define     LOBYTE(number) ((char)(number & 0x00FF))
#define     HIBYTE(number16) ((char)((number16 >> 8) & 0x00FF))

#define     PATCH_SIZE          32
#define     PATCH_BYTES         4

typedef char patch[ PATCH_SIZE ][ PATCH_BYTES ] ;

main()
{
     write_imp_init() ;
     throw_away_mac_header() ;
     process_data() ;
     write_imp_end() ;
     fflush(stdout) ; 
}

process_data()
{
     int data_byte ; short repeat_count, byte_num ;

     while ( (data_byte = getchar()) != EOF )
     {
          if ( data_byte > 127 )
          {
               repeat_count = 256 - data_byte + 1 ;
               data_byte = getchar() ;
               for (byte_num = 0 ; byte_num < repeat_count ; byte_num ++ )
                    add_byte ( (char) data_byte ) ;     
               
          }
          else 
          {
               repeat_count = data_byte + 1 ;
               for ( byte_num = 0 ; byte_num < repeat_count ; byte_num ++ )
                    add_byte ( (char) getchar() ) ;
          }
     }
     
     for ( byte_num = 0 ; byte_num < PAD_BYTES ; byte_num ++ )
          add_byte ( (char) 0 ) ;
}

add_byte( one_byte )
     char one_byte ;
{
     static patch patches[ HSIZE ] ; 
     static short patch_num = 0 ; 
     static short patch_row = 0 ; static short patch_col = 0 ;

     patches[ patch_num ][ patch_row ][ patch_col ] = one_byte ;

     if ( ++ patch_col >= PATCH_BYTES )
     {
          patch_col = 0 ;
          if ( ++ patch_num >= HSIZE )
          {
               patch_num = 0 ; 
               if ( ++ patch_row >= PATCH_SIZE )
               {
                    patch_row = 0 ;
                    write_patches ( patches ) ;
               }
          }
     }
}

write_patches ( patches )
     patch * patches ;
{
     short patchnum, patchrow, patchcol ;
     
     for ( patchnum = 0 ; patchnum < HSIZE ; patchnum ++ )
          for ( patchrow = 0 ; patchrow < PATCH_SIZE ; patchrow ++ )
               for ( patchcol = 0 ; patchcol < PATCH_BYTES ; patchcol ++ )
                    putchar( patches[ patchnum ][ patchrow ][ patchcol ] ) ;
}

throw_away_mac_header()
{
     short bytenum ;

     for ( bytenum = 0 ; bytenum < MAC_HEADER_LENGTH ; bytenum ++ ) 
          ( void ) getchar() ;
}

write_imp_init()
{
     char *getenv();
     char *ctime(), cimptime[26];
     long time(), imptime;

     imptime = time(0);			/* get time in internal form */
     strcpy (cimptime, ctime(&imptime));/* put time in string form */
     cimptime[24] = '\0';		/* nullify \n in time string */

     printf("@DOCUMENT(LANGUAGE IMPRESS, NAME \"MacPaint Document\"") ;
     printf(", OWNER \"%s\"",getenv("USER"));
     printf(", SPOOLDATE \"%s\")",cimptime);
     /* SET_ABS_H */
          putchar( (char) 135 ) ; 
          putchar( HIBYTE( (short) DOTS_PER_INCH )) ;
          putchar( LOBYTE( (short) DOTS_PER_INCH )) ;

     /* SET_ABS_V */
          putchar( (char) 137 ) ; 
          putchar( HIBYTE( (short) DOTS_PER_INCH )) ;
          putchar( LOBYTE( (short) DOTS_PER_INCH )) ;

     /* SET_MAGNIFICATION */
          putchar( (char) 236 ) ; putchar( (char) MAG_POWER ) ;

     /* BITMAP */
          putchar( (char) 235 ) ; putchar( (char) OPERATION_TYPE ) ;
          putchar( (char) HSIZE ) ; putchar( (char) VSIZE ) ;
}

write_imp_end()
{
     /* ENDPAGE */ 
        putchar( (char) 219 ) ;

     /* EOF */ 
        putchar( (char) 255 ) ;
}

-------
-------

