#include <stdio.h>
#include <stdlib.h>

void report_error(char *error_msg)
{
	fputs(error_msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
