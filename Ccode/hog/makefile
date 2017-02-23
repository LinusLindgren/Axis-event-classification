
LIBS=-lm
DEPS =$(wildcard *.h) 
FLAGS = -Wall
SRC=$(wildcard *.c)

%.o: %.c $(DEPS)
	$gcc -c -o $@ $< $(LIBS)


all: $(SRC) 
	gcc -o $@ $^ $(FLAGS) $(LIBS)

