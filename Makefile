CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Iinclude
SRCDIR  = src
OBJDIR  = build
TARGET  = huffman

SRCS = $(SRCDIR)/main.c \
       $(SRCDIR)/huffman.c \
       $(SRCDIR)/compress.c \
       $(SRCDIR)/decompress.c \
       $(SRCDIR)/utils.c

OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
