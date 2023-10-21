struct	universe { /* see getunent(3) */
	char	*un_name;
	char	*un_universe;
};

struct universe *getunent(), *getunnam();
