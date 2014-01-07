#include "parser.h"
#include "string_linked_list.h"
#include "general_utils.h"

#include <pcre.h>
#include <stdlib.h>
#include <string.h>

parser *init_parser(char *regex)
{
	parser *p = malloc(sizeof(parser));
	
	/*
	 * set the vector to hold results
	 * each pair in the first 2/3 of vectorsize holds the beginning
	 *	and one past the end of a substring's location
	 * the last third is used internally in pcre_exec()
	 * calculating number of substrings
	 */
	int count = 0;
	int i;
	int regex_len = strlen(regex);
	for (i = 0; i < regex_len; i++)
		if (regex[i] == '(')
			count++;
	p->vectorsize = 3 * (count + 1);
	
	// variables for holding error info
	const char *error;
	int erroroffset;
	
	// compile regex
	p->re = pcre_compile(regex, 0, &error, &erroroffset, 0);
	if (!p->re)
		report_error("Error compiling regex");

	// study regexp so pcre_exec can be optimized
	p->study = pcre_study(p->re, 0, &error);

	return p;
}

void parse_all(parser *p, char *text, size_t textlen, string_llist *destination, int *substring_indexes, size_t size)
{
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
		
		for (i = 0; i < size; i++)
		{
			// get start and one-past-the-end of the first part of the regex
			int stringbegin = vector[substring_indexes[i] * 2];
			
			if (stringbegin < 0)
				continue;
			
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

int substrings_to_array(parser *p, char *text, size_t textlen, size_t offset, char **array)
{	
	int vector[p->vectorsize];
	int nummatches = 0;
	int i;
	int numsubstrings = p->vectorsize / 3;
	// find matches for re with text, optimized w/ study
	int retval = pcre_exec(p->re, NULL,//p->study, 
		text, textlen, offset, 0, 
		vector, p->vectorsize);

	// if no matches found, return -1
	if (retval < 0 || vector[0] < 0)
		return -1;
	
	for (i = 0; i < numsubstrings; i++)
	{
		// get start and one-past-the-end of the first part of the regex
		int stringbegin = vector[i * 2];
		
		if (stringbegin < 0)
			array[i] = NULL;
		
		int stringend = vector[i * 2 + 1];
		int stringsize = stringend - stringbegin;
	
		// copy match to a new string
		char *newstring = (char *)malloc(stringsize + 1);
		//strcpy(newstring, text + stringbegin);
		memcpy(newstring, text + stringbegin, stringsize);
		newstring[stringsize] = '\0';
			
		// add to array
		array[i] = newstring;

	}
	return 0;
}

void clean_search_results(string_llist *list)
{
    char pattern[] = "((^(.(?!http))*$)|(<a onclick|<a class|google.com|facebook|youtube))";
    parser *jargonParser = init_parser(pattern);
    
    //int vector[jargonParser->vectorsize];
	int vector[jargonParser->vectorsize];
    int offset = 0;
	
	string_node **node = &list->front;
    while(*node != NULL)
	{
        string_node *current = *node;
		// find matches for re with text, optimized w/ study
		int retval = pcre_exec(jargonParser->re, NULL,//p->study,
                               current->string, strlen(current->string), offset, 0,
                               vector, jargonParser->vectorsize);
		
		node = &current->next;
        
        // if match found, delete from list
		if (retval > 0)
            string_llist_delete_node(list, &current);
	}
}

void kill_parser(parser *p)
{
	free(p->re);
	free(p->study);
	free(p);
}
