CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c11
LDFLAGS=

SRCS=main.c routes.c
OBJS=$(SRCS:.c=.o)
#HEADER=utils.h
TARGET=bin

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
