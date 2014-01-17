all:
	gcc cachetest.c utils/searchcache.c utils/string_linked_list.c utils/general_utils.c -o cachetest
