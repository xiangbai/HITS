#include <stdio.h>
#include <stdlib.h>

void report_error(char *error_msg)
{
	fputs(error_msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

int fgetline(char *line, int max)
{
    if (fgets(line, max, stdin) == NULL)
        return 0;
    else
        return strlen(line);
}