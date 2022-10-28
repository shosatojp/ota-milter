CC:=g++
CCOPT:=-W -Wall -Wno-write-strings -Wno-unused-parameter -Wno-unused-variable -std=c++20
LDOPT:=-lmilter
SRC:=$(shell find -name '*.cpp')
OBJ:=$(SRC:%.cpp=%.o)

default: main.out

main.out: $(OBJ)
	$(CC) $^ $(LDOPT) -o $@

%.o: %.cpp
	$(CC) $(CCOPT) $< -c -o $@

clean:
	rm -f main.out $(OBJ)
