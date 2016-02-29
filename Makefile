all: main

main: main.o queue.o util.o
	g++ -o main -levent main.o queue.o util.o

main.o: main.cpp queue.h
	g++ --std=c++11 -c main.cpp

queue.o: queue.cpp queue.h
	g++ --std=c++11 -c queue.cpp

util.o: util.cpp util.h
	g++ --std=c++11 -c util.cpp
