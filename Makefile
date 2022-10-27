COPT:=-W -Wall -Wno-write-strings -Wno-unused-parameter -Wno-unused-variable

default:
	g++ main.cpp $(COPT) -o main.out -lmilter -std=c++20
