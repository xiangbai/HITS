CC = gcc
CFLAGS = -Wall -g
OBJECTS = objectfiles/crawler_control.o objectfiles/socket_utils.o objectfiles/general_utils.o objectfiles/url_queue.o objectfiles/string_linked_list.o

all: crawler

crawler : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o crawler

objectfiles/crawler_control.o: crawler_control.c
	$(CC) $(CFLAGS) -c $< -o objectfiles/crawler_control.o

objectfiles/socket_utils.o: utils/socket_utils.c
	$(CC) $(CFLAGS) -c $< -o objectfiles/socket_utils.o

objectfiles/general_utils.o: utils/general_utils.c
	$(CC) $(CFLAGS) -c $< -o objectfiles/general_utils.o

objectfiles/url_queue.o: url_queue.c
	$(CC) $(CFLAGS) -c $< -o objectfiles/url_queue.o

objectfiles/string_linked_list.o: utils/string_linked_list.c
	$(CC) $(CFLAGS) -c $< -o objectfiles/string_linked_list.o