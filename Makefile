all: main

main: main.o queue.o
	g++ -o main -levent main.o queue.o

main.o: main.cpp queue.h
	g++ --std=c++11 -c main.cpp

queue.o: queue.cpp queue.h
	g++ --std=c++11 -c queue.cpp
