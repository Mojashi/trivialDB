
OBJS = table.o transaction.o utils.o
CFLAGS = -g
table.o: table.cpp
	g++ table.cpp -c $(CFLAGS)
transaction.o: transaction.cpp
	g++ transaction.cpp -c $(CFLAGS)
utils.o: utils.cpp
	g++ utils.cpp -c $(CFLAGS)
main.o: main.cpp
	g++ main.cpp -c $(CFLAGS)
test.o: test.cpp
	g++ test.cpp -c $(CFLAGS)
recovtest.o: recovtest.cpp
	g++ recovtest.cpp -c $(CFLAGS)
db: $(OBJS) main.o
	g++ $(OBJS) main.o -o db $(CFLAGS)
clean:
	rm -f $(OBJS) out.txt out2.txt db test recovtest redo.log data.db
test: $(OBJS) test.o recovtest.o
	g++ $(OBJS) test.o -o test $(CFLAGS);
	g++ $(OBJS) recovtest.o -o recovtest $(CFLAGS)
