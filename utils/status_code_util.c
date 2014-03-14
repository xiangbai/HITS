#include "status_code_util.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <pcre.h>
#include <string.h>


// Regex patterns to be used. Initialize once
parser *status_code_regex = NULL;
parser *location_regex = NULL;

int get_status_code(char *code)
{
	// define status_code_regex if not yet done
	if (!status_code_regex)
		status_code_regex = init_parser(STATUS_CODE_REGEX);

	int statuscode;
	
	// create array for substrings and clear it
	char *substrings[STATUS_CODE_SUBSTRINGS];
		
	substrings_to_array(status_code_regex, code, strlen(code), 0, substrings);
	
	if (!substrings[STATUS_CODE_NUMBER])
	{
		statuscode = -1;
		report_error("No status code found");
	}
	else
	{
		statuscode = atoi(substrings[STATUS_CODE_NUMBER]);
	}
	
	return statuscode;
}


char *get_300_location(char *code)
{
	// initialize location_regex if not yet done
	if (!location_regex)
		location_regex = init_parser(LOCATION_ERROR_REGEX);

	char *substrings[2];
	substrings[0] = NULL;
	substrings[1] = NULL;
	substrings_to_array(location_regex, code, strlen(code), 0, substrings);
	char *location = substrings[LOCATION_ERROR_URL];

	// free the other substring
	free(substrings[1 - LOCATION_ERROR_URL]);
		
	return location;
}
