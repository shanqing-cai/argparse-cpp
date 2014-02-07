BIN=test_argparse argparse_example

test_argparse: utils.cpp argparse.h argparse.cpp test_argparse.cpp
	g++ -Wall -c utils.cpp
	g++ -Wall -c argparse.cpp
	g++ -Wall -c test_argparse.cpp
	g++ -Wall argparse.o test_argparse.o utils.o -o test_argparse

argparse_example: utils.cpp argparse.h argparse.cpp argparse_example.cpp
	g++ -Wall -c utils.cpp
	g++ -Wall -c argparse.cpp
	g++ -Wall -c argparse_example.cpp
	g++ -Wall argparse.o argparse_example.o utils.o -o argparse_example

clean:
	rm -rf ${BIN} *.o