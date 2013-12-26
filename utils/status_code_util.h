#ifndef STATUS_CODE_UTIL_H
#define STATUS_CODE_UTIL_H

#define STATUS_CODE_REGEX "(HTTP/\\d.\\d (\\d+) (.+))\\n"
#define STATUS_CODE_SUBSTRINGS 4
#define STATUS_CODE_FULL 1
#define STATUS_CODE_NUMBER 2
#define STATUS_CODE_MESSAGE 3

int get_status_code(char *code);

#endif
