# Declare some macros

CC = gcc	
CFLAGS = -Wall -g 	# all warnings and debugger
OBJECTS = objectfiles/binarytree.o objectfiles/llist.o objectfiles/crawler_control.o objectfiles/socket_utils.o objectfiles/general_utils.o objectfiles/urlinfo.o objectfiles/url_linked_list.o objectfiles/string_linked_list.o objectfiles/parser.o objectfiles/status_code_util.o objectfiles/domaininfo.o objectfiles/searchcache.o objectfiles/hits.o objectfiles/ranksort.o objectfiles/progress_bar.o

all: crawler

crawler : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o crawler -lpcre -lm -g

objectfiles/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

objectfiles/%.o: utils/%.c
	$(CC) -c $(CFLAGS) $< -o $@
