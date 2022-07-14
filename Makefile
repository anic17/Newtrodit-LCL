CC=gcc
CFLAGS=-Wall -Wno-unused-variable -Wno-unused-but-set-variable
INFILE=src/newtrodit.c
OUTFILE=newtrodit


all:
	$(CC) $(CFLAGS) $(INFILE) -o $(OUTFILE)

clean: 
	-rm -f $(OUTFILE)