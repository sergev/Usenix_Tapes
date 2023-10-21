#
# include <math.h>
# include <stdio.h>
	/* Anova.c - WS Marshall - LSU/NO	*/
	/* Variables for ANOVA */
double y[20][100];
int N,n[20],ngrps;
double Gtotl,sumsqrgrp,corterm,SSgroups,SSwithin,SStot;
double MSamong,MSwithin,F;
double sumy[20];
double sumsqr[20],tsumsqr;
int i,j;

	/* Variables for Bartlett's test */

double S2,meanS2,X2[20],chisqr;
char ans;

	/* Variables for A posteriori tests */

double SScrit[3], SStest;
double rsumsqr;
double pow();
int rn;
main()
{
	printf("SINGLE CLASSIFICATION ANALYSIS OF VARIANCE\n\n");
	printf("for equal or unequal sample sizes and with\n");
	printf("Bartlett's test for equivalence of variance\n");
	printf("The two-sample case yields an F-test.\n\n");
	begin:
	for(i=0;i<ngrps;i++) sumy[i]=sumsqr[i]=SScrit[i]=0.0;
	N = Gtotl = tsumsqr = sumsqrgrp = chisqr = meanS2 = 0.0;
	printf(" ENTER NUMBER OF GROUPS =");
	scanf("%d", &ngrps);
	for(i=0;i<ngrps;i++)
		{
		printf("INPUT MEMBERS OF GROUP #%d\n", i+1);
		printf("LAST ENTRY = -1e10\n");
		for(j=0;j<100;j++)
			{
			printf("y[%d] = ",j+1);
			scanf("%F", &y[i][j]);
			if(y[i][j] == -1e10) break;
			}
		printf("\nAny corrections in this group? (y/n) : ");
		scanf("%s", &ans);
		if(ans == 'y') correction();
		n[i]= j;
		}
	for(i=0;i<ngrps;i++)
		{
		for(j=0;j<n[i];j++)
			{
			sumsqr[i]=+ y[i][j]*y[i][j];
			Gtotl=+ y[i][j];
			sumy[i]=+ y[i][j];
			}
		N=+ n[i];
		tsumsqr=+ sumsqr[i];
		sumsqrgrp=+ (sumy[i]*sumy[i])/n[i];
		}

	if(ngrps > 2)	bartletts();

	corterm= (Gtotl*Gtotl)/N;
	SStot= tsumsqr - corterm;
	SSgroups= sumsqrgrp - corterm;
	SSwithin= SStot - SSgroups;
	MSamong= SSgroups/(ngrps-1);
	MSwithin= SSwithin/(N - ngrps);
	F= MSamong/MSwithin;
	output();
	if(ngrps > 2)
		{
		printf("\n\nDo you want A posteriori comparisons? (y/n)\n");
		scanf("%s", &ans);
		if(ans == 'y') posteriori();
		}
	printf("\n\nMORE DATA? (y/n) :");
	scanf("%s", &ans);
	if(ans == 'y') goto begin;
}
correction()
{
	printf("\nEnter index # of correction : ");
	scanf("%d", &j);
	printf("\nCorrect y[%d] is: ", j);
	scanf("%F", &y[i][j-1]);
	printf("\nAny more corrections? (y/n) : ");
	scanf("%s", &ans);
	if(ans == 'y') correction();
}
bartletts()
{
	chisqr=0;
	for(i=0;i<ngrps;i++)
	{
		S2= (sumsqr[i] - ((sumy[i]*sumy[i])/n[i])) / (n[i]-1);
		chisqr=+ ((n[i]-1)*log(S2));
		meanS2=+ (S2/ngrps);
	}
	X2[0]=3.84; X2[1]=5.99; X2[2]=7.82; X2[3]=9.49; X2[4]=11.07;
	X2[5]=12.59; X2[6]=14.07; X2[7]=15.51; X2[8]=16.92; X2[9]=18.31;
	chisqr = -chisqr +(N-ngrps)*log(meanS2);
	printf("Bartlett's test result :\n");
	printf("chisquare = %7.4f for %d df\n",chisqr,ngrps-1);
	if(chisqr > X2[ngrps-2])
		{
		printf("\007Variances of the groups are unequal \n");
		printf("ANOVA may be invalid !\n");
		printf("Do you want the ANOVA result anyway? (y/n)\n");
		scanf("%s", &ans);
		if(ans == 'n') exit();
		}
}
posteriori()
{
	printf("Enter F value for df(num)=%d,df(denom)=%d",ngrps-1,N-ngrps);
	printf("\n at alpha = 0.01  :");
	scanf("%F", &SScrit[0]);
	printf("\n...and at alpha = 0.025 :");
	scanf("%F", &SScrit[1]);
	printf("\n...and at alpha = 0.05 :");
	scanf("%F", &SScrit[2]);
	for(i=0;i<3;i++) SScrit[i] =* MSwithin*(ngrps-1);
	printf("\nCOMPARISONS BETWEEN EACH PAIR OF GROUPS\n\n");
	for(i=0;i<ngrps-1;i++)
	{
	for(j=0;j<ngrps;j++)
		{
		if(j > i)
			{
			SStest= (sumy[i]*sumy[i]/n[i] + sumy[j] *
				sumy[j]/n[j]) -
				(pow((sumy[i] + sumy[j]),2.0) / (n[i]+n[j]));
			printf("GROUPS %d and %d :", i+1,j+1);
			printf(" SStest = %10.2f , ", SStest);
			if(SStest > SScrit[0]) printf(" P<0.01\n");
 			else if(SStest > SScrit[1]) printf(" P<0.025\n");
			else if(SStest > SScrit[2]) printf(" P<0.05\n");
			else  printf(" P>0.05 NS\n");
			}
		}
	}
	printf("\nCOMPARISONS BETWEEN EACH GROUP AND ALL OTHERS COMBINED\n\n");
	for(i=0;i<ngrps;i++)
	{
	rsumsqr = pow((Gtotl-sumy[i]),2.0) / (N - n[i]);
	SStest = sumy[i]*sumy[i]/n[i] + rsumsqr - corterm;
	printf("GROUP %d compared to all others :", i+1);
	printf("SStest = %10.2f , ", SStest);
	if(SStest > SScrit[0]) printf(" P<0.01\n");
	else if(SStest > SScrit[1]) printf(" P<0.025\n");
	else if(SStest > SScrit[2]) printf(" P<0.05\n");
	else printf(" P>0.05 NS\n");
	rsumsqr = 0.0;
	}
}
output()
{
	printf("                        ANOVA TABLE\n");
    printf("----------------------------------------------------------------");
	printf("\nSOURCE OF       df            SS             MS       F  ");
	printf("\nVARIATION\n");
    printf("----------------------------------------------------------------");
	printf("\n\n   AMONG    \t%d \t%10.3f",ngrps-1,SSgroups);
	printf(" \t%10.3f \t%6.4f\n\n",MSamong, F);
	printf("   WITHIN    \t%d \t%10.3f \t%10.3f",N-ngrps,SSwithin,MSwithin);
	printf("\n\n   TOTAL    \t%d \t%10.3f\n\n", N-1,SStot);
    printf("----------------------------------------------------------------");
}
