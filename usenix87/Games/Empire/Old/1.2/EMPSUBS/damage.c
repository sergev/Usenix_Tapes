#define D_SCTSTR
#define D_SHPSTR
#define D_FILES
#include        "empdef.h"

int     damages;

sectdam(dam)
int     dam;    /* dam = % damage to be done */
{
        short   damage();

        damages = dam;
        sect.sct_effic = damage((short)sect.sct_effic);
        sect.sct_civil = damage((short)sect.sct_civil);
        sect.sct_milit = damage((short)sect.sct_milit);
        sect.sct_shell = damage((short)sect.sct_shell);
        sect.sct_guns  = damage((short)sect.sct_guns);
        sect.sct_plane = damage((short)sect.sct_plane);
        sect.sct_ore   = damage((short)sect.sct_ore);
        sect.sct_prdct = damage((short)sect.sct_prdct);
        sect.sct_mobil = damage(sect.sct_mobil);
}

shipdam(str, dam)
struct  shpstr *str;
int     dam;
{
        short   damage();

        damages = dam;
        str->shp_effc  = damage((short)str->shp_effc);
        str->shp_crew  = damage((short)str->shp_crew);
        str->shp_shels = damage((short)str->shp_shels);
        str->shp_gun   = damage((short)str->shp_gun);
        str->shp_plns  = damage((short)str->shp_plns);
        str->shp_or    = damage((short)str->shp_or);
        str->shp_mbl   = damage(str->shp_mbl);
}

short
damage(q)
short   q;
{
        short   quant, i, loss;

        quant = q;
        i = quant * damages;
        loss = i / 100;
        i -= loss * 100;
        if( i > rand()%100 ) loss++;
        quant -= loss;
        if( quant < 0 ) quant = 0;
        return(quant);
}
