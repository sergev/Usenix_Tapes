BEGIN		{
		    print "/* This file was automatically generated */"
		    print "/* by the awk script \"sigconv.awk\".      */\n"
		    print "struct sigdesc {"
		    print "    char *name;"
		    print "    int  number;"
		    print "};\n"
		    print "struct sigdesc sigdesc[] = {"
		}

/^#define[ \t][ \t]*SIG[A-Z]/	{
				    printf "    \"%s\",\t%2d,\n", \
					substr($2, 4), $3
				}

END				{
				    print "    NULL,\t 0\n};"
				}
