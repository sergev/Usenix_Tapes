ordnp(n)
	int n;
{
	jnum(n,10,2,0);
	if((n > 10)&&(n < 14))n = 0;
	switch(n%10){
		case 0:
			printf("th.");
			break;
		case 1:
			printf("st.");
			break;
		case 2:
			printf("nd.");
			break;
		case 3:
			printf("rd.");
			break;
		default:
			printf("th.");
		}
}
