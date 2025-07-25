CC = gcc
CFLAGS = -Wall -Wextra -O2
OBJS = main.o compress.o decompress.o huffman.o utils.o

huffman: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o huffman