itoh(num)
int num;
{
 int i;
 char string[6];
 string[0] = (num & 0170000) >> 12;
 string[0] = (string[0] & 017) + 060;  /*  sign extension probs  */
 string[1] = 060 + ((num & 07400) >> 8);
 string[2] = 060 + ((num & 0360) >> 4);
 string[3] = 060 + (num & 017);
 for (i=0; i<4; i++)
	{if (string[i] >= 072)
		string[i] = string[i] + 7;
	putchar(string[i]);}
 return;
}
