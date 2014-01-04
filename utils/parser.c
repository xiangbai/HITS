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

void parse_all(parser *p, char *text, size_t textlen, string_llist *destination)
{
	//int vector[p->vectorsize];
	int vector[p->vectorsize];
	int offset = 0;
	int nummatches = 0;
	
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
        
		// get start and one-past-the-end of the first part of the regex
		int stringbegin = vector[0];
		int stringend = vector[1];
		int stringsize = stringend - stringbegin;
		
		// if match isn't found, break
		if (stringbegin < 0)
			break;
		
		// copy match to a new string
		char newstring[stringsize + 1];
		//strcpy(newstring, text + stringbegin);
		memcpy(newstring, text + stringbegin, stringsize);
		newstring[stringsize] = '\0';
        
		// push to linked list
		string_llist_push_back(destination, newstring);
		
		// update starting offset
		offset = stringend;
	}
}

void clean_search_results(string_llist *list)
{
    char pattern[] = "((^(.(?!http))*$)|(<a onclick|<a class|google.com))";
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
