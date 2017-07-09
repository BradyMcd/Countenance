CC = gcc
FLAGS = -Wall
OFLAGS = $(FLAGS) -c
OBJECTS = memory.o
SOURCES = $(OBJECTS:.o=.c)
PWD = $(shell pwd)
INCLUDE = -I$(PWD)

BUILDUTIL = build/sizeclasses build/pagesize

TEST =

TSRC = $(addsuffix .c,$(addprefix ./tests/,$(TEST)))
TTRGT = $(addprefix ./bin/,$(TEST))

all: vars memory.c
	$(CC) $(OFLAGS) memory.c -o memory.o -DSIZE_CLASSES=$(SIZE_CLASSES) -DPAGE_SIZE=$(PAGE_SIZE)

$(PWD)/build/%: $(PWD)/build/%.c
	$(CC) $(FLAGS) $< -o $@

.PHONY: vars
vars: $(BUILDUTIL)
	$(eval SIZE_CLASSES = $(shell ./build/sizeclasses))
	$(eval PAGE_SIZE = $(shell ./build/pagesize))

$(PWD)/bin/:
	mkdir bin

%.o: %.c
	$(CC) $(OFLAGS) $< -o $@

bin/%: tests/%.c
	$(CC) $(FLAGS) $< -o $@ $(OBJECTS) $(INCLUDE)

tests: $(PWD)/bin/ $(OBJECTS) $(TTRGT)

.PHONY: clean done

clean:
	rm -f $(OBJECTS) $(BUILDUTIL)

done: clean
	rm -rf bin *~
