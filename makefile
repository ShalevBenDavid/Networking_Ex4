.PHONY = all clean
#Defining Macros
AR = ar
CC = gcc
FLAGS = -Wall -g

all: ping

#Creating Programs
ping: ping.o
	$(CC) $(FLAGS) -o ping ping.o
watchdog: watchdog.o
	$(CC) $(FLAGS) -o ping ping.o

#Creating Objects
ping.o: ping.c
	$(CC) $(FLAGS) -c ping.c
watchdog.o: watchdog.c
	$(CC) $(FLAGS) -c watchdog.c

clean:
	rm -f *.o ping watchdog