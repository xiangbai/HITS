#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

/*
 * =====================================================================
 *
 *       Filename:  progress_bar.h
 *
 * =====================================================================
 */

typedef struct progress_bar
{
	long max;
	long value;
	char* display;
} progress_bar;

/**
 * Initialize progress bar to empty.
 * If bar is initialized, call bar_free_internal() before reinitializing
 * 	bar: pointer to the bar to initialize
 * 	max: the max value
 * 	display_length: length of string to display the progress
 */ 
void progress_init(progress_bar* bar, long max, int display_length);

/**
 * Free any allocated memory for the progress_bar
 * Currently equivalent to free(bar->display);
 * 	bar: pointer to the progress_bar
 */
void progress_free_internal(progress_bar* bar);

/**
 * Set the value of the given progress_bar
 * Properly adjusts the display_string
 * 	bar: pointer to the progress_bar
 * 	value: new value for the bar
 */
void progress_setval(progress_bar* bar, long value);

#endif
