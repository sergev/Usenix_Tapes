#include <stdio.h>
#include "bastrans.h"

dclvars(fp)
int fp;
{
	char var[8];
	FILE *fv, *fopen();
	if ((fv=fopen("/tmp/sbvar","r")) <= 0) error(FRE,"/tmp/sbvar");
	while (fscanf(fv,"%s",var) != EOF) {
		if (index(var,"str",1)>0)
			fprintf(fp,"\tchar ");
		else fprintf(fp,"\tdouble ");
		fprintf(fp,"%s;\n",var);
	}
	fclose(fv);
}
