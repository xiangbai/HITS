#include "parser.h"
#include "string_linked_list.h"
#include <pcre.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

parser *init_parser(char *regex)
{
	parser *p = malloc(sizeof(parser));
	
	// variables for holding error info
	const char *error;
	int erroroffset;
	
	// compile regex
	p->re = pcre_compile(regex, 0, &error, &erroroffset, 0);
	if (!p->re)
	{
		printf("error comiling regen");
		return NULL;
	}

	// study regexp so pcre_exec can be optimized
	p->study = pcre_study(p->re, 0, &error);

	/*
	 * set the vector to hold results
	 * each pair in the first 2/3 of vectorsize holds the beginning
	 *	and one past the end of a string's location
	 * the last third is used internally in pcre_exec()
	 */
	p->vectorsize = 30;
	return p;
}

void parse_all(parser *p, char *text, size_t textlen, string_llist *destination, int *substring_indexes, int num_substrings)
{
	//int vector[p->vectorsize];
	int vector[p->vectorsize];
	int offset = 0;
	int nummatches = 0;
	int i;

	// loop until no more matches are found
	while (1)
	{
		// find matches for re with text, optimized w/ study
		int retval = pcre_exec(p->re, NULL,//p->study, 
			text, textlen, offset, 0, 
			vector, p->vectorsize);
		// if no matches found, break
		if (retval < 0)
			break;

		// if match isn't found, break
		if (vector[0] < 0)
			break;
		
		for (i = 0; i < num_substrings; i++)
		{
			// get start and one-past-the-end of the first part of the regex
			int stringbegin = vector[substring_indexes[i] * 2];
			int stringend = vector[substring_indexes[i] * 2 + 1];
			int stringsize = stringend - stringbegin;
		
			// copy match to a new string
			char newstring[stringsize + 1];
			//strcpy(newstring, text + stringbegin);
			memcpy(newstring, text + stringbegin, stringsize);
			newstring[stringsize] = '\0';
				
			// push to linked list
			string_llist_push_back(destination, newstring);
		}

		// update starting offset
		offset = vector[1];
	}
}

void kill_parser(parser *p)
{
	free(p->re);
	free(p->study);
	free(p);
}
