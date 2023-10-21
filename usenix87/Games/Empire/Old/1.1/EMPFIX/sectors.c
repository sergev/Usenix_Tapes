#define D_SECTDES
#define D_SCTSTR
#define D_NATSTR
#define D_FILES
#include        "empdef.h"

extern int      xflg, wflg, mflg;

sectors()
{
        char    *cp, *getstri();
        int     i, j, offx, offy, num;
        long    k;

        cp = getstri("Offset as country #? ");
        if( *cp == '\0' ) return;
        num = atoi(cp);
        i = num * sizeof(nat);
        lseek(natf, (long)i, 0);
        i = read(natf, &nat, sizeof(nat));
        if( i >= sizeof(nat) ) goto X156;
        printf("Only %d bytes in that nation...\n", i);
        return;
X156:   
        offx = nat.nat_xcap;
        offy = nat.nat_ycap;
        sectf = open(sectfil, O_RDWR);
X220:   
        xflg = wflg = mflg = 0;
        cp = getstri("x,y? ");
        if( *cp != '\0' ) goto X274;
        close(sectf);
        return;
X274:   
        i = atoip(&cp) + offx;
        if( *cp++ == ',') goto X346;
        printf("format is x,y\n");
        goto X220;
X346:   
        j = atoip(&cp) + offy;
        k = (((long)w_ysize + j)%w_ysize)*w_xsize + ((long)w_xsize + i)%w_xsize;
        lseek(sectf, k * sizeof(sect), 0);
        read(sectf, &sect, sizeof(sect));
        printf("Sector %d, %d\n", i, j);
        bytefix("owned", &sect.sct_owned, 0);
        bytefix("desig", &sect.sct_desig, (int)(sect.sct_desig > S_MOUNT) ? S_RURAL : sect.sct_desig);
        bytefix("effic", &sect.sct_effic, 0);
        bytefix("miner", &sect.sct_miner, (int)sect.sct_miner);
        bytefix("gmin", &sect.sct_gmin, (int)sect.sct_gmin);
        bytefix("civil", &sect.sct_civil, 0);
        bytefix("milit", &sect.sct_milit, 0);
        bytefix("shell", &sect.sct_shell, 0);
        bytefix("guns", &sect.sct_guns, 0);
        bytefix("plane", &sect.sct_plane, 0);
        bytefix("ore", &sect.sct_ore, 0);
        bytefix("gold", &sect.sct_gold, 0);
        bytefix("c_use", &sect.sct_c_use, 0);
        bytefix("m_use", &sect.sct_m_use, 0);
        bytefix("s_use", &sect.sct_s_use, 0);
        bytefix("g_use", &sect.sct_g_use, 0);
        bytefix("p_use", &sect.sct_p_use, 0);
        bytefix("o_use", &sect.sct_o_use, 0);
        bytefix("b_use", &sect.sct_b_use, 0);
        bytefix("prdct", &sect.sct_prdct, 0);
        bytefix("contr", &sect.sct_contr, 0);
        wordfix("chkpt", &sect.sct_chkpt, 0);
        bytefix("dfend", &sect.sct_dfend, 0);
        wordfix("mobil", &sect.sct_mobil, 0);
        bytefix("p_stage", &sect.sct_p_stage, 0);
        bytefix("p_time", &sect.sct_p_time, 0);
        wordfix("lstup", &sect.sct_lstup, 0);
        if( mflg != 0 ) goto X1550;
        goto X220;
X1550:  
        lseek(sectf, k * sizeof(sect), 0);
        write(sectf, &sect, sizeof(sect));
        printf("Rewritten\n");
        goto X220;
}
