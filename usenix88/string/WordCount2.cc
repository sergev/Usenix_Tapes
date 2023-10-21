/* Variable Length String Example 2 */
/* Copyright (c) 1987  P. A. Buhr */

#include <stream.h>
#include "string.h"

/*
This program reads lines containing words, locates each individual word,
and prints each word on a separate line.
*/

/*
This program works by locating the position of the first word in the line
(from left to right), copies the word from the line BUT does not remove the
word from the front of the original line. Pointers within the original line
are used to make sure that a word is never found twice.
*/

main() {
	string line, word, alpha;
	int FirstAlpha, FirstNonAlpha;

	alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	while (cin >> line) {				// read & print line
		cout << "Line:\"" << line << "\"\n\n";
		cout << "Words:\n";

		FirstNonAlpha = 1;			// start scan at beginning of line
		for (;;) {				// scan words off a line
			FirstAlpha = line.include(alpha, FirstNonAlpha); // find position of first alphabetic
		if (FirstAlpha > line.length()) break;			 // any alphabetic characters left ?
			FirstNonAlpha = line.skip(alpha, FirstAlpha);	 // find posn of 1st non-alpha character
			word = line.substr(FirstAlpha, FirstNonAlpha - FirstAlpha); // extract word from start of line

			
			cout << "     " << word << "\n";// print word
		} // for

		cout << "\n";
	} // for
	cout << "end of words\n";
} // main
