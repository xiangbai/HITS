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
 * Put the first instance of each substring into an array
 */
int substrings_to_array(parser *p, char *text, size_t textlen, size_t offset, char **array);

/*
 * cleans jargon links from search engine results page
 */
void clean_search_results(string_llist *list);

/*
 * Free memory associated with a parser
 */
void kill_parser(parser *p);

#endif
