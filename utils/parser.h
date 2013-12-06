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
 * Create a linked list holding all values obtained from running regex p on text
 */
void parse_all(parser *p, char *text, size_t textlen, string_llist *destination);

/*
 * Free memory associated with a parser
 */
void kill_parser(parser *p);

#endif
