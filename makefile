.PHONY = all clean
#Defining Macros
AR = ar
CC = gcc
FLAGS = -Wall -g

all: ping watchdog better_ping

#Creating Programs
ping: ping.o
	$(CC) $(FLAGS) -o ping ping.o
watchdog: watchdog.o
	$(CC) $(FLAGS) -o watchdog watchdog.o
better_ping: better_ping.o
	$(CC) $(FLAGS) -o better_ping better_ping.o

#Creating Objects
ping.o: ping.c
	$(CC) $(FLAGS) -c ping.c
watchdog.o: watchdog.c
	$(CC) $(FLAGS) -c watchdog.c
better_ping.o: better_ping.c
	$(CC) $(FLAGS) -c better_ping.c

clean:
	rm -f *.o ping watchdog better_ping