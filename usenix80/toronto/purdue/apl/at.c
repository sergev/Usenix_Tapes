/*
 * Declarations of all functions (required by Version 7 C
 * for the array to be set up without warning messages)
 */

	int ex_abs();
	int ex_add();
	int ex_and();
	int ex_ap();
	int ex_arg1();
	int ex_arg2();
	int ex_asgn();
	int ex_auto();
	int ex_base();
	int ex_botch();
	int ex_br();
	int ex_br0();
	int ex_cat();
	int ex_catk();
	int ex_ceil();
	int ex_chdir();
	int ex_cir();
	int ex_close();
	int ex_com();
	int ex_com0();
	int ex_comb();
	int ex_comk();
	int ex_creat();
	int ex_crp();
	int ex_ddom();
	int ex_deal();
	int ex_dfmt();
	int ex_dibm();
	int ex_diot();
	int ex_div();
	int ex_drho();
	int ex_drop();
	int ex_dtrn();
	int ex_dup();
	int ex_elid();
	int ex_eps();
	int ex_eq();
	int ex_exd();
	int ex_exd0();
	int ex_exdk();
	int ex_exec();
	int ex_exit();
	int ex_exp();
	int ex_fac();
	int ex_fdef();
	int ex_float();
	int ex_floor();
	int ex_fork();
	int ex_fun();
	int ex_gdd();
	int ex_gddk();
	int ex_gdu();
	int ex_gduk();
	int ex_ge();
	int ex_gt();
	int ex_hprint();
	int ex_ibr();
	int ex_ibr0();
	int ex_immed();
	int ex_index();
	int ex_iprod();
	int ex_kill();
	int ex_label();
	int ex_le();
	int ex_log();
	int ex_loge();
	int ex_lt();
	int ex_max();
	int ex_mdom();
	int ex_menc();
	int ex_meps();
	int ex_mfmt();
	int ex_mibm();
	int ex_min();
	int ex_minus();
	int ex_miot();
	int ex_mod();
	int ex_mrho();
	int ex_mtrn();
	int ex_mul();
	int ex_nand();
	int ex_nc();
	int ex_ne();
	int ex_nilret();
	int ex_nor();
	int ex_not();
	int ex_open();
	int ex_oprod();
	int ex_or();
	int ex_pi();
	int ex_pipe();
	int ex_plus();
	int ex_print();
	int ex_pwr();
	int ex_rand();
	int ex_rav();
	int ex_ravk();
	int ex_rd();
	int ex_read();
	int ex_recip();
	int ex_red();
	int ex_red0();
	int ex_redk();
	int ex_rep();
	int ex_rest();
	int ex_rev();
	int ex_rev0();
	int ex_revk();
	int ex_rot();
	int ex_rot0();
	int ex_rotk();
	int ex_run();
	int ex_scan();
	int ex_scn0();
	int ex_scnk();
	int ex_seek();
	int ex_sgn();
	int ex_signl();
	int ex_sub();
	int ex_take();
	int ex_unlink();
	int ex_wait();
	int ex_write();



int (*exop[])()
{
	0,		/* 0 */
	&ex_add,	/* 1 */
	&ex_plus,	/* 2 */
	&ex_sub,	/* 3 */
	&ex_minus,	/* 4 */
	&ex_mul,	/* 5 */
	&ex_sgn,	/* 6 */
	&ex_div,	/* 7 */
	&ex_recip,	/* 8 */
	&ex_mod,	/* 9 */
	&ex_abs,	/* 10 */
	&ex_min,	/* 11 */
	&ex_floor,	/* 12 */
	&ex_max,	/* 13 */
	&ex_ceil,	/* 14 */
	&ex_pwr,	/* 15 */
	&ex_exp,	/* 16 */
	&ex_log,	/* 17 */
	&ex_loge,	/* 18 */
	&ex_cir,	/* 19 */
	&ex_pi,		/* 20 */
	&ex_comb,	/* 21 */
	&ex_fac,	/* 22 */
	&ex_deal,	/* 23 */
	&ex_rand,	/* 24 */
	&ex_drho,	/* 25 */
	&ex_mrho,	/* 26 */
	&ex_diot,	/* 27 */
	&ex_miot,	/* 28 */
	&ex_rot0,	/* 29 */
	&ex_rev0,	/* 30 */
	&ex_dtrn,	/* 31 */
	&ex_mtrn,	/* 32 */
	&ex_dibm,	/* 33 */
	&ex_mibm,	/* 34 */
	&ex_gdu,	/* 35 */
	&ex_gduk,	/* 36 */
	&ex_gdd,	/* 37 */
	&ex_gddk,	/* 38 */
	&ex_exd,	/* 39 */
	&ex_scan,	/* 40 */
	&ex_exdk,	/* 41 */
	&ex_scnk,	/* 42 */
	&ex_iprod,	/* 43 */
	&ex_oprod,	/* 44 */
	0,		/* 45 */
	0,		/* 46 */
	&ex_br0,	/* 47 */
	&ex_br,		/* 48 */
	&ex_ddom,	/* 49 */
	&ex_mdom,	/* 50 */
	&ex_com,	/* 51 */
	&ex_red,	/* 52 */
	&ex_comk,	/* 53 */
	&ex_redk,	/* 54 */
	&ex_rot,	/* 55 */
	&ex_rev,	/* 56 */
	&ex_rotk,	/* 57 */
	&ex_revk,	/* 58 */
	&ex_cat,	/* 59 */
	&ex_rav,	/* 60 */
	&ex_catk,	/* 61 */
	&ex_ravk,	/* 62 */
	&ex_print,	/* 63 */
	0,		/* 64 */
	&ex_elid,	/* 65 */
	0,		/* 66 */
	0,		/* 67 */
	&ex_index,	/* 68 */
	&ex_hprint,	/* 69 */
	0,		/* 70 */
	&ex_lt,		/* 71 */
	&ex_le,		/* 72 */
	&ex_gt,		/* 73 */
	&ex_ge,		/* 74 */
	&ex_eq,		/* 75 */
	&ex_ne,		/* 76 */
	&ex_and,	/* 77 */
	&ex_or,		/* 78 */
	&ex_nand,	/* 79 */
	&ex_nor,	/* 80 */
	&ex_not,	/* 81 */
	&ex_eps,	/* 82 */
	&ex_meps,	/* 83 */
	&ex_rep,	/* 84 */
	&ex_take,	/* 85 */
	&ex_drop,	/* 86 */
	&ex_exd0,	/* 87 */
	&ex_asgn,	/* 88 */
	&ex_immed,	/* 89 */
	0,		/* 90 */
	0,		/* 91 */
	&ex_fun,	/* 92 */
	&ex_arg1,	/* 93 */
	&ex_arg2,	/* 94 */
	&ex_auto,	/* 95 */
	&ex_rest,	/* 96 */
	&ex_com0,	/* 97 */
	&ex_red0,	/* 98 */
	&ex_exd0,	/* 99 */
	&ex_scn0,	/*100 */
	&ex_base,	/*101 */
	&ex_menc,	/*102 */        /*      monadic encod   */
	&ex_label,	/*103 */
	0,		/*104 */
	0,		/*105 */
	0,		/*106 */
	0,		/*107 */
	0,		/*108 */
	0,		/*109 */
	0,		/*110 */
	0,		/*111 */
	&ex_run,	/*112 */
	&ex_fork,	/*113 */
	&ex_wait,	/*114 */
	&ex_exec,	/*115 */
	&ex_fdef,	/*116 */
	&ex_exit,	/*117 */
	&ex_pipe,	/*118 */
	&ex_chdir,	/*119 */
	&ex_open,	/*120 */
	&ex_close,	/*121 */
	&ex_read,	/*122 */
	&ex_write,	/*123 */
	&ex_creat,	/*124 */
	&ex_seek,	/*125 */
	&ex_unlink,	/*126 */
	&ex_rd,		/*127 */
	&ex_dup,	/*128 */
	&ex_ap,		/*129 */
	&ex_kill,	/*130 */
	&ex_crp,	/*131 */
	&ex_dfmt,	/*132 */
	&ex_mfmt,	/*133 */
	&ex_nc,		/*134 */
	&ex_nilret,	/*135 */
	&ex_botch,	/*136 	(XQUAD--never executed) */
	&ex_ibr,	/*137 */
	&ex_ibr0,	/*138 */
	&ex_botch,	/*139  (RVAL--in a1.c) */
	&ex_signl,	/*140 */
	&ex_float,	/*141 */
	&ex_botch,	/*142 */
	&ex_botch,	/*143 */
};
