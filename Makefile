MAINS = main.o bench.o
OBJS = hashmap.o table.o transaction.o utils.o server.o record.o
CFLAGS = -std=c++17 -O3 -I/home/akihiro/vcpkg/installed/x64-linux/include/ -L/home/akihiro/vcpkg/installed/x64-linux/lib -ltbb -ltbbmalloc -lboost_chrono -lstdc++fs -pthread -Wfatal-errors
GCC = g++-8

table.o: table.cpp table.hpp utils.hpp hashmap.hpp transaction.hpp
	$(GCC) table.cpp -c $(CFLAGS)
record.o: record.cpp record.hpp utils.hpp
	$(GCC) record.cpp -c $(CFLAGS)
hashmap.o: hashmap.cpp hashmap.hpp
	$(GCC) hashmap.cpp -c $(CFLAGS)
server.o: server.cpp table.hpp server.hpp
	$(GCC) server.cpp -c $(CFLAGS)
transaction.o: transaction.cpp cnf.hpp table.hpp transaction.hpp
	$(GCC) transaction.cpp -c $(CFLAGS)
utils.o: utils.cpp
	$(GCC) utils.cpp -c $(CFLAGS)
main.o: main.cpp server.hpp
	$(GCC) main.cpp -c $(CFLAGS)
test.o: test.cpp
	$(GCC) test.cpp -c $(CFLAGS)
recovtest.o: recovtest.cpp
	$(GCC) recovtest.cpp -c $(CFLAGS)
bench.o: bench.cpp
	$(GCC) bench.cpp -c $(CFLAGS)
db: $(OBJS) main.o
	$(GCC) $(OBJS) main.o -o db $(CFLAGS)
clean:
	rm -f $(OBJS) $(MAINS) out.txt out2.txt db test recovtest logs/* data.db bench
test: $(OBJS) test.o recovtest.o
	$(GCC) $(OBJS) test.o -o test $(CFLAGS);
	$(GCC) $(OBJS) recovtest.o -o recovtest $(CFLAGS)

bench: $(OBJS) bench.o
	$(GCC) $(OBJS) bench.o -o bench $(CFLAGS)