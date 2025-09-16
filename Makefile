CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = lunar
SRCS = lunar.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
