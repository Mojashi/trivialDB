
OBJS = table.o transaction.o utils.o main.o
CFLAGS = -g
table.o: table.cpp
	g++ table.cpp -c $(CFLAGS)
transaction.o: transaction.cpp
	g++ transaction.cpp -c $(CFLAGS)
utils.o: utils.cpp
	g++ utils.cpp -c $(CFLAGS)
main.o: main.cpp
	g++ main.cpp -c $(CFLAGS)
db: $(OBJS)
	g++ $(OBJS) -o db $(CFLAGS)
clean:
	rm -f $(OBJS)