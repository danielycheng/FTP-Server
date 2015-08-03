# Daniel Cheng - dyc8av
# Written April 27, 2015
# makefile for HW5, includes main.cpp 
# creates an executable my_ftpd

all: main.cpp 
	g++ main.cpp -o my_ftpd

clean:
	-rm -f *.o *~
