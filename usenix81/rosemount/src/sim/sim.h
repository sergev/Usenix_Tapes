/*
 *   S I M U L A T I O N   D A T A   S T R U C T U R E S
 */

extern double simtime ;						/* Simulation time */

struct squeue {								/* Semaphore item */
	int count ;
	char *first ;
	char *last ;
	} ;
