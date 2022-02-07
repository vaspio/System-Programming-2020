MODULES = ../modules

CFLAGS = -c -g3 -I.
CC = gcc

PROGRAM = diseaseMonitor

OBJS = main.o date.o

include $(MODULES)/List/make.inc
include $(MODULES)/HashTable/make.inc
include $(MODULES)/AvlTree/make.inc
include $(MODULES)/Queries/make.inc
include $(MODULES)/Topk/make.inc

$(PROGRAM): clean $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM)

clean:
	rm -f $(PROGRAM) $(OBJS)

r: $(PROGRAM)
	./$(PROGRAM) -p small.txt -h1 10 -h2 10 -b 80
