all:
	gcc cachetest.c utils/parser.c utils/binarytree.c utils/llist.c utils/searchcache.c utils/string_linked_list.c utils/general_utils.c urlinfo.c utils/url_linked_list.c -o cachetest -lpcre
