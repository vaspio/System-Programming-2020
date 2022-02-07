
CFLAGS = -c -g3
CC = gcc

PROGRAM1 = diseaseAggregator
PROGRAM2 = workers

OBJS1 = main.o extrafunctions.o
OBJS2 = workers.o list.o date.o avltree.o hashtable.o queries.o extrafunctions.o
SCRIPT = create_infiles.sh
FIFO = fifo*

$(PROGRAM1): clean $(OBJS1) $(OBJS2)
	$(CC) $(OBJS1) -o $(PROGRAM1)
	$(CC) $(OBJS2) -o $(PROGRAM2)

main.o: main.c
	$(CC) $(CFLAGS) main.c

workers.o: workers.c
	$(CC) $(CFLAGS) workers.c

list.o: list.c
	$(CC) $(CFLAGS) list.c

avltree.o: avltree.c
	$(CC) $(CFLAGS) avltree.c

hashtable.o: hashtable.c
	$(CC) $(CFLAGS) hashtable.c

date.o: date.c
	$(CC) $(CFLAGS) date.c

queries.o: queries.c
	$(CC) $(CFLAGS) queries.c

extrafunctions.o: extrafunctions.c
	$(CC) $(CFLAGS) extrafunctions.c

clean:
	rm -f $(PROGRAM1) $(OBJS1) $(PROGRAM2) $(OBJS2) $(FIFO) log_file*

r: $(PROGRAM1)
	./$(PROGRAM1) -w 4 -b 50 -i Database
