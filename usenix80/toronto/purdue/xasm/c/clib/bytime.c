    int odlist[]{
	8,	4,
	9,	4,
	57,	5,
	59,	10,
	62,	9,
	63,	12,
	110,	4,
	126,	3,
	140,	3,
	141,	8,
	142,	3,
	143,	4,
	151,	4,
	151,	4,
	156,	4,
	159,	5,
	158,	4,
	167,	6,
	172,	6,
	173,	8,
	174,	6,
	175,	7,
	183,	5,
	188,	5,
	189,	9,
	190,	5,
	191,	6,
	206,	3,
	207,	4,
	210,	2,
	215,	4,
	221,	0,
	222,	4,
	223,	5,
	231,	6,
	238,	6,
	239,	7,
	247,	5,
	254,	5,
	255,	6,
	0,	0,
	0,	0,
	};

bytime(byte)
	int byte;
{
	register *k;
	register saveit;

	saveit = byte;
	if(((byte & 017) >= 7) || (byte == 210)){
		k = odlist;
		while(*k){
			if(byte == *k++){
				return(*k);
				}
			k++;
			}
		}
	byte = (saveit >> 4) & 017;
	if((byte & 012) == 0)return(2);
	if((byte & 016) == 2)return(4);
	if(byte & 010){
		switch(byte & 3){
			case 0:
				return(2);
				break;
			case 1:
				return(3);
				break;
			case 2:
				return(5);
				break;
			case 3:
				return(4);
				break;
			}
		}
	if((byte & 017) == 6)return(7);
	if((byte & 017) == 7)return(6);
}
