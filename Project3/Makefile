
CFLAGS = -c -g3
CC = gcc

PROGRAM1 = master
PROGRAM2 = workers
PROGRAM3 = whoServer
PROGRAM4 = whoClient

OBJS1 = master.o extrafunctions.o
OBJS2 = workers.o list.o date.o avltree.o hashtable.o queries.o extrafunctions.o
OBJS3 = whoServer.o extrafunctions.o whoServerFunctions.o
OBJS4 = whoClient.o extrafunctions.o whoClientFunctions.o
SCRIPT = create_infiles.sh
FIFO = fifo*

$(PROGRAM1): clean $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4)
	$(CC) $(OBJS1) -o $(PROGRAM1)
	$(CC) $(OBJS2) -o $(PROGRAM2)
	$(CC) $(OBJS3) -o $(PROGRAM3) -lpthread
	$(CC) $(OBJS4) -o $(PROGRAM4) -lpthread

master.o: master.c
	$(CC) $(CFLAGS) master.c

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

whoServer.o: whoServer.c
	$(CC) $(CFLAGS) whoServer.c

whoClient.o: whoClient.c
	$(CC) $(CFLAGS) whoClient.c

whoServerFunctions.o: whoServerFunctions.c
	$(CC) $(CFLAGS) whoServerFunctions.c

whoClientFunctions.o: whoClientFunctions.c
	$(CC) $(CFLAGS) whoClientFunctions.c

clean:
	rm -f $(PROGRAM1) $(OBJS1) $(PROGRAM2) $(OBJS2) $(PROGRAM3) $(OBJS3) $(PROGRAM4) $(OBJS4) $(FIFO) log_file*

r: $(PROGRAM1)
	./$(PROGRAM1) -w 4 -b 50 -s 127.0.0.1 -p 5000 -i Database
