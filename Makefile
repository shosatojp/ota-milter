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

DOCKER_TAG:=shosatojp/ota-milter:v1.0
docker-build:
	docker build -t $(DOCKER_TAG) .

docker-push:
	docker push $(DOCKER_TAG)
