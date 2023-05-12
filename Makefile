CC= gcc
FLAGS= -Wall -g
AR= ar

#make all
all: stnc 
.PHONY: clean
#make clean
clean:
	rm -f *.o stnc

stnc: stnc.o
	$(CC)  -o stnc stnc.o
stnc.o : stnc.c
	$(CC)  -c stnc.c


