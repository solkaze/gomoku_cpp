CC = g++
CFLAGS = -g -Wall -Wextra

test: testmove.o testClass.o testCsv.o testProhibit.o testEvaluate.o
	$(CC) $(CFLAGS) -o $@ $^

testClass.o: testClass.cpp testClass.hpp
	$(CC) $(CFLAGS) -c -o $@ testClass.cpp

testCsv.o: testCsv.cpp testCsv.hpp
	$(CC) $(CFLAGS) -c -o $@ testCsv.cpp

testProhibit.o: testProhibit.cpp testProhibit.hpp
	$(CC) $(CFLAGS) -c -o $@ testProhibit.cpp

testmove.o: testmove.cpp
	$(CC) $(CFLAGS) -c -o $@ testmove.cpp

testEvaluate.o: testEvaluate.cpp
	$(CC) $(CFLAGS) -c -o $@ testEvaluate.cpp

clean:
	$(RM) *.o
	$(RM) test
