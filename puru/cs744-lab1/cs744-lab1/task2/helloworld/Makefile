CC = gcc
CFLAGS = -std=c23

SRCS = hw.c helloworld.c
OBJS = $(SRCS:.c=.o)
TARGET = hw

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ 

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)
