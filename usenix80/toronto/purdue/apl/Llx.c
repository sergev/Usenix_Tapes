	/*
	 * check for latent expr., and evaluate it if it is there:
	 */
	if((n=nlook("Llx")) && n->itemp->type == CH && n->itemp->size){
		putchar('\n');
		*sp++ = dupdat(n->itemp);
		ex_meps();
		ex_hprint();
		error("");
	}
