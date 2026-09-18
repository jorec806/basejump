#Define compiler
CC = gcc

#Define the flags for compilation
CFLAGS = -Wall -Wextra -pedantic -std=gnu99
LDFLAGS += -fsanitize=address,undefined

# Specify the default target to run
.DEFAULT_GOAL := basejump

#Compilation process
basejump.o : main.c constants.h
	$(CC) $(CFLAGS) -c $< -o $@ 

program.o : program.c program.h parser.h constants.h 	
	$(CC) $(CFLAGS) -c $< -o $@ 

parser.o : parser.c parser.h constants.h
	$(CC) $(CFLAGS) -c $< -o $@

#filemode.o : filemode.c filemode.h constants.h parser.h
#	$(CC) $(CFLAGS) -c $< -o $@

#outputs.o : outputs.c outputs.h parser.h
#	$(CC) $(CFLAGS) -c $< -o $@

#interactive.o : interactive.c interactive.h parser.h outputs.h constants.h
#	$(CC) $(CFLAGS) -c $< -o $@

#Linking process
basejump : basejump.o program.o parser.o #filemode.o outputs.o interactive.o
	$(CC) $(CFLAGS) $^  $(LDFLAGS) -o $@


.PHONY: debug clean

debug: CFLAGS += -g -O0 -fsanitize=address,undefined
debug: clean basejump

clean:
	rm -f basejump *.o
