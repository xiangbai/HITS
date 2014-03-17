#ifndef STATUS_CODE_UTIL_H
#define STATUS_CODE_UTIL_H

#define STATUS_CODE_REGEX "(HTTP/\\d.\\d (\\d\\d\\d)(.+))\\n"
#define STATUS_CODE_SUBSTRINGS 4
#define STATUS_CODE_FULL 1
#define STATUS_CODE_NUMBER 2
#define STATUS_CODE_MESSAGE 3

#define LOCATION_ERROR_REGEX "Location: +([^\\n]+)\\n"
#define LOCATION_ERROR_URL 1

int get_status_code(char *code);

char *get_300_location(char *code);

#endif
