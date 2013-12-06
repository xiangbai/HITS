all: crawler

crawler: crawler_control.o string_linked_list.o get_address_info.o url_queue.o report_error.o
	gcc crawler_control.o string_linked_list.o get_address_info.o url_queue.o report_error.o -o crawler

crawler_control.o: crawler_control.c
	gcc -c crawler_control.c

url_queue.o: url_queue.c
	gcc -c url_queue.c

string_linked_list.o: utils/string_linked_list.c
	gcc -c utils/string_linked_list.c

report_error.o: utils/report_error.c
	gcc -c utils/report_error.c

get_address_info.o: get_address_info.c utils/report_error.c
	gcc -c get_address_info.c utils/report_error.c

