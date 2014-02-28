#ifndef PARSER_H
#define PARSER_H

#include <pcre.h>
#include "string_linked_list.h"

typedef struct parser
{
	pcre *re;
	pcre_extra *study;
	int vectorsize;
} parser;

/*
 * Initialize a parser with a given regexp
 */
parser *init_parser(char *regex);

/*
 * Add all values obtained from running regex p on text to linked list.
 * substring_index indicates which substring to seach for. 
 *	0 = entire pattern
 *	1 = the first substring in parentheses, etc.
 */
void parse_all(parser *p, char *text, size_t textlen, string_llist *destination, int *substring_indexes, size_t size);

/*
 * Put each substring of the first match into an array
 * 	parser: regex parser. This should be loaded with the pattern (see utils/parser.h)
 * 	text: text to parse
 *	textlen: length of above text = strlen(text)
 *	offset: starting point to look in text. use higher values to find more entries
 *	array: destingation array for substrings
 *	
 * returns the index of the substring found. Use as next offset when looping.
 */
int substrings_to_array(parser *p, char *text, size_t textlen, size_t offset, char **array);

/*
 * Free memory associated with a parser
 */
void kill_parser(parser *p);

#endif
