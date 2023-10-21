main(){
	int clear,up,down,over,back,esc,equals,y1,y16,x79,star;
	int home,y7,bell,x40,y9;
	clear=26;up=11;down=10;over=12;back=8;esc=27;
	equals=61;y1=32;y16=55;x79=110;star=48;
	home=30;y7=44;x40=71;bell=7;y9=46;

	shell("stty nl");
	printf("%c",clear);
	printf("%c%c%c%c%cBEEP%c%c",home,esc,equals,y7,x40,bell,bell);

	printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
		bell,back,esc,equals,y9,x40,over,star,star,
		back,down,star,back,down,star,back,back,star,
		back,back,star,back,up,star,back,up,star,bell);

	printf("%c%c%c%c%c",esc,equals,y1,y1,star);

	printf("%c%c%c%c%c",esc,equals,y1,x79,star);

	printf("%c%c%c%c%c",esc,equals,y16,x79,star);

	printf("%c%c%c%c%c",esc,equals,y16,y1,star);

	printf("                                     ");
	printf("%c%c%c%c%c%c",up,up,up,up,up,up);
	shell("stty -nl");
}
