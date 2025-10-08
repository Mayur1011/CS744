CC = gcc -Wno-write-strings -lpthread -g --no-warnings
C_FILE = generator.c
HTTP_CLIENT_FILE = http_client.c

all: generator

generator: $(C_FILE) $(HTTP_CLIENT_FILE)
	$(CC) $(C_FILE) $(HTTP_CLIENT_FILE) -o generator

clean:
	rm -f generator
	rm -rf results