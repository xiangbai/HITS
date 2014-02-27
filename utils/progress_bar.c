/*
 * ===========================================================================
 *
 *       Filename:  progress_bar.c
 *
 * ===========================================================================
 */

#include "progress_bar.h"
#include <stdlib.h>
#include <string.h>

/**
 * Initialize progress bar to empty.
 * If bar is initialized, call bar_free_internal() before reinitializing
 * 	bar: pointer to the bar to initialize
 * 	max: the max value
 * 	display_length: length of string to display the progress
 */ 
void progress_init(progress_bar* bar, long max, int display_length)
{
	bar->max = max;
	bar->display = (char*)malloc(sizeof(char) * (display_length + 1));
	
	// basic setup for ends of display
	bar->display[0] = '[';
	bar->display[display_length - 1] = ']';
	bar->display[display_length] = '\0';

	// set all other chars to non-null
	int i;
	for (i = 1; i < display_length - 1; i++)
		bar->display[i] = ' ';
		
	// set value and draw initial display
	progress_setval(bar, 0);
}

/**
 * Free any allocated memory for the progress_bar
 * Currently equivalent to free(bar->display);
 * 	bar: pointer to the progress_bar
 */
void progress_free_internal(progress_bar* bar)
{
	bar->max = 0;
	bar->value = 0;
	free(bar->display);
}

/**
 * Set the value of the given progress_bar
 * Properly adjusts the display_string
 * 	bar: pointer to the progress_bar
 * 	value: new value for the bar
 */
void progress_setval(progress_bar* bar, long value)
{
	// set value
	bar->value = value;

	
	int max_chars = strlen(bar->display) - 2;	// -2 for outer brackets
	float percent = (float)value / bar->max;
	int full_chars = percent * max_chars;
	int i;
	
	// draw full chars
	for (i = 0; i < full_chars; i++)
		bar->display[i+1] = '*';		// +1 offset for first bracket
	
	// draw empty chars
	for ( ; i < max_chars; i++)
		bar->display[i+1] = ' ';		// +1 offset for first bracket
}

