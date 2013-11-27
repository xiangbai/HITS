/**************************************************************
 * doublebuffer.h
 * 
 * helper function(s) to construct string of length 2x
 *   from strings of length x 
 * this solves the problem of searching through buffer strings
 *   when the searched-for string spans 2 buffer strings
 **************************************************************/

/*
 * push a new buffer to the doublebuffer
 * this removes the first buffer in the double buffer
 *   then moves the second buffer to the first position
 *   then places the new buffer at the second position
 */
void push_to_double_buffer(char *double_buffer, char *buffer, size_t size);
