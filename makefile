.PHONY = all clean
#Defining Macros
AR = ar
CC = gcc
FLAGS = -Wall -g

all: ping

#Creating Programs
ping: ping.o
	$(CC) $(FLAGS) -o ping ping.o

#Creating Objects
ping.o: ping.c
	$(CC) $(FLAGS) -c ping.c

clean:
	rm -f *.o ping