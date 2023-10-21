#define D_SECTDES
#define D_SCTSTR
#define D_FILES
#include        <stdio.h>
#include        "empdef.h"

chkok(message)
char    *message;
{
        register        i;
        char    jnk[80];

        if( sect.sct_desig != S_HIWAY ) goto X160;
        if( sect.sct_chkpt != 0 ) goto X160;
X26:    
        return(0);
X32:    
        printf("Checkpoint!\tcode:");
        fflush(stdout);
        signal(02, 01);
        sread(jnk, 10);
        i = atoi(jnk);
        printf("\n");
        if( i == sect.sct_chkpt ) goto X26;
        printf("Permission denied\n");
        if( i == 0 ) goto X176;
        return(-2);
X160:   
        if( sect.sct_chkpt != 0 ) goto X32;
        printf("%s", message);
X176:   
        return(-1);
}
