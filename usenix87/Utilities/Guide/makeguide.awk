BEGIN { false = 0; true = 1; lin1 = false;}
$2 == "" {
	if(lin1 && $1 != prev){ print(prev); }
	prev = $1;
	lin1 = true; 
  }
$2 != "" {
	if(lin1 && prev != $1){ print(prev); }
	printf( "%s\t", $1);
	if(length($1) < 8 ){ printf("\t"); }
	if(prev != $1 && $2 != "#")printf("# ");
	for(i=2;i <= NF-1;i++)printf("%s ",$i);
	printf("%s\n",$NF);
	lin1 = false;
  }
END { if(lin1){ print(prev); } }
