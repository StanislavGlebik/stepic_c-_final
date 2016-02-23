all: main

main: main.cpp
	g++ --std=c++11 -o main -levent main.cpp
