CC=gcc
CFLAGS=-Wall -std=gnu99

# Target to build everything
all: Histogram

# Rule for building A1
Histogram: Histogram.c
	$(CC) $(CFLAGS) Histogram.c -o Histogram

# Clean rule for removing compiled objects and executable
clean:
	rm -f Histogram *.o
