/* Variable Length String Example 1 */
/* Copyright (c) 1987  P. A. Buhr */

#include <stream.h>
#include "string.h"

/*
This program reads lines containing words, locates each individual word,
and prints each word on a separate line.
*/

/*
This program works by locating the position of the first word in the line
(from left to right), copies the word from the line and removes the word
from the front of the original line. This is continued until the original
line has zero length.
*/

main() {
	string line, word, alpha;
	int bl;

	alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	while (cin >> line) {				// read & print line
		cout << "Line:\"" << line << "\"\n\n";
		cout << "Words:\n";

		
		for (;;) {				// scan words off a line
			line = line.substr(line.include(alpha)); // remove leading non-alphabetics
		if (line.length() == 0) break;		// any characters left ?
			bl = line.skip(alpha);		// find posn of 1st non-alpha character
			word = line.substr(1, bl - 1);	// extract word from start of line

			cout << "     " << word << "\n";// print word
			line = line.substr(bl);		// delete word from line
		} // for

		cout << "\n";
	} // for
	cout << "end of words\n";
} // main
