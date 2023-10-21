/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

/*
 *              # define CLASES 6
 *              # define MAXSHIPS 7
 *              # define ESPTYP 10
 *              # define ESPSIZ 8
 *
 *      struct _movable
 *      {
 *              int     popul[CLASES] ;
 *              int     metals ;
 *              int     know   ;
 *      };
 *
 *      struct  _planet
 *      {
 *              char    symbol  ;
 *              char    d_symbol[2] ;
 *              char    pid[4]  ;
 *              int     coord[2] ;
 *              planet *gate[10] ;
 *              int     whos ;
 *              movable inventar ;
 *              movable to_take ;
 *              int     secur ;
 *              int     alms    ;
 *              int     paint   ;
 *              int     detect  ;
 *              int     to_build[3] ;
 *              info *reports ;
 *              int     ships[MAXSHIPS] ;
 *              int     espion[2][ESPTYP][ESPSIZ];
 *      };
 *
 *      Some explanations about the planet structure :
 *
 *      The first field is the char shown on the map: '*', '@' and '#'.
 *      The second is the char currently displayed at each player term.
 *      Next is the name . Always three characters + '\0'.
 *      Next two numbers are the planets' coordinates on the terminal.
 *      Ten pointers that show aloowed planets to go to.
 *      The order in which they are initialized is the order of the
 *         key-pad on the tvi-925.
 *                              7    8    9
 *                              4    *    6
 *                              1    2    3
 *                             <-         ->
 *      Int representing ownership. 0-"*" 1-"@"  or 2 (for no-one).
 *  #### The next 3 items are movable, but belong to a planet. ######
 *      Next 6 numbers indicate the population, divided to occupation.
 *      One number shows the amount of metal digged out in A-type quan.
 *      One number indicating the knowledge level on the planet.
 *  #### The next 3 items are the storage location for transporting.
 *      Next 6 numbers indicate the population, divided to occupation.
 *      One number shows the amount of metal digged out in A-type quan.
 *      One number indicating the knowledge level on the planet.
 *      One number indicates black- out.
 *      One number indicates ALMs layed on that planet.
 *      One number indicating the sum dedicated to paint the ships.
 *      One number indicating sum dedicated to detect movement.
 *      one number indicating sum dedicated to build ships.
 *      Next 2 no. indicating how many ships to be build & their level.
 *      One poineter to reports about this planet.
 *      Next 7 no. show the ships and their level.
 *      Next 160 numbers indicate Espinage.
 *      Each player has a two dimension array that is comprised
 *      of 10 kinds of espionage and 8 levels of counter espionage.
 *      Each cell holds the money invested for obtaining information
 *      for that level and kind of information.
 */

planet pl[MAXPL] = {
    {
        '*', '*', '*', "l00", 8, 17, &pl[1], &pl[2], &pl[3], &pl[4], &pl[5], &pl[6], &pl[7], &pl[8], NULL, NULL, 0, 960, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 15
    },
    {
        '*', '*', '*', "l10", 6, 17, &pl[9], NULL, NULL, &pl[2], &pl[0], &pl[8], NULL, NULL, &pl[8], &pl[2]
    },
    {
        '*', '*', '*', "l11", 7, 19, NULL, &pl[10], NULL, &pl[3], NULL, &pl[0], NULL, &pl[1], &pl[1], &pl[3]
    },
    {
        '*', '*', '*', "l12", 8, 20, NULL, NULL, &pl[11], NULL, NULL, &pl[4], &pl[0], &pl[2], &pl[2], &pl[4]
    },
    {
        '*', '*', '*', "l13", 9, 19, NULL, &pl[3], NULL, &pl[12], NULL, &pl[5], NULL, &pl[0], &pl[3], &pl[5]
    }
    ,
    {
        '*', '*', '*', "l14", 10, 17, &pl[0], &pl[4], NULL, NULL, &pl[13], NULL, NULL, &pl[6], &pl[4], &pl[6]
    }
    ,
    {
        '*', '*', '*', "l15", 9, 15, NULL, &pl[0], NULL, &pl[5], NULL, &pl[14], NULL, &pl[7], &pl[5], &pl[7]
    }
    ,
    {
        '*', '*', '*', "l16", 8, 14, NULL, &pl[8], &pl[0], &pl[6], NULL, NULL, &pl[15], NULL, &pl[6], &pl[8]
    }
    ,
    {
        '*', '*', '*', "l17", 7, 15, NULL, &pl[1], NULL, &pl[0], NULL, &pl[7], NULL, &pl[16], &pl[7], &pl[1]
    }
    ,
    {
        '*', '*', '*', "l20", 3, 17, &pl[17], NULL, NULL, &pl[10], &pl[1], &pl[16], NULL, NULL, &pl[16], &pl[10]
    }
    ,
    {
        '*', '*', '*', "l21", 5, 22, NULL, &pl[19], NULL, &pl[11], NULL, &pl[2], NULL, &pl[9], &pl[9], &pl[11]
    }
    ,
    {
        '*', '*', '*', "l22", 8, 25, NULL, NULL, &pl[21], NULL, NULL, &pl[12], &pl[3], &pl[10], &pl[10], &pl[12]
    }
    ,
    {
        '*', '*', '*', "l23", 11, 22, NULL, &pl[11], NULL, &pl[23], NULL, &pl[13], NULL, &pl[4], &pl[11], &pl[13]
    }
    ,
    {
        '*', '*', '*', "l24", 13, 17, &pl[5], &pl[12], NULL, NULL, &pl[25], NULL, NULL, &pl[14], &pl[12], &pl[14]
    }
    ,
    {
        '*', '*', '*', "l25", 11, 12, NULL, &pl[6], NULL, &pl[13], NULL, &pl[27], NULL, &pl[15], &pl[13], &pl[15]
    }
    ,
    {
        '*', '*', '*', "l26", 8, 9, NULL, &pl[16], &pl[7], &pl[14], NULL, NULL, &pl[29], NULL, &pl[14], &pl[16]
    }
    ,
    {
        '*', '*', '*', "l27", 5, 12, NULL, &pl[9], NULL, &pl[8], NULL, &pl[15], NULL, &pl[31], &pl[15], &pl[9]
    }
    ,
    {
        '*', '*', '*', "l30", 0, 17, NULL, NULL, &pl[66], &pl[18], &pl[9], &pl[32], NULL, NULL, &pl[32], &pl[18]
    }
    ,
    {
        '*', '*', '*', "l31", 1, 22, NULL, &pl[66], NULL, &pl[19], NULL, NULL, NULL, &pl[17], &pl[17], &pl[19]
    }
    ,
    {
        '*', '*', '*', "l32", 3, 26, NULL, NULL, &pl[71], &pl[20], NULL, &pl[10], NULL, &pl[18], &pl[18], &pl[20]
    }
    ,
    {
        '*', '*', '*', "l33", 5, 28, NULL, NULL, &pl[74], &pl[21], NULL, NULL, NULL, &pl[19], &pl[19], &pl[21]
    }
    ,
    {
        '*', '*', '*', "l34", 8, 29, NULL, NULL, &pl[77], NULL, NULL, &pl[22], &pl[11], &pl[20], &pl[20], &pl[22]
    }
    ,
    {
        '*', '*', '*', "l35", 11, 28, NULL, &pl[21], &pl[80], NULL, NULL, &pl[23], NULL, NULL, &pl[21], &pl[23]
    }
    ,
    {
        '*', '*', '*', "l36", 13, 26, NULL, &pl[22], &pl[83], NULL, NULL, &pl[24], NULL, &pl[12], &pl[22], &pl[24]
    }
    ,
    {
        '*', '*', '*', "l37", 15, 22, NULL, &pl[23], NULL, &pl[86], NULL, &pl[25], NULL, NULL, &pl[23], &pl[25]
    }
    ,
    {
        '*', '*', '*', "l38", 16, 17, &pl[13], &pl[24], &pl[86], NULL, NULL, NULL, NULL, &pl[26], &pl[24], &pl[26]
    }
    ,
    {
        '*', '*', '*', "l39", 15, 12, NULL, NULL, NULL, &pl[25], NULL, NULL, NULL, &pl[27], &pl[25], &pl[27]
    }
    ,
    {
        '*', '*', '*', "l3a", 13, 8, NULL, &pl[14], NULL, &pl[26], NULL, NULL, NULL, &pl[28], &pl[26], &pl[28]
    }
    ,
    {
        '*', '*', '*', "l3b", 11, 6, NULL, NULL, NULL, &pl[27], NULL, NULL, NULL, &pl[29], &pl[27], &pl[29]
    }
    ,
    {
        '*', '*', '*', "l3c", 8, 5, NULL, &pl[30], &pl[15], &pl[28], NULL, NULL, NULL, NULL, &pl[28], &pl[30]
    }
    ,
    {
        '*', '*', '*', "l3d", 5, 6, NULL, &pl[31], NULL, NULL, NULL, &pl[29], NULL, NULL, &pl[29], &pl[31]
    }
    ,
    {
        '*', '*', '*', "l3e", 3, 8, NULL, &pl[32], NULL, &pl[16], NULL, &pl[30], NULL, NULL, &pl[30], &pl[32]
    }
    ,
    {
        '*', '*', '*', "l3f", 1, 12, NULL, &pl[17], NULL, NULL, NULL, &pl[31], NULL, NULL, &pl[31], &pl[17]
    }
    ,
    {
        '@', '@', '@', "r00", 8, 61, &pl[34], &pl[35], &pl[36], &pl[37], &pl[38], &pl[39], &pl[40], &pl[41], NULL, NULL, 1, 960, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 15
    }
    ,
    {
        '@', '@', '@', "r10", 6, 61, &pl[42], NULL, NULL, &pl[35], &pl[33], &pl[41], NULL, NULL, &pl[41], &pl[35], 1
    }
    ,
    {
        '@', '@', '@', "r11", 7, 63, NULL, &pl[43], NULL, &pl[36], NULL, &pl[33], NULL, &pl[34], &pl[34], &pl[36], 1
    }
    ,
    {
        '@', '@', '@', "r12", 8, 64, NULL, NULL, &pl[44], NULL, NULL, &pl[37], &pl[33], &pl[35], &pl[35], &pl[37], 1
    }
    ,
    {
        '@', '@', '@', "r13", 9, 63, NULL, &pl[36], NULL, &pl[45], NULL, &pl[38], NULL, &pl[33], &pl[36], &pl[38], 1
    }
    ,
    {
        '@', '@', '@', "r14", 10, 61, &pl[33], &pl[37], NULL, NULL, &pl[46], NULL, NULL, &pl[39], &pl[37], &pl[39], 1
    }
    ,
    {
        '@', '@', '@', "r15", 9, 59, NULL, &pl[33], NULL, &pl[38], NULL, &pl[47], NULL, &pl[40], &pl[38], &pl[40], 1
    }
    ,
    {
        '@', '@', '@', "r16", 8, 58, NULL, &pl[41], &pl[33], &pl[39], NULL, NULL, &pl[48], NULL, &pl[39], &pl[41], 1
    }
    ,
    {
        '@', '@', '@', "r17", 7, 59, NULL, &pl[34], NULL, &pl[33], NULL, &pl[40], NULL, &pl[49], &pl[40], &pl[34], 1
    }
    ,
    {
        '@', '@', '@', "r20", 3, 61, &pl[50], NULL, NULL, &pl[43], &pl[34], &pl[49], NULL, NULL, &pl[49], &pl[43], 1
    }
    ,
    {
        '@', '@', '@', "r21", 5, 66, NULL, &pl[52], NULL, &pl[44], NULL, &pl[35], NULL, &pl[42], &pl[42], &pl[44], 1
    }
    ,
    {
        '@', '@', '@', "r22", 8, 69, NULL, NULL, &pl[54], NULL, NULL, &pl[45], &pl[36], &pl[43], &pl[43], &pl[45], 1
    }
    ,
    {
        '@', '@', '@', "r23", 11, 66, NULL, &pl[44], NULL, &pl[56], NULL, &pl[46], NULL, &pl[37], &pl[44], &pl[46], 1
    }
    ,
    {
        '@', '@', '@', "r24", 13, 61, &pl[38], &pl[45], NULL, NULL, &pl[58], NULL, NULL, &pl[47], &pl[45], &pl[47], 1
    }
    ,
    {
        '@', '@', '@', "r25", 11, 56, NULL, &pl[39], NULL, &pl[46], NULL, &pl[60], NULL, &pl[48], &pl[46], &pl[48], 1
    }
    ,
    {
        '@', '@', '@', "r26", 8, 53, NULL, &pl[49], &pl[40], &pl[47], NULL, NULL, &pl[62], NULL, &pl[47], &pl[49], 1
    }
    ,
    {
        '@', '@', '@', "r27", 5, 56, NULL, &pl[42], NULL, &pl[41], NULL, &pl[48], NULL, &pl[64], &pl[48], &pl[42], 1
    }
    ,
    {
        '@', '@', '@', "r30", 0, 61, NULL, NULL, NULL, &pl[51], &pl[42], &pl[65], &pl[70], NULL, &pl[65], &pl[51], 1
    }
    ,
    {
        '@', '@', '@', "r31", 1, 66, NULL, NULL, NULL, &pl[52], NULL, NULL, NULL, &pl[50], &pl[50], &pl[52], 1
    }
    ,
    {
        '@', '@', '@', "r32", 3, 70, NULL, NULL, NULL, &pl[53], NULL, &pl[43], NULL, &pl[51], &pl[51], &pl[53], 1
    }
    ,
    {
        '@', '@', '@', "r33", 5, 72, NULL, NULL, NULL, &pl[54], NULL, NULL, NULL, &pl[52], &pl[52], &pl[54], 1
    }
    ,
    {
        '@', '@', '@', "r34", 8, 73, NULL, NULL, NULL, NULL, NULL, &pl[55], &pl[44], &pl[53], &pl[53], &pl[55], 1
    }
    ,
    {
        '@', '@', '@', "r35", 11, 72, NULL, &pl[54], NULL, NULL, NULL, &pl[56], NULL, NULL, &pl[54], &pl[56], 1
    }
    ,
    {
        '@', '@', '@', "r36", 13, 70, NULL, &pl[55], NULL, NULL, NULL, &pl[57], NULL, &pl[45], &pl[55], &pl[57], 1
    }
    ,
    {
        '@', '@', '@', "r37", 15, 66, NULL, &pl[56], NULL, NULL, NULL, &pl[58], NULL, NULL, &pl[56], &pl[58], 1
    }
    ,
    {
        '@', '@', '@', "r38", 16, 61, &pl[46], &pl[57], NULL, NULL, NULL, NULL, &pl[90], &pl[59], &pl[57], &pl[59], 1
    }
    ,
    {
        '@', '@', '@', "r39", 15, 56, NULL, NULL, NULL, &pl[58], NULL, &pl[90], NULL, &pl[60], &pl[58], &pl[60], 1
    }
    ,
    {
        '@', '@', '@', "r3a", 13, 52, NULL, &pl[47], NULL, &pl[59], NULL, NULL, &pl[85], &pl[61], &pl[59], &pl[61], 1
    }
    ,
    {
        '@', '@', '@', "r3b", 11, 50, NULL, NULL, NULL, &pl[60], NULL, NULL, &pl[82], &pl[62], &pl[60], &pl[62], 1
    }
    ,
    {
        '@', '@', '@', "r3c", 8, 49, NULL, &pl[63], &pl[48], &pl[61], NULL, NULL, &pl[79], NULL, &pl[61], &pl[63], 1
    }
    ,
    {
        '@', '@', '@', "r3d", 5, 50, NULL, &pl[64], NULL, NULL, NULL, &pl[62], &pl[76], NULL, &pl[62], &pl[64], 1
    }
    ,
    {
        '@', '@', '@', "r3e", 3, 52, NULL, &pl[65], NULL, &pl[49], NULL, &pl[63], &pl[73], NULL, &pl[63], &pl[65], 1
    }
    ,
    {
        '@', '@', '@', "r3f", 1, 56, NULL, &pl[50], NULL, NULL, NULL, &pl[64], NULL, &pl[70], &pl[64], &pl[50], 1
    }
    ,
    {
        '#', '#', '#', "c00", 0, 28, NULL, NULL, &pl[67], NULL, NULL, &pl[18], &pl[17], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c01", 0, 34, NULL, NULL, &pl[68], NULL, &pl[71], NULL, &pl[66], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c02", 0, 39, NULL, NULL, &pl[69], NULL, &pl[72], NULL, &pl[67], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c03", 0, 44, NULL, NULL, &pl[70], NULL, &pl[73], NULL, &pl[68], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c04", 0, 50, NULL, NULL, &pl[50], &pl[65], NULL, NULL, &pl[69], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c11", 3, 34, &pl[67], NULL, &pl[72], NULL, &pl[74], NULL, &pl[19], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c12", 3, 39, &pl[68], NULL, &pl[73], NULL, &pl[75], NULL, &pl[71], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c13", 3, 44, &pl[69], NULL, &pl[64], NULL, &pl[76], NULL, &pl[72], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c21", 5, 34, &pl[71], NULL, &pl[75], NULL, &pl[77], NULL, &pl[20], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c22", 5, 39, &pl[72], NULL, &pl[76], NULL, &pl[78], NULL, &pl[74], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c23", 5, 44, &pl[73], NULL, &pl[63], NULL, &pl[79], NULL, &pl[75], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c31", 8, 34, &pl[74], NULL, &pl[78], NULL, &pl[80], NULL, &pl[21], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c32", 8, 39, &pl[75], NULL, &pl[79], NULL, &pl[81], NULL, &pl[77], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c33", 8, 44, &pl[76], NULL, &pl[62], NULL, &pl[82], NULL, &pl[78], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c41", 11, 34, &pl[77], NULL, &pl[81], NULL, &pl[83], NULL, &pl[22], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c42", 11, 39, &pl[78], NULL, &pl[82], NULL, &pl[84], NULL, &pl[80], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c43", 11, 44, &pl[79], NULL, &pl[61], NULL, &pl[85], NULL, &pl[81], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c51", 13, 34, &pl[80], NULL, &pl[84], NULL, &pl[87], NULL, &pl[23], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c52", 13, 39, &pl[81], NULL, &pl[85], NULL, &pl[88], NULL, &pl[83], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c53", 13, 44, &pl[82], NULL, &pl[60], NULL, &pl[89], NULL, &pl[84], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c60", 16, 28, NULL, NULL, &pl[87], NULL, NULL, NULL, &pl[25], &pl[24], NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c61", 16, 34, &pl[83], NULL, &pl[88], NULL, NULL, NULL, &pl[86], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c62", 16, 39, &pl[84], NULL, &pl[89], NULL, NULL, NULL, &pl[87], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c63", 16, 44, &pl[85], NULL, &pl[90], NULL, NULL, NULL, &pl[88], NULL, NULL, NULL, 2
    }
    ,
    {
        '#', '#', '#', "c64", 16, 50, NULL, &pl[59], &pl[58], NULL, NULL, NULL, &pl[89], NULL, NULL, NULL, 2
    }
};
