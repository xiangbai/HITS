/**************************************************************
 * multibuffer.c
 * 
 * this solves the problem of searching through buffer strings
 *   when the searched-for string spans 2 buffer strings
 **************************************************************/

/*
 * push a new buffer to the doublebuffer
 * this removes the first buffer in the double buffer
 *   then moves the second buffer to the first position
 *   then places the new buffer at the second position
 */
void push_to_multibuffer(char *double_buffer, char *buffer, size_t size)
{
	// store the size of the buffer minus its null character
	size_t = content_size = size - sizeof(char);

	memcpy(double_buffer, double_buffer[content_size], content_size);
	memcpy(double_buffer[content_size], buffer, size); 
}
