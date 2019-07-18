CC = gcc
CCFLAGS =
SOURSES = Cmd.c
OBJECTS = $(SOURSES:.c=.o)
EXECUTABLE = $(SOURSES:.c=)

all: $(EXECUTABLE)

debug: clean
debug: CCFLAGS += -g
debug: all

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE).elf $(CCFLAGS)

$(OBJECTS): $(SOURSES)
	$(CC) $(SOURSES) -c $(CCFLAGS)

.PHONY: clean
clean:
	rm *.elf *.o
