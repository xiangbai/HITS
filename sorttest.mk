CC = gcc
CLFAGS = -Wall -g
OBJECTS = objectfiles/ranksorttest.o objectfiles/urlinfo.o objectfiles/ranksort.o objectfiles/parser.o objectfiles/string_linked_list.o objectfiles/general_utils.o
all: ranksorttest

ranksorttest : $(OBJECTS)
	$(CC) $(CLFAGS) $(OBJECTS) -o ranksorttest -lpcre

objectfiles/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

objectfiles/%.o: utils/%.c
	$(CC) -c $(CFLAGS) $< -o $@
