# Declare some macros

CC = gcc	
CFLAGS = -Wall -g 	# all warnings and debugger
OBJECTS = objectfiles/crawler_control.o objectfiles/socket_utils.o objectfiles/general_utils.o objectfiles/url_queue.o objectfiles/string_linked_list.o objectfiles/parser.o

all: crawler

crawler : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o crawler -lpcre

objectfiles/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

objectfiles/%.o: utils/%.c
	$(CC) -c $(CFLAGS) $< -o $@
