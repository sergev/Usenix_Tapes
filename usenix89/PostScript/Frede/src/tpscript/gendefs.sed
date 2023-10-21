/^#/		d
/^$/		d
/^[ 	]/	d
/^[^ 	]/	{
		s/{//
		s/#.*//
		t loop
		:loop
			N
			s/\n}$/ /
			t done
			s/#.*//
			t loop
			b loop
		:done
		s/[ 	][ 	]*/ /g
		s/\n/ /g
		s/ /	"/
		s/^/#define	DEF_/
		s/ *$/"/
		}
