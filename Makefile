CC = gcc
FLAGS = -Wall
OFLAGS = $(FLAGS) -c
OBJECTS = memory.o
SOURCES = $(OBJECTS:.o=.c)
PWD = $(shell pwd)
INCLUDE = -I$(PWD)

TEST =

TSRC = $(addsuffix .c,$(addprefix ./tests/,$(TEST)))
TTRGT = $(addprefix ./bin/,$(TEST))

all: #empty until I make this a proper shared object

$(PWD)/bin/:
	mkdir bin

%.o: %.c
	$(CC) $(OFLAGS) $< -o $@

bin/%: tests/%.c
	$(CC) $(FLAGS) $< -o $@ $(OBJECTS) $(INCLUDE)

tests: $(PWD)/bin/ $(OBJECTS) $(TTRGT)

.PHONY: clean done

clean:
	rm -f $(OBJECTS)

done: clean
	rm -rf bin *~
