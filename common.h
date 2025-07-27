/**
 * Prints the given ASCII code {@code c} into a string. {@code -1} is
 * interpreted as EOF and printed as such. Printable characters are printed like
 * normal. Special characters are printed as such:
 * \0 for null character
 * \n for newline character
 * \r for carrier return
 * \t for tab
 * \s for space
 * \x__ for specifying the char hex code
 *
 * Any non-ASCII code is printed as:
 * \?________
 */
char *print_char(int c);
