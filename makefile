.PHONY = all clean
#Defining Macros
AR = ar
CC = gcc
FLAGS = -Wall -g

all: ping watchdog better_ping

#Creating Programs
ping: ping.c
	$(CC) $(FLAGS) ping.c -o parta
watchdog: watchdog.c
	$(CC) $(FLAGS) watchdog.c -o watchdog
better_ping: better_ping.c
	$(CC) $(FLAGS) better_ping.c -o partb

clean:
	rm -f watchdog parta partb