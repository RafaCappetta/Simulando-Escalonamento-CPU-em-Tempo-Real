CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -Iinclude

rate: rate.o task.o
	$(CC) -o $@ $^

edf: edf.o
	$(CC) -o $@ $^

edf.o: ./source/edf.c ./include/task.h
	$(CC) -c $< -o $@ $(CFLAGS)

task.o: ./source/task.c ./include/task.h
	$(CC) -c $< -o $@ $(CFLAGS)

rate.o: ./source/rate.c ./include/task.h
	$(CC) -c $< -o $@ $(CFLAGS)

clean: 
	rm -rf *.o rate edf